//业务模块

#pragma once

#include "json.hpp"
#include <muduo/net/TcpConnection.h>
#include <unordered_map>
#include <functional>
#include <mutex>
#include "usermodel.hpp"
#include "offlinemsgmodel.hpp"
#include "friendmodel.hpp"
#include "groupmodel.hpp"
#include "redis.hpp"
using namespace std;
using namespace muduo;
using namespace muduo::net;
using json=nlohmann::json;

//消息id对应的事件回调函数类型
using MsgHandler=std::function<void(const TcpConnectionPtr &conn,json &js,Timestamp tim)>;


//聊天服务器的业务类（单例模式：C++11静态初始化，线程安全版本）
class ChatService{
public:
    //提供静态对外接口（单例模式）
    static ChatService& getInstance();

    //处理登录业务
    void login(const TcpConnectionPtr &conn,json &js,Timestamp tim);
    //处理注册业务
    void reg(const TcpConnectionPtr &conn,json &js,Timestamp tim);
    //处理一对一聊天业务
    void oneChat(const TcpConnectionPtr &conn,json &js,Timestamp tim);
    //处理添加好友业务
    void addFriend(const TcpConnectionPtr &conn,json &js,Timestamp tim);
    //处理创建群组业务
    void createGroup(const TcpConnectionPtr &conn,json &js,Timestamp tim);
    //处理加入群组业务
    void addGroup(const TcpConnectionPtr &conn,json &js,Timestamp tim);
    //处理群聊天业务
    void groupChat(const TcpConnectionPtr &conn,json &js,Timestamp tim);
    //处理登出业务
    void loginout(const TcpConnectionPtr &conn,json &js,Timestamp tim);

    //获取消息id对应的事件回调函数
    MsgHandler getHandler(int msgid);

    //处理客户端异常断开
    void clientCloseException(const TcpConnectionPtr &conn);

    //服务器异常断开（如Ctrl+C），重置业务
    void reset();

    // 从redis消息队列中获取订阅的消息
    void handleRedisSubscribeMessage(int userid, string msg);

private:
    //构造函数私有化（单例模式）
    ChatService();
    ChatService(const ChatService&)=delete;
    ChatService& operator=(const ChatService&)=delete;
    
    //消息处理器表<消息id，对应业务处理方法>
    unordered_map<int,MsgHandler> _MsgHandlerMap;

    //数据库操作类对象
    UserModel _userModel;
    OfflineMsgModel _offlineMsgModel;
    FriendModel _friendModel;
    GroupModel _groupModel;

    //存储在线用户的通信连接<用户id，通信连接>
    unordered_map<int,TcpConnectionPtr> _userConnMap;

    //互斥锁，保证_userConnMap多线程安全
    mutex _connmutex;

    //redis操作对象
    Redis _redis;
};