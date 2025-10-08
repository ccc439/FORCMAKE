#pragma once
#include <string>
#include <vector>
using namespace std;


//offline_messages表（离线消息表）的数据操作类（增删改查）
class OfflineMsgModel{
public:
    //存储用户离线消息（userid：用户id，msg：消息）
    bool insert(int userid,string msg);

    //删除用户离线消息
    bool remove(int userid);

    //查询用户离线消息
    vector<string> query(int userid);

private:

};