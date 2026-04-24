// RPC方法的调用者(客户端)
#include <iostream>
#include "MprpcApplication.h"
#include "user.pb.h"
#include "MprpcChannel.h"

using namespace MyRPC;
using std::cout;
using std::endl;

int main(int argc,char** argv){
    // 启动框架初始化(只初始化一次)
    MprpcApplication::Init(argc,argv);
    // 演示调用远程发布的RPC方法Login
    // UserServiceRpc_Stub类与UserService类相对应,相当于客户端-服务器
    // 发起RPC调用的一方是客户端,通过Stub类完成远程调用,其内部封装了网络通信、发起请求、等待响应等细节
    // Stub对象需要用RpcChannel对象初始化,由RpcChannel对象负责执行CallMethod()
    // RpcChannel是一个抽象基类,需要自定义一个继承自RpcChannel的子类并重写其虚函数

    // 定义Stub代理对象,用RpcChannel初始化,基类RpcChannel封装了CallMethod()方法
    UserServiceRpc_Stub stub(new MprpcChannel());
    // 定义并填写参数信息
    LoginRequest request;
    request.set_name("zhang san");
    request.set_pwd("123456");
    // 定义响应数据,等待对方填写
    LoginResponse response;
    // 发起RPC调用,传入RpcController(暂时置空) request response Closure(暂时置空)
    stub.Login(nullptr,&request,&response,nullptr);
    // 完成RPC调用,读取结果
    if(response.result().errcode()==0){
        cout<<"rpc login request success: "<<response.success()<<endl;
    }
    else{
        cout<<"rpc login request error: "<<response.result().errmsg()<<endl;
    }

    // 演示调用远程发布的RPC方法Register
    RegisterRequest req;
    req.set_id(2026);
    req.set_name("mprpc");
    req.set_pwd("666666");
    RegisterResponse rsp;
    stub.Register(nullptr,&req,&rsp,nullptr);
    if(rsp.result().errcode()==0){
        cout<<"rpc login request success: "<<rsp.success()<<endl;
    }
    else{
        cout<<"rpc login request error: "<<rsp.result().errmsg()<<endl;
    }
    return 0;
}