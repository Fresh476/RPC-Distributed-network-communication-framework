#include "RpcHeader.pb.h"
#include "MprpcApplication.h"
#include "ZookeeperUtil.h"
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>

using namespace mprpc;

using std::string;
using std::cout;
using std::endl;

// 发起RPC调用请求
void MprpcChannel::CallMethod(
    const google::protobuf::MethodDescriptor *method,
    google::protobuf::RpcController *controller, 
    const google::protobuf::Message *request, 
    google::protobuf::Message *response, 
    google::protobuf::Closure *done){
        const google::protobuf::ServiceDescriptor* sd=method->service(); // 获取服务对象
        string service_name=sd->name(); // 获取服务名称
        string method_name=method->name(); // 获取方法名称
        // 获取参数的序列化字符串
        uint32_t args_size = 0; // 参数长度
        string args_str;
        if(request->SerializeToString(&args_str)){
            args_size=args_str.size();
        }
        else{
            controller->SetFailed("serialize request error!");
            return;
        }
        // 定义RPC请求的头部信息RpcHeader
        RpcHeader rpcHeader;
        rpcHeader.set_service_name(service_name);
        rpcHeader.set_method_name(method_name);
        rpcHeader.set_args_size(args_size);
        uint32_t header_size=0; // RPC头部长度
        string rpc_header_str; // 序列化后的rpcHeader
        if(rpcHeader.SerializeToString(&rpc_header_str)){
            header_size=rpc_header_str.size(); // 序列化成功,填写头部长度
        }
        else{
            controller->SetFailed("serialize rpc header error!");
            return;
        }
        // 组织最终发送的rpc请求
        string send_rpc_str;
        send_rpc_str.insert(0,string((char*)&header_size,4)); // 头部4字节写入header_size
        send_rpc_str+=rpc_header_str; // 然后拼接上rpc头部信息
        send_rpc_str+=args_str; // 然后拼接上参数信息
        cout<<"=========debug========="<<endl;
        cout<<"header_size: "<<header_size<<endl;
        cout<<"rpc_header_str: "<<rpc_header_str<<endl;
        cout<<"service_name: "<<service_name<<endl;
        cout<<"method_name: "<<method_name<<endl;
        cout<<"args_size: "<<args_size<<endl;
        cout<<"args_str: "<<args_str<<endl;
        cout<<"========================"<<endl;
        // TCP编程完成RPC方法的远程调用
        int clientfd=socket(AF_INET,SOCK_STREAM,0);
        if(clientfd==-1){
            char errtxt[512]={0};
            sprintf(errtxt,"create socket error! errno: %d",errno);
            controller->SetFailed(errtxt);
            return;
        }

        // 通过ZkClient查询该RPC方法所在的主机ip和port
        ZkClient ZkCli;
        ZkCli.Start();
        string method_path="/"+service_name+"/"+method_name; // 组装待查询的方法的路径名
        string host_data=ZkCli.GetData(method_path.c_str()); // 通过Zk客户端查询该方法的ip和port信息
        // 如果Zk上不存在该路径,报错
        if(host_data==""){
            controller->SetFailed(method_path+" is not exist!");
            return;
        }
        int idx=host_data.find(":"); // 查找分隔符,前面的信息是ip,后面的信息是port
        // 查不到,无效信息
        if(idx==-1){
            controller->SetFailed(method_path+" address is invalid!");
            return;
        }
        string ip=host_data.substr(0,idx); // 提取出ip
        uint16_t port=atoi(host_data.substr(idx+1,host_data.size()-idx).c_str()); // 提取出port 
        
        struct sockaddr_in serv_addr;
        memset(&serv_addr,0,sizeof(serv_addr));
        serv_addr.sin_family=AF_INET;
        serv_addr.sin_port=htons(port);
        serv_addr.sin_addr.s_addr=inet_addr(ip.c_str());
        if(connect(clientfd,(struct sockaddr*)&serv_addr,sizeof(serv_addr))==-1){
            char errtxt[512]={0};
            sprintf(errtxt,"connect error! errno: %d",errno);
            controller->SetFailed(errtxt);
            close(clientfd);
            return;
        }
        // 发送rpc请求
        if(send(clientfd,send_rpc_str.c_str(),send_rpc_str.size(),0)==-1){
            char errtxt[512]={0};
            sprintf(errtxt,"send error! errno: %d",errno);
            controller->SetFailed(errtxt);
            close(clientfd);
            return;
        }
        // 接收rpc响应
        char recv_buf[1024]={0};
        ssize_t recv_size=0;
        if( (recv_size=recv(clientfd,recv_buf,sizeof(recv_buf),0)) ==-1){
            char errtxt[512]={0};
            sprintf(errtxt,"recv error! errno: %d",errno);
            controller->SetFailed(errtxt);
            close(clientfd);
            return ;
        }
        // 把rpc响应结果反序列化并写入response
        if(!response->ParseFromArray(recv_buf,recv_size)){
            char errtxt[2048]={0};
            sprintf(errtxt,"recv error! recv_buf: %s",recv_buf);
            controller->SetFailed(errtxt);
            close(clientfd);
            return ;
        }
        close(clientfd);
}       