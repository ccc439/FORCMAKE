#include "chatserver.hpp"
#include "chatservice.hpp"
#include <iostream>
#include <signal.h>
using namespace std;

void resetHandler(int){
    ChatService::getInstance().reset();
    exit(0);
}

int main(int argc,char ** argv){

    //需要传两个参数：服务器IP+服务器port
    if(argc<3){
        cerr<<"command invalid! example:./ChatServer 127.0.0.1 6000"<<endl;
        exit(-1);
    }
    
    //获取命令行传递的ip和port
    char* ip=argv[1];
    uint16_t port=atoi(argv[2]);

    signal(SIGINT,resetHandler);

    EventLoop loop;
    InetAddress addr(ip,port);
    ChatServer server(&loop,addr,"ChatServer");
    server.start();
    loop.loop();
    
    return 0;
}