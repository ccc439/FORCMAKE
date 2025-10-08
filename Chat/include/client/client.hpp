#pragma once
#include "json.hpp"
#include <thread>
#include <vector>
#include <chrono>
#include <ctime>
using json=nlohmann::json;
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iostream>
#include <string>
using namespace std;
#include "user.hpp"
#include "group.hpp"
#include "public.hpp"

//记录当前系统登录的用户信息
inline User g_currentUser;
//记录当前用户的好友列表信息
inline vector<User> g_currentUserFriendList;
//记录当前用户的群组列表信息
inline vector<Group> g_currentUserGroupList;
//显示当前登录成功用户的基本信息
void showCurrentUserData();

//接收线程
void readTaskHandler(int clientfd);
//获取系统时间（聊天信息需要添加时间信息）
string getCurrentTime();
//登录业务
void _login(int clientfd);
//注册业务
void _register(int clientfd);
//显示首页面菜单（登录，注册，退出）
void showMenu(int clientfd);
//聊天主菜单页面程序（登录成功后）
void mainMenu(int clientfd);
//控制主菜单页面程序（用于登出业务）
inline bool Running=false;
//系统支持的客户端命令列表
inline unordered_map<string,string> commandMap={
    {"help","显示所有支持的命令，格式 help"},
    {"chat","一对一聊天，格式 chat:friendid:message"},
    {"addfriend","添加好友，格式 addfriend:friendid"},
    {"creategroup","创建群组，格式 create:groupname:groupdesc"},
    {"addgroup","加入群组，格式 addgroup:groupid"},
    {"groupchat","群聊，格式 groupchat:groupid:message"},
    {"loginout","登出，格式 loginout"}
};

//命令处理函数
void help(int clientfd=0,string str="");//显示所有支持的命令
void chat(int clientfd,string str);//一对一聊天
void addfriend(int clientfd,string str);//添加好友
void creategroup(int clientfd,string str);//创建群组
void addgroup(int clientfd,string str);//加入群组
void groupchat(int clientfd,string str);//群聊
void loginout(int clientfd,string str="");//登出

//系统支持的客户端命令处理
inline unordered_map<string,function<void(int,string)>> commandHandlerMap={
    {"help",help},
    {"chat",chat},
    {"addfriend",addfriend},
    {"creategroup",creategroup},
    {"addgroup",addgroup},
    {"groupchat",groupchat},
    {"loginout",loginout}
};
