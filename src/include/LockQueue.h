#pragma once
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>

using std::queue;
using std::thread;
using std::mutex;
using std::unique_lock;
using std::lock_guard;
using std::condition_variable;

// 异步写日志的日志队列,执行把日志消息写入文件的操作
template<typename T>
class LockQueue{
public:
    void push(const T& data); // 向队列里写日志(多线程写)
    T pop(); // 从队列中读出一条日志(单线程读)
private:
    queue<T> m_queue; // 保存所有日志信息的队列
    mutex m_mutex; // 保证队列的互斥访问
    condition_variable m_condvariable; // 保证出队与入队同步的信号量
};

// 把日志加入到队列中
template<typename T>
void LockQueue<T>::push(const T& data){
    // 多个线程互斥访问队列,加锁
    lock_guard<mutex>lock(m_mutex);
    m_queue.push(data); // 日志入队
    m_condvariable.notify_one(); // 唤醒一个线程,解除pop()内的阻塞
}

template<typename T>
T LockQueue<T>::pop(){
    unique_lock<mutex>lock(m_mutex);
    // 日志队列为空,线程进入wait状态
    while(m_queue.empty()){
        m_condvariable.wait(lock); // 此处的阻塞必须被push()解除,然后退出循环
    }
    T data=m_queue.front(); // 获取队首元素
    m_queue.pop(); // 出队
    return data;
}