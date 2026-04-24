// RPC方法的提供者(服务器)
#include <iostream>
#include <string>
#include "user.pb.h"
#include "MprpcApplication.h"
#include "RpcProvider.h"

using namespace MyRPC;
using std::cout;
using std::endl;
using std::string;

// UserService原本是一个本地服务,提供两个本地方法,需要将其改为支持rpc调用
// 修改方法: 1.让该类继承自UserServiceRpc类,该类于user.proto中定义,由服务端发布者继承
// 2.重写该类的对应方法
class UserService:public MyRPC::UserServiceRpc{
public:
    // 本地的Login方法
    bool Login(string name,string pwd){
        cout<<"doing local service: Login()"<<endl;
        cout<<"name: "<<name<<" pwd: "<<pwd<<endl;
        return true;
    }
    // 本地的Register方法
    bool Register(uint32_t id,string name,string pwd){
        cout<<"doing local service: Register()"<<endl;
        cout<<"id: "<<id<<", name: "<<name<<", pwd: "<<pwd<<endl;
        return true;
    }
    /*
        以下是UserServiceRpc类中提供的虚函数,由框架直接调用
        远端发起Login()请求,填写相关参数,通过底层Muduo网络库将请求发送到被调用方
        RPC框架接收该请求,执行虚函数
        虚函数Login有四个参数:
        1.controller是RPC控制器,控制RPC调用过程,管理调用状态
        2.request是请求参数,封装调用方准备好的参数
        3.response是响应参数,由被调用方执行完毕后填写相关的响应信息
        4.done是回调对象,被调用方执行完毕后需要调用其Run()方法通知RPC框架发送响应
    */
    void Login(google::protobuf::RpcController* controller,
        const ::MyRPC::LoginRequest* request,
        ::MyRPC::LoginResponse* response,
        ::google::protobuf::Closure* done){
        // 1.从protobuf中解析出参数信息
        string name=request->name();
        string pwd=request->pwd();
        // 2.做本地业务
        bool login_result=Login(name,pwd);
        // 3.填写响应信息
        ResultCode *code=response->mutable_result();
        code->set_errcode(0);
        code->set_errmsg("no error");
        response->set_success(login_result);
        // 4.执行回调操作,Run()方法的内部将执行响应消息的序列化和发送操作,由框架完成
        done->Run();
    }
    void Register(google::protobuf::RpcController* controller,
        const ::MyRPC::RegisterRequest* request,
        MyRPC::RegisterResponse* response,
        google::protobuf::Closure* done){
        cout<<"start RPC::Register()."<<endl;
        // 1.解析参数
        uint32_t id=request->id();
        string name=request->name();
        string pwd=request->pwd();
        cout<<"id = "<<id<<endl;
        cout<<"name = "<<name<<endl;
        cout<<"pwd = "<<pwd<<endl;
        // 2.执行本地方法
        bool ret=Register(id,name,pwd);
        // 3.填写响应结果
        response->mutable_result()->set_errcode(0);
        response->mutable_result()->set_errmsg("");
        response->set_success(ret);
        // 4.通过回调对象进行发送
        done->Run();
        cout<<"RPC::Register() end."<<endl;
    }
private:

};

int main(int argc,char **argv){
    // 调用框架的初始化操作
    MprpcApplication::Init(argc,argv);
    RpcProvider provider;
    provider.NotifyService(new UserService()); // 新建UserService服务对象进行管理
    provider.Run(); // 启动RPC服务,阻塞等待RPC请求
    return 0;
}