#define THREADED
#include "ZookeeperUtil.h"
#include "MprpcApplication.h"
#include <iostream>

using std::endl;
using std::cout;

/*
    ZkClient与ZkServer建立连接的核心API:
    zhandle_t* zookeeper_init(const char *host,watcher_fn watcher,int recv_timeout,
        const clientid_t *clientid,void *context,int flags);
    1.host:连接请求,格式为 ip:port,代表ZkServer服务器的地址和端口号
    2.watcher:回调函数,当会话状态发生变化或节点状态发生变化时由客户端库进行调用
    3.recv_timeout:会话超时时间(毫秒)
    4.clientid:会话恢复id,首次连接时置空,重连时传入之前保存的id
    5.context:用户自定义的上下文信息,作为watcher()的最后一个参数,可在watcher()中读取
    6.flags:保留参数,传0即可
    return:若连接成功,则返回有效的连接句柄,失败则返回nullptr
*/

/*
    watcher回调函数:
    typedef void (*watcher_fn)(zhandle_t *zh,int type,int state,
        const char *path,void *watcherCtx);
    1.zh:当前的ZkClient连接句柄,即zookeeper_init()成功后的连接句柄,由客户端库自动填写
    2.type:事件类型,分为会话事件和节点事件,通过宏进行定义,由客户端库从服务器响应包解析后自动填写
    3.state:会话状态,由客户端库通过解析服务器响应和获取客户端状态后自动填写
    4.path:节点路径,由客户端库从服务器响应包解析后自动填写
    5.watcherCtx:用户自定义的上下文,从zookeeper_init()中的context参数获取
*/

/*
    zookeeper_init()是异步调用的函数,调用时将创建一个网络IO线程(pthread_create+poll)和一个回调线程
    zookeeper_init()只负责开辟空间,然后立刻返回,不保证返回时一定成功建立了连接
    因此当前线程需要通过信号量来保证同步,确保连接已建立后再使用zhandle_t句柄
    在调用zookeeper_init()后创建信号量sem,将该信号量通过zoo_set_context()绑定给zhandle_t句柄然后wait
    在连接已建立后,会话状态将发生改变,导致watcher()回调函数被调用
    在watcher()中通过zoo_get_context()提取出该信号量sem,然后post该信号量
    此时将解除当前线程的阻塞,可以使用该zhandle_t句柄
*/

/*
    判断znode节点是否存在:
    int zoo_exists(zhandle_t *zh,const char *path,int watch, Stat *stat); (同步,阻塞等待响应)
    1.zh:当前的zhandle_t句柄,一个有效的Zk连接
    2.path:要查找的路径(绝对路径)
    3.watch:是否已注册watcher回调,传入0则只检测节点是否存在,传入非0则当节点状态变化(创建/删除/变更)时触发watcher
    注意如果注册watcher,该watcher是zookeeper_init()中传入的全局watcher,且是一次性注册,触发一次后即失效
    4.stat:输出参数,如果节点存在则返回该节点的状态信息
    return:返回宏表示节点的信息(节点存在/不存在/连接丢失/连接超时/连接关闭/会话过期/无权限访问)
*/

/*
    创建znode节点:
    int zoo_create(zhandle_t *zh,const char *path,const char *data,int datalen,
        const struct ACL_vector *acl,int flags,char *path_buffer,int buffer_len);
    1.zh:当前的zhandle_t句柄,一个有效的Zk连接
    2.path:节点路径(绝对路径)
    3.data:节点要存储的数据
    4.datalen:数据的长度
    5.acl:权限列表,ZOO_OPEN_ACL_UNSAFE(完全开放)/ZOO_READ_ACL_UNSAFE(只读)/ZOO_CREATE_ALL_ACL(创建者拥有所有权限)
    6.flags:节点类型标志,0(普通持久节点)/ZOO_EPHEMERAL(临时节点)/ZOO_SEQUENCE(顺序节点)
    7.path_buffer:返回的实际路径(用于顺序节点)
    8.buffer_len:路径缓冲区长度
    return:返回宏表示是否创建成功
*/

/*
    获取znode节点的值:
    int zoo_get(zhandle_t *zh,const char *path,int watch,
        char *buffer,int *buffer_len,Stat *stat);
    1.zh:当前的zhandle_t句柄,一个有效的Zk连接
    2.path:节点路径(绝对路径)
    3.watch:是否已注册watch回调,传入0则不注册,传入非0则一次性注册全局watcher回调
    4.buffer:输出参数,返回znode节点的内容
    5.buffer_len:输入/输出参数,缓冲区的长度/返回znode节点数据的长度
    6.stat:输出参数,znode节点的状态信息
    return:返回宏本次查询是否成功
*/

// 全局的watcher观察器(回调函数)
void global_watcher(zhandle_t *zh,int type,int state,
    const char* path,void *watcherCtx){
        if(type==ZOO_SESSION_EVENT){ // 回调的消息类型是和会话相关的消息类型
            if(state==ZOO_CONNECTED_STATE){ // 如果已建立连接,则获取信号量
                sem_t *sem=(sem_t*)zoo_get_context(zh); // 从指定的句柄zh上获取其携带的信号量
                sem_post(sem); // 给这个信号量+1,解除sem_wait()处的阻塞
            }
        }
}

// 构造函数:ZkClient句柄初始化为空
ZkClient::ZkClient():m_zhandle(nullptr){
}

// 析构函数:关闭ZkClient句柄
ZkClient::~ZkClient(){
    if(m_zhandle!=nullptr){
        zookeeper_close(m_zhandle);
    }
}

// 启动连接ZkServer
void ZkClient::Start(){
    string host=MprpcApplication::GetInstance().GetConfig().Load("zookeeperip");
    string port=MprpcApplication::GetInstance().GetConfig().Load("zookeeperport");
    string connstr=host+":"+port; // 组装向ZkServer发起连接的语句
    // 连接到ZkServer
    m_zhandle=zookeeper_init(connstr.c_str(),global_watcher,30000,nullptr,nullptr,0);
    if(m_zhandle==nullptr){
        cout<<"zookeeper_init() failed!"<<endl;
        exit(EXIT_FAILURE);
    }
    // 等待连接建立,通过信号量完成当前线程与zookeeper_init()的同步
    sem_t sem;
    sem_init(&sem,0,0);
    zoo_set_context(m_zhandle,&sem); // 设置zhandle的上下文信息,将信号量绑定给m_zhandle
    sem_wait(&sem); // 等待连接建立,在watcher()中解除此处的阻塞
    cout<<"zookeeper_init() success!"<<endl;
}

// 根据指定的path创建znode节点,保存到数据为data,长度为datalen,state=0默认为永久节点
void ZkClient::Create(const char*path,const char* data,int datalen,int state){
    char path_buffer[128]={0}; // 存储返回的节点实际路径
    int bufferlen=sizeof(path_buffer);
    // 检验path指定的节点在句柄m_zhandle上是否存在
    int flag=zoo_exists(m_zhandle,path,0,nullptr);
    // 节点不存在,创建之
    if(flag==ZNONODE){
        // 在指定的path创建znode节点
        flag=zoo_create(m_zhandle,path,data,datalen,
            &ZOO_OPEN_ACL_UNSAFE,state,path_buffer,bufferlen);
        if(flag==ZOK){
            cout<<"znode create success... path: "<<path<<endl;
        }
        else{
            cout<<"flag: "<<flag<<endl;
            cout<<"znode create failed... path: "<<path<<endl;
            exit(EXIT_FAILURE);
        }
    }
}

// 根据指定path获取znode节点的值
string ZkClient::GetData(const char *path){
    char buffer[64]={0};
    int buffer_len=sizeof(buffer);
    int flag=zoo_get(m_zhandle,path,0,buffer,&buffer_len,nullptr);
    if(flag!=ZOK){
        cout<<"get znode failed... path: "<<path<<endl;
        return "";
    }
    else{
        return buffer;
    }
}