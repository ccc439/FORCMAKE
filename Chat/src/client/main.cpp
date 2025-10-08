#include "client.hpp"

//聊天客户端程序实现（main线程作为发送线程，子线程作为接收线程）
int main(int argc,char ** argv){
    
    //需要传两个参数：服务器IP+负载均衡器port
    if(argc<3){
        cerr<<"command invalid! example:./ChatClient 127.0.0.1 8000"<<endl;
        exit(-1);
    }
    
    //获取命令行传递的ip和port
    char* ip=argv[1];
    uint16_t port=atoi(argv[2]);

    //创建client端的socket
    int clientfd=socket(AF_INET,SOCK_STREAM,0);
    if(clientfd==-1){
        cerr<<"socket create error"<<endl;
        exit(-1);
    }

    //填写client需要连接的server信息，ip+port
    sockaddr_in server;
    memset(&server,0,sizeof(sockaddr_in));

    server.sin_family=AF_INET;
    server.sin_port=htons(port);
    server.sin_addr.s_addr=inet_addr(ip);

    //client和server进行连接
    if(connect(clientfd,(sockaddr*)&server,sizeof(sockaddr_in))==-1){
        cerr<<"connect server error"<<endl;
        close(clientfd);
        exit(-1);
    }

    //main线程用于接收用户输入，负责发送数据
    showMenu(clientfd);
    

    return 0;
}