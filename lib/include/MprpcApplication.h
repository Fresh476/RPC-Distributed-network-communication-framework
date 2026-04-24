#pragma once

#include "MprpcChannel.h"
#include "MprpcController.h"
#include "MprpcConfig.h"

// mprpc框架的基础类(单例模式),负责框架的初始化操作
class MprpcApplication{
public:
    static void Init(int argc,char** argv); // 根据配置文件初始化框架
    static MprpcApplication &GetInstance(); // 获取唯一的实例
    static MprpcConfig& GetConfig();
private:
    MprpcApplication(){}
    MprpcApplication(const MprpcApplication &)=delete;
    MprpcApplication(const MprpcApplication &&)=delete;
    MprpcApplication operator=(const MprpcApplication &)=delete;
private:
    static MprpcConfig m_config; // 负责解析配置文件
};