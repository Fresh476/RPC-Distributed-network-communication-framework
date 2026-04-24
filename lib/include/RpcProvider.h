#pragma once
#include "google/protobuf/service.h"
#include "muduo/net/TcpServer.h"
#include "muduo/net/EventLoop.h"
#include "muduo/net/InetAddress.h"
#include "muduo/net/TcpConnection.h"
#include "muduo/base/Timestamp.h"
#include <string>
#include <functional>
#include <unordered_map>
#include <iostream>
#include <google/protobuf/descriptor.h>
#include "MprpcApplication.h"

using namespace muduo;
using namespace muduo::net;

using std::cout;
using std::endl;
using std::string;
using std::function;
using std::unordered_map;
using std::bind;
using std::placeholders::_1;
using std::placeholders::_2;
using std::placeholders::_3;

// 专门负责发布RPC服务的网络对象类
// 由于所有的服务(如UserService)都是从基类Service继承而来,故该类一律使用Service基类指针进行操作
class RpcProvider{
public:
    void NotifyService(google::protobuf::Service *service); // 发布RPC方法的接口
    void Run(); // 启动RPC服务节点,开始提供RPC远程网络调用服务
private:
    void onConnection(const TcpConnectionPtr&); // 连接回调
    void onMessage(const TcpConnectionPtr&, Buffer*, Timestamp); // 读写消息回调
    // Closure的回调操作,用于序列化RPC的响应和网络发送
    void SendRpcResponse(const TcpConnectionPtr &,google::protobuf::Message*); 
private:
    EventLoop m_eventloop; // mainLoop
    // Service服务类型信息
    struct ServiceInfo{
        google::protobuf::Service *m_service; // 保存服务对象
        // 方法映射表,保存该服务对象的所有RPC方法 <方法名,方法指针>
        unordered_map<string,const google::protobuf::MethodDescriptor*> m_methodMap; 
    };
    // 服务映射表,保存当前RpcProvider所管理的所有服务对象 <服务名,服务对象信息>
    unordered_map<string,ServiceInfo> m_serviceMap; 
};