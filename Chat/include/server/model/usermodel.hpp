#pragma once

#include "user.hpp"

//User表的数据操作类（增删改查）
class UserModel{
public:
    //增加User
    bool insert(User &user);
    //根据id查找User
    User query(int id);
    //更新User的state（需要user.GetState(),user.GetId()）
    bool updateState(User &user);
    //重置所有online1用户的状态为offline
    bool resetState();
private:

};