#pragma once

#include "user.hpp"

//群组用户，继承自User，只多一个grouprole组内角色信息
class GroupUser:public User{
public:
    void SetRole(string role){grouprole=role;}
    string GetRole(){return grouprole;}
private:
    string grouprole;
};