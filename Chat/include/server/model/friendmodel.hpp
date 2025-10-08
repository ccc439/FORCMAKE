#pragma once
#include <vector>
#include "user.hpp"
using namespace std;

//Friend表的数据操作类（增删改查）
class FriendModel{
public:
    //添加好友关系（单向好友）
    bool insert(int userid,int friendid);
    //返回用户好友列表
    vector<User> query(int userid);
private:

};