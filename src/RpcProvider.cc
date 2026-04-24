#include "RpcProvider.h"
#include "RpcHeader.pb.h"
#include "Logger.h"
#include "ZookeeperUtil.h"

using namespace mprpc;

// RPC框架使用的所有用户自定义服务都继承自google::protobuf::Service类,使用Service基类指针进行多态调用
// Service类是所有protobuf类的抽象基类,RPC服务器通过Service对象注册服务
// 发布RPC方法的接口,传入一个待发布的RPC服务类,该类继承于Service类
void RpcProvider::NotifyService(google::protobuf::Service *service){
    ServiceInfo service_info;
    // google::protobuf::ServiceDescriptor用于描述一个service的类型信息
    // ServiceDescriptor对象具有反射功能,可以在运行时查询一个服务有哪些方法/哪些参数
    // 通过调用Service对象的GetDescriptor()方法可以获取到该对象的const ServiceDescriptor*
    const google::protobuf::ServiceDescriptor *pserviceDesc=service->GetDescriptor();
    // 获取服务的名字
    string service_name=pserviceDesc->name();
    // 获取服务对象service的方法数量
    int methodCnt=pserviceDesc->method_count();

    LOG_INFO("service_name: %s",service_name.c_str());

    // MethodDescriptor类用于描述一个RPC方法的信息,包括函数名、参数类型、返回值类型等
    // 调用ServiceDescriptor::method(int i)方法可以获取当前服务对象的第i个RPC方法
    for(int i=0;i<methodCnt;i++){
        // 获取第i个RPC方法
        const google::protobuf::MethodDescriptor *pmethodDesc=pserviceDesc->method(i);
        // 获取当前方法的名字
        string method_name=pmethodDesc->name();
        LOG_INFO("method_name: %s",method_name.c_str());
        // 把<方法名,方法>保存到方法映射表中
        service_info.m_methodMap.insert({method_name,pmethodDesc});
    }
    service_info.m_service=service; // 记录传入的服务对象
    m_serviceMap.insert({service_name,service_info}); // 把<服务对象名,服务对象>保存到服务映射表中
}

// 启动RPC服务节点,开始提供RPC远程网络调用服务
void RpcProvider::Run(){
    // 获取MprpcApplication实例的m_config成员,查询其rpcserverip/port配置项的值作为ip地址和port
    string ip = MprpcApplication::GetInstance().GetConfig().Load("rpcserverip");
    uint16_t port=atoi(MprpcApplication::GetInstance().GetConfig().Load("rpcserverport").c_str());
    InetAddress address(ip,port);

    // 创建TcpServer对象并初始化
    TcpServer server(&m_eventloop,address,"RpcProvider");
    // 绑定连接回调和消息读写回调方法
    server.setConnectionCallback(bind(&RpcProvider::onConnection,this,_1));
    server.setMessageCallback(bind(&RpcProvider::onMessage,this,_1,_2,_3));
    // 设置Muduo库的线程数量
    server.setThreadNum(4);

    // 把RPC上要发布的服务全部注册到zookeeper上,使ZkClient可以从ZkServer上发现服务
    ZkClient zkCli;
    zkCli.Start(); // 启动一个Zk客户端完成注册工作
    // 遍历服务映射表
    for(auto &sp:m_serviceMap){
        // 如果当前服务名是FriendServiceRpc,则路径为 /FriendServiceRpc
        string service_path="/"+sp.first; // 组装服务所在的路径名 
        zkCli.Create(service_path.c_str(),nullptr,0); // 创建znode父节点(永久节点)
        // 遍历该服务下的所有方法
        for(auto &mp:sp.second.m_methodMap){
            // 如果当前方法是FriendServiceRpc::GetFriendsList,则路径为 /FriendServiceRpc/GetFriendsList
            string method_path=service_path+"/"+mp.first; // 组装方法所在的路径名
            char method_path_data[128]={0};
            sprintf(method_path_data,"%s:%d",ip.c_str(),port); // 保存的数据为 ip:port
            // 在父节点下创建znode子节点,保存该方法的ip和port,作为临时节点
            // 假设使用永久节点,如果该主机宕机,用户的请求仍然会被分发到该主机上,导致大量的调用失败
            // 使用临时节点,自动剔除宕机的服务器,用户可以实时感知服务器的状态
            // 且临时节点的生命周期与创建它的ZkClient客户端相绑定,当ZkCli析构时或超时时自动删除
            zkCli.Create(method_path.c_str(),method_path_data,strlen(method_path_data),ZOO_EPHEMERAL);
        }
    }

    // 启动网络服务,开启主事件循环
    cout<<"RpcProvider start server at ip: "<<ip<<", port: "<<port<<endl;
    server.start(); 
    m_eventloop.loop();
}

// socket连接回调(Muduo库回调)
void RpcProvider::onConnection(const TcpConnectionPtr& conn){
    // 如果连接断开
    if(!conn->connected()){
        conn->shutdown(); // 关闭连接
    }
}

// 读写消息回调(Muduo库回调):如果远程有RPC调用请求,则onMessage()就会响应
void RpcProvider::onMessage(const TcpConnectionPtr& conn,Buffer *buffer,Timestamp){
    string recv_buf=buffer->retrieveAllAsString(); // 获取所有的数据(字节流)
    // 开始解析RPC请求
    // 先读取前4个字节的内容,表示RPC调用请求头部的长度
    uint32_t header_size=0;
    recv_buf.copy((char*)&header_size,4,0);
    // 根据header_size读取RPC调用请求头部(字节流),即从第4个字节开始,读取header_size的长度
    string rpc_header_str=recv_buf.substr(4,header_size);
    // 反序列化
    RpcHeader rpcHeader;
    string service_name;
    string method_name;
    uint32_t args_size;
    if(rpcHeader.ParseFromString(rpc_header_str)){
        // 反序列化成功,读取服务名称、方法名称、参数长度
        service_name=rpcHeader.service_name();
        method_name=rpcHeader.method_name();
        args_size=rpcHeader.args_size();
    } 
    else{
        // 反序列化失败
        cout<<"rpc_header_str: "<<rpc_header_str<<"parse error!"<<endl;
        return ;
    }
    // 读取RPC方法的参数信息(字节流)
    string args_str=recv_buf.substr(4+header_size,args_size);

    cout<<"=========debug========="<<endl;
    cout<<"header_size:"<<header_size<<endl;
    cout<<"rpc_header_str:"<<rpc_header_str<<endl;
    cout<<"service_name:"<<service_name<<endl;
    cout<<"method_name:"<<method_name<<endl;
    cout<<"args_size:"<<args_size<<endl;
    cout<<"args_str:"<<args_str<<endl;
    cout<<"========================"<<endl;

    // 获取service对象和method对象
    auto it=m_serviceMap.find(service_name);
    if(it==m_serviceMap.end()){
        cout<<service_name<<"is not exist!"<<endl;
        return ;
    }
    auto mit=it->second.m_methodMap.find(method_name);
    if(mit==it->second.m_methodMap.end()){
        cout<<method_name<<"is not exist!"<<endl;
        return ;
    }
    google::protobuf::Service *service=it->second.m_service; // 成功获取service对象
    const google::protobuf::MethodDescriptor *method=mit->second; // 成功获取method对象
    // 生成请求request和响应response,所有的消息类型都继承于Message类
    // 调用Service::GetRequestPrototype(method).New()获取method方法对应的请求消息类型,绑定给Message基类指针
    google::protobuf::Message *request=service->GetRequestPrototype(method).New();
    // 序列化后的request(即参数信息)已经保存在args_str中,调用ParseFromString()方法解析即可
    if(!request->ParseFromString(args_str)){
        // 参数解析失败
        cout<<"request parse error! content: "<<args_str<<endl;
        return ;
    }
    // 调用Service::GetResponsePrototype(method).New()获取method方法对应的响应消息类型,绑定给Message基类指针
    google::protobuf::Message *response=service->GetResponsePrototype(method).New();
    // 给method方法的调用,绑定一个Closure回调函数
    // 调用protobuf::NewCallback()方法,传入回调函数及其需要的参数,生成Closure对象用于回调
    google::protobuf::Closure *done=
        google::protobuf::NewCallback<RpcProvider,const TcpConnectionPtr&,google::protobuf::Message*>
            (this,&RpcProvider::SendRpcResponse,conn,response);
    // 在框架上调用method方法,request是输入参数,response是输出参数,done是完成请求后的回调对象,RpcController置空
    service->CallMethod(method,nullptr,request,response,done);
}

// Closure的回调操作,用于把RPC的响应序列化并通过网络发送
void RpcProvider::SendRpcResponse(const TcpConnectionPtr& conn,google::protobuf::Message *response){
    string response_str;
    if(response->SerializeToString(&response_str)){
        // response序列化成功,存储到response_str中,通过conn发送给调用方
        conn->send(response_str);
    }
    else{
        cout<<"serialize response_str error!"<<endl;
        return;
    }
    conn->shutdown(); // RpcProvider主动断开连接
}