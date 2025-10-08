#include "chatserver.hpp"
#include "json.hpp"
#include "chatservice.hpp"
#include <functional>
#include <string>
using namespace std;
using namespace placeholders;
using json=nlohmann::json;

ChatServer::ChatServer(EventLoop* loop,//事件循环
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

void ChatServer::start(){
    _server.start();
}

void ChatServer::onConnection(const TcpConnectionPtr& conn){
    //客户建立连接
    if(conn->connected()){

    }
    //客户断开连接
    else{
        ChatService::getInstance().clientCloseException(conn);//处理客户端异常断开

        conn->shutdown();
    }
}
void ChatServer::onMessage(const TcpConnectionPtr& conn,//连接
                            Buffer* buffer,//缓冲区
                            Timestamp tim)//接收到数据时的时间信息
{
    //取出缓冲区所有数据并返回为std::string
    string buf=buffer->retrieveAllAsString();
    //将字符串解析为JSON对象（数据的反序列化）
    json js=json::parse(buf);

    //一个消息id对应一个事件处理，需要解耦网络模块和业务模块（传递参数conn,js,tim）

    auto msgHandler=ChatService::getInstance().getHandler(js["msgid"].get<int>());
    //ChatService::getInstance()返回一个对象实例，将js中"msgid"键对应值转换为int类型，传入getHandler获取对应事件处理函数msgHandler
    
    //进行业务处理
    msgHandler(conn,js,tim);

}