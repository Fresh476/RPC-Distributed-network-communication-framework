#include <iostream>
#include <string>
#include <vector>
#include "friend.pb.h"
#include "MprpcApplication.h"
#include "RpcProvider.h"
#include "Logger.h"

using namespace MyRPC;
using std::cout;
using std::endl;
using std::string;
using std::vector;

// 好友服务模块
class FriendService:public MyRPC::FriendServiceRpc{
public:
    // 本地业务:查询指定userid的好友列表
    vector<string> GetFriendsList(uint32_t userid){
        cout<<"do local service: GetFriendsList(),userid = "<<userid<<endl;
        vector<string>vec;
        vec.push_back("gao yang");
        vec.push_back("zhang san");
        vec.push_back("li si");
        vec.push_back("wang wu");
        return vec;
    }
    // 重写基类方法,实现远程调用
    void GetFriendsList(google::protobuf::RpcController* controller,
        const GetFriendsListRequest* request,
        GetFriendsListResponse* response,
        google::protobuf::Closure* done){
        // 1.提取参数
        uint32_t userid=request->userid();
        // 2.做本地业务
        vector<string>friendsList=GetFriendsList(userid);
        // 3.填写响应信息
        response->mutable_result()->set_errcode(0);
        response->mutable_result()->set_errmsg("");
        for(string &name:friendsList){
            string *p=response->add_friends();
            *p=name;
        }
        // 4.通知回调对象发送
        done->Run();
    }
};

int main(int argc,char **argv){
    LOG_INFO("first log message!");
    //LOG_ERR("%s:%s:%d",__FILE__,__FUNCTION__,__LINE__);
    MprpcApplication::Init(argc,argv);
    RpcProvider provider;
    provider.NotifyService(new FriendService()); // 新建FriendService服务对象进行管理
    provider.Run(); // 启动RPC服务,阻塞等待RPC请求
    return 0;
}