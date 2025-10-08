#include "chatservice.hpp"
#include "public.hpp"
#include <muduo/base/Logging.h>//LOG_ERROR
#include "database.h"
using namespace muduo;
using namespace placeholders;

ChatService& ChatService::getInstance(){
    //局部静态变量在程序生命周期内只会被初始化一次
    static ChatService m_instance;
    return m_instance;
}

ChatService::ChatService(){
    //绑定消息id与对应业务处理方法
    _MsgHandlerMap.insert({LOGIN_MSG,bind(&ChatService::login,this,_1,_2,_3)});//登录业务
    //类的成员函数不能直接作为回调，想要其作为回调，必须显式绑定this指针，所以使用std::bind 进行绑定

    _MsgHandlerMap.insert({REG_MSG,bind(&ChatService::reg,this,_1,_2,_3)});//注册业务
    _MsgHandlerMap.insert({ONE_CHAT_MSG,bind(&ChatService::oneChat,this,_1,_2,_3)});//一对一聊天业务
    _MsgHandlerMap.insert({ADD_FRIEND_MSG,bind(&ChatService::addFriend,this,_1,_2,_3)});//添加好友业务
    _MsgHandlerMap.insert({CREATE_GROUP_MSG,bind(&ChatService::createGroup,this,_1,_2,_3)});//创建群组业务
    _MsgHandlerMap.insert({ADD_GROUP_MSG,bind(&ChatService::addGroup,this,_1,_2,_3)});//加入群组业务
    _MsgHandlerMap.insert({GROUP_CHAT_MSG,bind(&ChatService::groupChat,this,_1,_2,_3)});//群聊天业务
    _MsgHandlerMap.insert({LOGINOUT_MSG,bind(&ChatService::loginout,this,_1,_2,_3)});//登出业务

    //连接redis服务器
    if(_redis.connect()){
        //设置上报消息的回调
        _redis.init_notify_handler(bind(&ChatService::handleRedisSubscribeMessage,this,_1,_2));
    }
}

//处理登录业务（除了msgid还需要id和password字段）
void ChatService::login(const TcpConnectionPtr &conn,json &js,Timestamp tim){
    //获取字段
    int msgid=js["msgid"];
    int id=js["id"];
    string pwd=js["password"];
    //通过ip查找数据库中User
    User user=_userModel.query(id);

    //检查password是否相同（user.GetId()==-1->id对应用户不存在或其他错误）
    if(pwd==user.GetPassword()&&user.GetId()!=-1){
        //密码正确，返回json给客户端
        if(user.GetState()=="online"){
            //用户已登录，不能重复登录
            json response;
            response["msgid"]=msgid;
            response["receive_state"]="success";//表示接收成功
            response["errno"]=1;//表示登录有错误
            response["errmsg"]="The user is already logged in. No repeated login is allowed";
            conn->send(response.dump());
        }
        else{
            //登录成功
            json response;
            response["msgid"]=msgid;
            response["receive_state"]="success";//表示接收成功
            response["errno"]=0;//表示登录无错误
            response["id"]=user.GetId();
            response["name"]=user.GetName();
            
            //1.更新用户state到数据库
            user.SetState("online");
            _userModel.updateState(user);
            
            //2.记录用户通信连接信息（多线程安全）
            {
                lock_guard<mutex> lock(_connmutex);
                _userConnMap.insert({id,conn});
            }
        
            //3.向redis订阅channel(id)
            _redis.subscribe(id);

            //4.查找是否有离线消息
            vector<string> vec;
            vec=_offlineMsgModel.query(id);
            if(!vec.empty()){
                //若有离线消息，直接传出
                response["offline_msg"]=vec;
                //将其离线消息从_offlineMsgModel中移除
                _offlineMsgModel.remove(id);
            }

            //5.显示好友信息
            vector<User> FriendVec=_friendModel.query(id);
            if(!FriendVec.empty()){
                //若有好友
                response["friends"]=json::array();// JSON 数组 "friends"
                for(User &user:FriendVec){
                    json friend_js;
                    friend_js["friend_id"]=user.GetId();
                    friend_js["friend_name"]=user.GetName();
                    friend_js["friend_state"]=user.GetState();
                    response["friends"].push_back(friend_js);
                }
            }
            //6.显示群组信息
            vector<Group> GroupVec=_groupModel.queryGroups(id);
            if(!GroupVec.empty()){
                //若有群组
                response["groups"]=json::array();// JSON 数组 "groups"
                for(Group &group:GroupVec){
                    json group_js;
                    group_js["group_id"]=group.GetId();
                    group_js["group_name"]=group.GetGroupname();
                    group_js["group_desc"]=group.GetGroupdesc();
                    vector<GroupUser> guservec=group.getUsers();
                    group_js["group_users"]=json::array();// JSON 数组 "group_users"
                    for(GroupUser &guser:guservec){
                        json guser_js;
                        guser_js["id"]=guser.GetId();
                        guser_js["name"]=guser.GetName();
                        guser_js["state"]=guser.GetState();
                        guser_js["grouprole"]=guser.GetRole();
                        group_js["group_users"].push_back(guser_js);
                    }
                    response["groups"].push_back(group_js);
                }
            }

            conn->send(response.dump());//只需要序列化reponse，其中的json不需要依次序列化
        }
    }
    else{
        //密码错误，或id对应用户不存在，或其他错误：返回json给客户端
        json response;
        response["msgid"]=msgid;
        response["receive state"]="success";//表示接收成功
        response["errno"]=1;//表示登录有错误
        response["errmsg"]="incorrect password, or the user corresponding to the ID does not exist, or there is some other error.";
        conn->send(response.dump());
    }
}
//处理注册业务（除了msgid还需要name和password字段）
void ChatService::reg(const TcpConnectionPtr &conn,json &js,Timestamp tim){
    //获取字段
    int msgid=js["msgid"];
    string name=js["name"];
    string pwd=js["password"];
    //创建User对象
    User user;
    user.SetName(name);
    user.SetPassword(pwd);
    //将此User对象插入User表
    bool ret=_userModel.insert(user);
    if(ret){
        //注册成功，返回json给客户端
        json response;
        response["msgid"]=msgid;
        response["receive_state"]="success";//表示接收成功
        response["errno"]=0;//表示注册无错误
        response["id"]=user.GetId();
        conn->send(response.dump());
    }
    else{
        //注册失败，返回json给客户端
        json response;
        response["msgid"]=msgid;
        response["receive state"]="success";//表示接收成功
        response["errno"]=1;//表示注册有错误
        conn->send(response.dump());
    }
}
//处理一对一聊天业务（除了msgid还需要id，name，toid，msg，time字段：己方id，己方name，对方id，聊天消息，发送消息的时间）
void ChatService::oneChat(const TcpConnectionPtr &conn,json &js,Timestamp tim){
    //获取字段
    int msgid=js["msgid"];
    int toid=js["toid"];
    //调整消息格式
    json res;
    res["msgid"]=msgid;
    res["from_id"]=js["id"];
    res["from_name"]=js["name"];
    res["msg"]=js["msg"];
    res["time"]=js["time"];
    
    //操作_userConnMap需要多线程安全{}
    {
        lock_guard<mutex> lock(_connmutex);
        //在_userConnMap中查找对方是否在线
        auto it=_userConnMap.find(toid);
        if(it!=_userConnMap.end()){
            //找到，说明对方在线，直接转发消息给对方
            it->second->send(res.dump());
            return ;//直接退出，下文代码属于未找到的操作
        }
    }

    //在数据库中查找对方是否在线，若在线，说明对方连接着其他服务器
    User user=_userModel.query(toid);
    if(user.GetState()=="online"){
        _redis.publish(toid,res.dump());
        return;
    }

    //未找到，说明对方不在线，存储离线消息
    _offlineMsgModel.insert(toid,res.dump());
}
//处理添加好友业务（除了msgid还需要id，toid字段：己方id，对方id）
void ChatService::addFriend(const TcpConnectionPtr &conn,json &js,Timestamp tim){
    //获取字段
    int userid=js["id"];
    int friendid=js["toid"];
    //存储好友信息
    _friendModel.insert(userid,friendid);
}

MsgHandler ChatService::getHandler(int msgid){
    
    auto it=_MsgHandlerMap.find(msgid);
    if(it==_MsgHandlerMap.end()){
        //若msgid没有对应的事件回调函数，返回一个默认函数（lambda）
        return [=](const TcpConnectionPtr &conn,json &js,Timestamp tim)->void{
            LOG_ERROR<<"msgid:"<<msgid<<"  can not find handler!"; //记录错误日志
        };
    }
    else{
        //未出错
        return _MsgHandlerMap[msgid];
    }
    
}
//处理创建群组业务（除了msgid还需要userid，groupname，groupdesc字段：群组创建用户id，群组名，群组功能描述）
void ChatService::createGroup(const TcpConnectionPtr &conn,json &js,Timestamp tim){
    //获取字段
    int userid=js["userid"];
    string groupname=js["groupname"];
    string groupdesc=js["groupdesc"];
    
    //群组（需要1.群组id 2.群组名 3.群组功能描述 4.群组用户信息）
    Group group;
    group.SetGroupname(groupname); //2.
    group.SetGroupdesc(groupdesc); //3.

    //创建群组(自动生成主键：1.群组id)
    if(_groupModel.createGroup(group)){
        //将群组创建用户加入群组 4.
        _groupModel.addGroup(userid,group.GetId(),"creator");
    }

}
//处理加入群组业务（除了msgid还需要userid，groupid字段）
void ChatService::addGroup(const TcpConnectionPtr &conn,json &js,Timestamp tim){
    //获取字段
    int userid=js["userid"];
    int groupid=js["groupid"];
    int msgid=js["msgid"];

    if(_groupModel.addGroup(userid,groupid,"normal")){
        //加入成功，返回json给客户端
        json res;
        res["msgid"]=msgid;
        res["receive_state"]="success"; //表示接收成功
        res["errno"]=0; //表示无错误
        conn->send(res.dump());
    }
    else{
        //加入失败，返回json给客户端
        json res;
        res["msgid"]=msgid;
        res["receive_state"]="success"; //表示接收成功
        res["errno"]=1; //表示有错误
        res["errmsg"]="The group you are trying to join does not exist or an error occurred!";
        conn->send(res.dump());
    }
}
//处理群聊天业务（除了msgid还需要userid，groupid，msg，time字段）
void ChatService::groupChat(const TcpConnectionPtr &conn,json &js,Timestamp tim){
    //获取字段
    int msgid=js["msgid"];
    int userid=js["userid"];
    int groupid=js["groupid"];
    string msg=js["msg"];

    //封装要发送msg的json
    json res;
    res["msgid"]=msgid;
    res["from_membername"]=js["name"];
    res["from_groupid"]=groupid;
    res["from_memberid"]=userid;
    res["msg"]=msg;
    res["time"]=js["time"];

    //根据groupid查询群组用户id列表，除userid自己
    vector<int> idVec=_groupModel.queryGroupUsers(userid,groupid);
    
    //向每个用户分别发送msg
    lock_guard<mutex> lock(_connmutex); //操作_userConnMap需要多线程安全
    for(int &memberid:idVec){
        //在_userConnMap中查找对方是否在线
        auto it=_userConnMap.find(memberid);
        if(it!=_userConnMap.end()){
            //找到，说明对方在线，直接转发消息给对方
            it->second->send(res.dump());
        }
        else{
            //在数据库中查找对方是否在线，若在线，说明对方连接着其他服务器
            User user=_userModel.query(memberid);
            if(user.GetState()=="online"){
                _redis.publish(memberid,res.dump());
            }
            else{
                //未找到，说明对方不在线，存储离线消息
                _offlineMsgModel.insert(memberid,res.dump());
            }
        }
    }
}
//处理登出业务（除了msgid还需要userid字段）
void ChatService::loginout(const TcpConnectionPtr &conn,json &js,Timestamp tim){
    //获取字段
    int msgid=js["msgid"];
    int userid=js["userid"];
    User user;
    user.SetId(userid);
    //多线程安全{}
    {
        lock_guard<mutex> lock(_connmutex);
        //将用户从_userConnMap中移除
        auto it=_userConnMap.find(userid);
        if(it!=_userConnMap.end()){
            _userConnMap.erase(it);
        }   
    }

    //在redis中取消订阅
    _redis.unsubscribe(userid);

    //将用户数据库state改为offline
    user.SetState("offline");
    _userModel.updateState(user);
    //封装json，发送给客户端
    json res;
    res["msgid"]=msgid;
    conn->send(res.dump());
}
//处理客户端异常断开
void ChatService::clientCloseException(const TcpConnectionPtr &conn){
    User user;
    //多线程安全{}
    {
        lock_guard<mutex> lock(_connmutex);
        
        //1.通过conn在_userConnMap中查找用户id
        for(auto it=_userConnMap.begin();it!=_userConnMap.end();it++){
            if(it->second==conn){
                user.SetId(it->first);
                //2.将该用户从_userConnMap中移除
                _userConnMap.erase(it);
                break;
            }
        }
    }
    if(user.GetId()!=-1){//确定找到该用户了

        //在redis中取消订阅
        _redis.unsubscribe(user.GetId());

        //3.将该用户数据库state改为offline
        user.SetState("offline");
        _userModel.updateState(user);
    }
}
//服务器异常断开（如Ctrl+C），重置业务
void ChatService::reset(){
    //重置所有online1用户的状态为offline
    _userModel.resetState();
}

// 从redis消息队列中获取订阅的消息
void ChatService::handleRedisSubscribeMessage(int userid, string msg){

    lock_guard<mutex> lock(_connmutex);
    auto it=_userConnMap.find(userid);
    if(it!=_userConnMap.end()){
        it->second->send(msg);
        return;
    }
    //存储离线消息
    _offlineMsgModel.insert(userid,msg);

}