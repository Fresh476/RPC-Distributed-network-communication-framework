#pragma once

#include "google/protobuf/service.h"
#include "google/protobuf/descriptor.h"
#include "google/protobuf/message.h"

/*
    request格式:
    1.header_size(数据头部长度)
    2.RpcHeader(请求头)
    3.args(序列化的参数)
*/
/*
    RpcHeader格式:
    1.service_name(服务名称)
    2.method_name(方法名称)
    3.args_size(参数长度)
*/

// 定义MprpcChannel类,继承自RpcChannel,重写其虚函数
class MprpcChannel:public google::protobuf::RpcChannel{
public:
    // 所有通过Stub代理对象调用的RPC方法,都通过CallMethod()完成数据序列化和网络发送
    void CallMethod(
        const google::protobuf::MethodDescriptor *method, // 要调用的方法
        google::protobuf::RpcController *controller, // Rpc管理器
        const google::protobuf::Message *request, // 参数信息
        google::protobuf::Message *response, // 响应信息
        google::protobuf::Closure *done); // 回调对象
};