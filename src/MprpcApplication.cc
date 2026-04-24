#include "MprpcApplication.h"
#include <iostream>
#include <unistd.h>
#include <string>

using std::string;
using std::cout;
using std::endl;

MprpcConfig MprpcApplication::m_config;

// 打印错误信息
void ShowArgsHelp(){
    cout<<"format: command -i <configfile>"<<endl;
}

// 根据配置文件初始化框架
void MprpcApplication::Init(int argc,char **argv){
    if(argc<2){
        ShowArgsHelp();
        exit(EXIT_FAILURE);
    }
    int c=0;
    string config_file; // 配置文件的名称
    // 调用getopt解析程序的参数,"i:"表示选项i必须有一个参数
    while((c=getopt(argc,argv,"i:"))!=-1){
        switch (c)
        {
            // 返回'i',说明读取到了"-i"选项,后面的参数作为配置文件的名称
            case 'i':
                config_file=optarg;
                break;
            // 返回'?',说明读取到了未知选项
            case '?':
                ShowArgsHelp();
                exit(EXIT_FAILURE);
            // 返回':',说明缺少必需参数
            case ':':
                ShowArgsHelp();
                exit(EXIT_FAILURE);
            default:
                break;
        }
    }
    // 开始加载配置文件
    m_config.LoadConfigFile(config_file.c_str());
    /*
    cout<<"rpcserverip = "<<m_config.Load("rpcserverip")<<endl;
    cout<<"rpcserverport = "<<m_config.Load("rpcserverport")<<endl;
    cout<<"zookeeperip = "<<m_config.Load("zookeeperip")<<endl;
    cout<<"zookeeperport = "<<m_config.Load("zookeeperport")<<endl;
    */
}

// 获取唯一的实例
MprpcApplication& MprpcApplication::GetInstance(){
    static MprpcApplication app;
    return app;
}

// 获取MprpcConfig对象
MprpcConfig& MprpcApplication::GetConfig(){
    return m_config;
}