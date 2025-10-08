//网络模块

#pragma once

#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>
using namespace muduo;
using namespace muduo::net;

//聊天服务器的主类
class ChatServer{
public:
    //初始化ChatServer
    ChatServer(EventLoop* loop,
            const InetAddress& listenAddr,
            const string& nameArg);
    //启动服务
    void start();
private:
    TcpServer _server;
    EventLoop *_loop;
    //专门处理用户连接的创建和断开的回调函数
    void onConnection(const TcpConnectionPtr& conn);
    //专门处理用户读写事件的回调函数
    void onMessage(const TcpConnectionPtr& conn,
                            Buffer* buffer,
                            Timestamp tim);
};