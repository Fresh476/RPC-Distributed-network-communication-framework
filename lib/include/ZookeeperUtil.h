#pragma once

#include <semaphore.h>
#include <zookeeper/zookeeper.h>
#include <zookeeper/proto.h>
#include <zookeeper/recordio.h>
#include <string>

using std::string;

// zookeeper客户端类
class ZkClient{
public:
    ZkClient(); // 构造函数
    ~ZkClient(); // 析构函数

    void Start(); // 启动连接ZkServer
    // 在ZkServer上根据指定的path创建znode节点,包含数据data,长度为datalen,state=0默认为永久节点
    void Create(const char* path,const char *data,int datalen,int state=0);
    string GetData(const char *path); // 根据path获取指定的znode节点数据
private:
    zhandle_t *m_zhandle; // zk客户端句柄,代表一个与Zk服务器的连接实例
};