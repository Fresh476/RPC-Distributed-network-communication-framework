#include <iostream>
#include "MprpcApplication.h"
#include "friend.pb.h"

using namespace MyRPC;
using std::cout;
using std::endl;

int main(int argc,char **argv){
    MprpcApplication::Init(argc,argv);
    // 定义Stub代理对象,用RpcChannel初始化,基类RpcChannel封装了CallMethod()方法
    FriendServiceRpc_Stub stub(new MprpcChannel());
    // 定义并填写参数信息
    GetFriendsListRequest request;
    request.set_userid(2026);
    // 定义响应数据,等待对方填写
    GetFriendsListResponse response;
    // 发起RPC调用,传入RpcController(暂时置空) request response Closure(暂时置空)
    MprpcController controller;
    stub.GetFriendsList(&controller,&request,&response,nullptr);
    // 完成RPC调用,读取结果
    if(controller.Failed()){
        // 本次RPC调用过程中出错
        cout<<"controller: "<<controller.ErrorText()<<endl;
    }
    else{
        // RPC调用成功
        if(response.result().errcode()==0){
            cout<<"rpc GetFriendsList request success!"<<endl;
            int result_size=response.friends_size();
            for(int i=0;i<result_size;i++){
                cout<<"FriendName: "<<response.friends(i)<<endl;
            }
        }
        else{
            cout<<"rpc GetFriendsList request error: "<<response.result().errmsg()<<endl;
        }
    }
    return 0;
}