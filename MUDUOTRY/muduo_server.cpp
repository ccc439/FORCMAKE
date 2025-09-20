/*
muduo网络库给用户提供了两个主要的类
TcpServer：用于编写服务器程序
TcpClient：用于编写客户端程序

以便专注于业务代码（1.用户的连接和断开 2.用户的可读写事件）而不是网络I/O代码（网络库管理）
*/
#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>
#include <iostream>
#include <string>
#include <functional>
 
using namespace std;
using namespace muduo;
using namespace muduo::net;
using namespace placeholders;
/*
基于muduo网络库开发服务器程序
1.创建TcpServer对象
2.创建EventLoop对象指针
3.确定TcpServer构造函数需要的参数，写ChatServer的构造函数
4.在ChatServer的构造函数中，注册处理（1.用户的连接和断开 2.用户的可读写事件）的回调函数
5.设置服务器的线程数量（muduo会自适应，分配I/O线程和工作线程）
*/


class ChatServer{
public:
    ChatServer(EventLoop* loop,//事件循环
            const InetAddress& listenAddr,//IP+port
            const string& nameArg)//服务器的名字
            :_server(loop,listenAddr,nameArg)
            ,_loop(loop)
            {
                //给服务器注册用户连接的创建和断开回调
                _server.setConnectionCallback(bind(&ChatServer::onConnection,this,_1));
                //给服务器注册用户读写事件回调
                _server.setMessageCallback(bind(&ChatServer::onMessage,this,_1,_2,_3));
                //设置服务器的线程数量
                _server.setThreadNum(4);//1个I/O线程，3个工作线程
            }
    //开始事件循环
    void start(){
        _server.start();
    }
private:
    TcpServer _server;
    EventLoop *_loop;//~epoll

//专注于以下业务代码!

    //专门处理用户连接的创建和断开
    void onConnection(const TcpConnectionPtr& conn){
        //用户连接
        if(conn->connected()){
            cout<<"客户端IP："<<conn->peerAddress().toIp()<<" 端口："<<conn->peerAddress().port()
            <<"-->"<<"服务器IP："<<conn->localAddress().toIp()<<" 端口："<<conn->localAddress().port()
            <<endl;
            cout<<"state:online"<<endl;
        }
        //用户断开
        else{
            cout<<"客户端IP："<<conn->peerAddress().toIp()<<" 端口："<<conn->peerAddress().port()
            <<"-->"<<"服务器IP："<<conn->localAddress().toIp()<<" 端口："<<conn->localAddress().port()
            <<endl;
            cout<<"state:offline"<<endl;
            conn->shutdown();//~close(fd)
        }
    }
    //专门处理用户读写事件
    void onMessage(const TcpConnectionPtr& conn,//连接
                            Buffer* buffer,//缓冲区
                            Timestamp tim)///接收到数据时的时间信息
    {
        string buf=buffer->retrieveAllAsString();
        cout<<"recv data:"<<buf<<" time:"<<tim.toString()<<endl;
        conn->send(buf);
    }
};

int main(){
    EventLoop loop;
    InetAddress addr("127.0.0.1",6000);
    ChatServer server(&loop,addr,"ChatServer");
    server.start();
    loop.loop();//~epoll_wait 以阻塞方式等待（1.用户的连接和断开 2.用户的可读写事件）

    return 0;
}