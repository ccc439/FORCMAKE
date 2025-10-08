#pragma once
#include <hiredis/hiredis.h>
#include <thread>
#include <functional>
#include <string>
using namespace std;

class Redis{
public:
    Redis();
    ~Redis();

    //连接redis服务器
    bool connect();
    
    //向redis指定的频道channel发布消息
    bool publish(int channel,string message);
    
    //向redis指定的频道subscribe订阅消息
    bool subscribe(int channel);

    //向redis指定的频道unsubscribe取消订阅消息
    bool unsubscribe(int channel);

    //在独立线程中接收订阅频道中的消息
    void observer_channel_message();

    //初始化向业务层上报频道消息的回调对象
    void init_notify_handler(function<void(int,string)> fn);
private:
    //独立的 Redis 连接，用于 PUBLISH（避免与 SUBSCRIBE 混用导致阻塞）
    redisContext *_publish_context;

    //独立的 Redis 连接，用于 SUBSCRIBE 和 UNSUBSCRIBE
    redisContext *_subscribe_context;
    
    //回调操作，收到订阅的消息，给service层上报
    function<void(int,string)> _notify_message_handler;
};