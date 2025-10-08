#pragma once

#include "group.hpp"

//AllGroup和GroupUser表的数据操作类（增删改查）
class GroupModel{
public:
    //创建群组
    bool createGroup(Group &group);
    //查询群组是否不存在
    bool isGroupNotExist(int groupid);
    //加入群组
    bool addGroup(int userid,int groupid,string role);
    //查询用户所在群组信息
    vector<Group> queryGroups(int userid);
    //根据指定的groupid查询群组用户id列表，除userid自己，主要用于用户群聊业务给群组其他成员群发消息
    vector<int> queryGroupUsers(int userid,int groupid);
private:
};