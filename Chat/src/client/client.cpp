#include "client.hpp"

//显示当前登录成功用户的基本信息
void showCurrentUserData(){
    cout<<"____________________login user____________________"<<endl;//当前系统登录的用户信息
    cout<<"current login user  ->  id:"<<g_currentUser.GetId()
    <<" name:"<<g_currentUser.GetName()<<endl;
    cout<<"____________________friend list____________________"<<endl;//当前用户的好友列表信息
    if(!g_currentUserFriendList.empty()){
        for(User user:g_currentUserFriendList){
            cout<<user.GetId()<<"  "<<user.GetName()<<"  "<<user.GetState()<<endl;
        }
    }
    cout<<"____________________group list____________________"<<endl;//当前用户的群组列表信息
    if(!g_currentUserGroupList.empty()){
        for(Group &group:g_currentUserGroupList){
            cout<<group.GetId()<<"  "<<group.GetGroupname()<<"  "<<group.GetGroupdesc()<<endl;
            for(GroupUser &guser:group.getUsers()){
//没进来
                cout<<guser.GetId()<<"  "<<guser.GetName()<<"  "
                <<guser.GetState()<<"  "<<guser.GetRole()<<endl;
            }
        }
    }
    cout<<"__________________________________________________"<<endl;
}
//接收线程
void readTaskHandler(int clientfd){
    //不断等待消息传入并处理
    while(true){
        char buffer[1024]={0};
        int len =recv(clientfd,buffer,1024,0);
        if(len==-1||len==0){
            close(clientfd);
            exit(-1);
        }
        json js=json::parse(buffer);
        int msgid=js["msgid"];
        //登出时要退出循环结束接收线程
        if(msgid==LOGINOUT_MSG){
            break;
        }
        else if(msgid==ONE_CHAT_MSG){//一对一聊天
            //一对一聊天消息输出格式：time+[id-name]+said:...
            cout<<js["time"]<<"["<<js["from_id"]<<"-"<<js["from_name"]
            <<"] said: "<<js["msg"]<<endl;
            continue;
        }
        else if(msgid==GROUP_CHAT_MSG){//群组聊天
            //群组聊天消息输出格式：time+[groupid:...]+[memberid-membername]+said:...
            cout<<js["time"]<<"[group id:"<<js["from_groupid"]<<"] ["
            <<js["from_memberid"]<<"-"<<js["from_membername"]<<"] said: "<<js["msg"]<<endl;
            continue;
        }
        else if(msgid==ADD_GROUP_MSG){//加入群组
            if(js["errno"]==0){
                //加入成功
                cout<<"add group success!"<<endl;
            }
            else{
                //加入失败
                cout<<"add group fail! errmsg:"<<js["errmsg"]<<endl;
            }
            continue;
        }
        
    }
}
//获取系统时间（聊天信息需要添加时间信息）
string getCurrentTime(){
    auto tt = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

    struct tm *ptm = localtime(&tt);

    char date[60] = {0};

    sprintf(date, "%d-%02d-%02d %02d:%02d:%02d",

    (int)ptm->tm_year + 1900, (int)ptm->tm_mon + 1, (int)ptm->tm_mday,

    (int)ptm->tm_hour, (int)ptm->tm_min, (int)ptm->tm_sec);

    return std::string(date);
}

//登录业务
void _login(int clientfd){
    int id=0;
    char pwd[50]={0};
    cout<<"user id:";
    cin>>id;
    cin.get();//读掉缓冲区残留的回车
    cout<<"user password:";
    cin.getline(pwd,50);

    //组装json请求
    json js;
    js["msgid"]=LOGIN_MSG;
    js["id"]=id;
    js["password"]=pwd;
    
    //序列化json请求，发送给服务器
    string request=js.dump();
    int len=send(clientfd,request.c_str(),strlen(request.c_str())+1,0);
    if(len==-1){
        cerr<<"send login msg error:"<<request<<endl;
    }
    else{
        //发送成功，等待服务器回传信息
        char buffer[1024]={0};
        len=recv(clientfd,buffer,1024,0);
        if(len==-1){
            cerr<<"recv login response error"<<endl;
        }
        else{
            json responsejs=json::parse(buffer);//反序列化服务器回传信息
            if(responsejs["errno"]!=0){
                //登录失败
                cerr<<responsejs["errmsg"]<<endl;
            }
            else{
                //登录成功
                //1.记录当前系统登录的用户信息
                g_currentUser.SetId(responsejs["id"]);
                g_currentUser.SetName(responsejs["name"]);
                //2.记录当前用户的好友列表信息
                if(responsejs.contains("friends")){

                    //初始化g_currentUserFriendList（针对登出业务）
                    g_currentUserFriendList.clear();

                    for(json &friendjs:responsejs["friends"]){// JSON 数组 "friends"
                        User user;
                        user.SetId(friendjs["friend_id"]);
                        user.SetName(friendjs["friend_name"]);
                        user.SetState(friendjs["friend_state"]);
                        g_currentUserFriendList.push_back(user);
                    }
                }
                //3.记录当前用户的群组列表信息
                if(responsejs.contains("groups")){// JSON 数组 "groups"

                    //初始化g_currentUserGroupList（针对登出业务）
                    g_currentUserGroupList.clear();

                    for(json &groupjs:responsejs["groups"]){
                        Group group;
                        group.SetId(groupjs["group_id"]);
                        group.SetGroupname(groupjs["group_name"]);
                        group.SetGroupdesc(groupjs["group_desc"]);

                        // JSON 数组 "group_users"
                        for(json &guserjs:groupjs["group_users"]){
                            GroupUser user;
                            user.SetId(guserjs["id"]);
                            user.SetName(guserjs["name"]);
                            user.SetState(guserjs["state"]);
                            user.SetRole(guserjs["grouprole"]);
                            group.getUsers().push_back(user);
                        }
                        g_currentUserGroupList.push_back(group);
                    }
                }
                //显示当前登录成功用户的基本信息
                showCurrentUserData();
                //显示当前用户离线消息
                if(responsejs.contains("offline_msg")){
                    vector<string> vec=responsejs["offline_msg"];
                    for(string &str:vec){
                        json strjs=json::parse(str);//反序列化
                        //判断该离线消息为一对一聊天消息还是群组聊天消息
                        if(strjs["msgid"]==ONE_CHAT_MSG){
                            //一对一聊天消息输出格式：time+[id-name]+said:...
                            cout<<strjs["time"]<<"["<<strjs["from_id"]<<"-"<<strjs["from_name"]
                            <<"] said: "<<strjs["msg"]<<endl;
                        }
                        else{
                            //群组聊天消息输出格式：time+[groupid:...]+[memberid-membername]+said:...
                            cout<<strjs["time"]<<"[group id:"<<strjs["from_groupid"]<<"] ["
                            <<strjs["from_memberid"]<<"-"<<strjs["from_membername"]<<"] said: "<<strjs["msg"]<<endl;
                        }
                    }
                }
                //登录成功，启动接收线程负责接收数据
                std::thread readTask(readTaskHandler,clientfd);
                readTask.detach();
                //进入聊天主菜单页面
                Running=true;
                mainMenu(clientfd);
            }
        }
    }
}
//注册业务
void _register(int clientfd){
    char name[50]={0};
    char pwd[50]={0};
    cout<<"user name:";
    cin.getline(name,50);
    cout<<"user password:";
    cin.getline(pwd,50);
    
    //组装json请求
    json js;
    js["msgid"]=REG_MSG;
    js["name"]=name;
    js["password"]=pwd;

    //序列化json请求，发送给服务器
    string request=js.dump();
    int len=send(clientfd,request.c_str(),strlen(request.c_str())+1,0);
    if(len==-1){
        cerr<<"send reg msg error:"<<request<<endl;
    }
    else{
        char buffer[1024]={0};
        len=recv(clientfd,buffer,1024,0);
        if(len==-1){
            cerr<<"recv reg response error"<<endl;
        }
        else{
            json responsejs=json::parse(buffer);
            if(responsejs["errno"]!=0){
                cerr<<"register error!"<<endl;
            }
            else{
                cout<<name<<" register success , user id:"<<responsejs["id"]
                <<" , do not forget it!"<<endl;
            }
        }
    }
}

//显示首页面菜单（登录，注册，退出）
void showMenu(int clientfd){
    while(true){
        cout<<"______________________"<<endl;
        cout<<"1. login"<<endl;
        cout<<"2. register"<<endl;
        cout<<"3. quit"<<endl;
        cout<<"______________________"<<endl;
        int choice=0;
        //防止输入非int类型choice
        while(true){
            cout<<"choice:";
            cin>>choice;

            if(cin.fail()){
                //若输入非int类型
                cerr<<"input type error! try again!"<<endl;
                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
            }
            else{
                //输入类型正确，退出循环
                break;
            }
        }
        
        cin.get(); //读掉缓冲区残留的回车
        
        switch(choice)
        {
            case 1://登录业务
            {
                _login(clientfd);
                break;
            }
            case 2://注册业务
            {
                _register(clientfd);
                break;
            }
            case 3://退出业务
            {
                close(clientfd);
                exit(0);
            }
            default://输入有误
            {
                cerr<<"invalid input!"<<endl;
                break;
            }
        }
    }
}

//聊天主菜单页面程序（登录成功后）
void mainMenu(int clientfd){
    help();//显示所有支持的命令

    char buffer[1024]={0};
    while(Running){
        //读取用户输入
        cin.getline(buffer,1024);
        //存储用户输入
        string commandbuf(buffer);
        //用于存储用户输入的命令部分
        string command;
        //找:符号位置
        int idx=commandbuf.find(":");
        if(idx==-1){
            //若找不到: ，则用户输入即为命令部分
            command=commandbuf;
        }
        else{
            //若找到: ，则:前即为命令部分
            command=commandbuf.substr(0,idx);
        }
        //在客户端命令处理列表查找用户输入命令
        auto it=commandHandlerMap.find(command);
        if(it==commandHandlerMap.end()){
            //若用户输入命令不存在
            cerr<<"invalid input command!"<<endl;
            continue;
        }
        //调用相应命令的事件处理回调函数（符合开闭原则）
        it->second(clientfd,commandbuf.substr(idx+1,commandbuf.size()-idx));
        //参数：clientfd,用户输入中 命令后的部分
    }
}

//命令处理函数
void help(int clientfd,string str){//显示所有支持的命令
    cout<<"show command list >>>"<<endl;
    for(auto &p:commandMap){
        cout<<p.first<<" : "<<p.second<<endl;
    }
}
void chat(int clientfd,string str){//一对一聊天
    //用户输入中 命令后的部分为friendid:message
    int friendid=0;
    string message="";
    //找:符号位置
    int idx=str.find(":");
    if(idx==-1){
        //若找不到:
        cerr<<"invalid chat command!"<<endl;
        return;
    }
    
    //若找到: 
    friendid=atoi(str.substr(0,idx).c_str());
    message=str.substr(idx+1,str.size()-idx);

    //封装json请求
    json js;
    js["msgid"]=ONE_CHAT_MSG;
    js["id"]=g_currentUser.GetId();//己方id
    js["name"]=g_currentUser.GetName();//己方name
    js["toid"]=friendid;//对方id
    js["msg"]=message;
    js["time"]=getCurrentTime();//发送消息的时间
    //序列化json请求，发送给服务器
    string buffer=js.dump();

    int len=send(clientfd,buffer.c_str(),strlen(buffer.c_str())+1,0);
    if(len==-1){
        cerr<<"send chat msg error! buffer: "<<buffer<<endl;
    }
    
}
void addfriend(int clientfd,string str){//添加好友
    //用户输入中 命令后的部分即为friendid
    int friendid=atoi(str.c_str());
    //封装json请求
    json js;
    js["msgid"]=ADD_FRIEND_MSG;
    js["id"]=g_currentUser.GetId();//己方id
    js["toid"]=friendid;//对方id
    //序列化json请求，发送给服务器
    string buffer=js.dump();
    int len=send(clientfd,buffer.c_str(),strlen(buffer.c_str())+1,0);
    if(len==-1){
        cerr<<"send adddfriend msg error! buffer: "<<buffer<<endl;
    }
}
void creategroup(int clientfd,string str){//创建群组
    //用户输入中 命令后的部分为groupname:groupdesc
    string groupname="";
    string groupdesc="";

    //找:符号位置
    int idx=str.find(":");
    if(idx==-1){
        //若找不到:
        cerr<<"invalid creategroup command!"<<endl;
        return;
    }
    
    //若找到: 
    groupname=str.substr(0,idx);
    groupdesc=str.substr(idx+1,str.size()-idx);

    //封装json请求
    json js;
    js["msgid"]=CREATE_GROUP_MSG;
    js["userid"]=g_currentUser.GetId();//群组创建用户id
    js["groupname"]=groupname;//群组名
    js["groupdesc"]=groupdesc;//群组功能描述
    //序列化json请求，发送给服务器
    string buffer=js.dump();
    int len=send(clientfd,buffer.c_str(),strlen(buffer.c_str())+1,0);
    if(len==-1){
        cerr<<"send creategroup msg error! buffer: "<<buffer<<endl;
    }
}
void addgroup(int clientfd,string str){//加入群组
    //用户输入中 命令后的部分即为groupid
    int groupid=atoi(str.c_str());
    //封装json请求
    json js;
    js["msgid"]=ADD_GROUP_MSG;
    js["userid"]=g_currentUser.GetId();
    js["groupid"]=groupid;
    //序列化json请求，发送给服务器
    string buffer=js.dump();
    int len=send(clientfd,buffer.c_str(),strlen(buffer.c_str())+1,0);
    if(len==-1){
        cerr<<"send adddgroup msg error! buffer: "<<buffer<<endl;
    }
}
void groupchat(int clientfd,string str){//群聊
    //用户输入中 命令后的部分为groupid:message
    int groupid=0;
    string message="";

    //找:符号位置
    int idx=str.find(":");
    if(idx==-1){
        //若找不到:
        cerr<<"invalid groupchat command!"<<endl;
        return;
    }
    
    //若找到: 
    groupid=atoi(str.substr(0,idx).c_str());
    message=str.substr(idx+1,str.size()-idx);

    //封装json请求
    json js;
    js["time"]=getCurrentTime();
    js["name"]=g_currentUser.GetName();
    js["msgid"]=GROUP_CHAT_MSG;
    js["userid"]=g_currentUser.GetId();
    js["groupid"]=groupid;
    js["msg"]=message;
    //序列化json请求，发送给服务器
    string buffer=js.dump();
    int len=send(clientfd,buffer.c_str(),strlen(buffer.c_str())+1,0);
    if(len==-1){
        cerr<<"send groupchat msg error! buffer: "<<buffer<<endl;
    }
}
void loginout(int clientfd,string str){//登出
    //封装json请求
    json js;
    js["msgid"]=LOGINOUT_MSG;
    js["userid"]=g_currentUser.GetId();
    //序列化json请求，发送给服务器
    string buffer=js.dump();
    int len=send(clientfd,buffer.c_str(),strlen(buffer.c_str())+1,0);
    if(len==-1){
        cerr<<"send loginout msg error! buffer: "<<buffer<<endl;
    }
    else{
        //设置Running为false退出主菜单页面
        Running =false;
    }
}
