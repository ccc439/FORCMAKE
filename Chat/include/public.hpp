//server和client的公共文件
#pragma once

enum EnMsgType{
    //登录的消息id（从1开始）
    LOGIN_MSG=1,
    //注册的消息id
    REG_MSG,
    //一对一聊天的消息id
    ONE_CHAT_MSG,
    //添加好友的消息id
    ADD_FRIEND_MSG,
    //创建群组的消息id
    CREATE_GROUP_MSG,
    //加入群组的消息id
    ADD_GROUP_MSG,
    //群聊天的消息id
    GROUP_CHAT_MSG,
    //登出的消息id
    LOGINOUT_MSG,
};