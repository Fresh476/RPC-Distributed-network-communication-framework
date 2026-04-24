#pragma once

#include <string>
#include "LockQueue.h" 

using std::string;

// 日志级别
enum LogLevel{
    INFO, // 普通信息
    ERROR // 错误信息
};

/*
    写日志操作的调用链:
    用户调用LOG_INFO/LOG_ERR => 
    获取/构造Logger唯一实例,在构造时启动写日志线程writeTask =>
    调用Logger::Log()方法,把消息交给lockqueue成员 => 
    lockqueue调用push()方法,把消息加入到自己底层的互斥队列中,并设置信号量 =>
    写日志线程writeTask调用lockqueue成员的pop()方法,获取一条日志信息 =>
    打开日志文件并写入,完成

    LOG_INFO()/LOG_ERR() => Logger()/GetInstance() => writeTask() => lockqueue.pop()(阻塞) =>
    Logger::Log() => lockqueue.push()(解除阻塞) => writeTask被唤醒,fputs()写入
*/

// Mprpc框架提供的日志系统(单例模式)
class Logger{
public:
    void setLogLevel(LogLevel level); // 设置日志的级别
    void Log(string msg); // 写入日志
    static Logger& GetInstance(); // 获取日志的单例
private:
    Logger();
    Logger(const Logger&)=delete;
    Logger(Logger&&)=delete;
    Logger operator=(const Logger&)=delete;
private:
    int m_loglevel; // 记录日志级别
    LockQueue<string> m_lckQue; // 日志缓冲队列,Logger只负责把消息放入队列
};

// 定义宏函数作为写日志的用户接口
#define LOG_INFO(logmsgformat,...) \
    do { \
        Logger &logger=Logger::GetInstance(); \
        logger.setLogLevel(INFO); \
        char msg[1024]={0}; \
        snprintf(msg,1024,logmsgformat,##__VA_ARGS__); \
        logger.Log(msg); \
    }while(0);


#define LOG_ERR(logmsgformat,...) \
    do { \
        Logger &logger=Logger::GetInstance(); \
        logger.setLogLevel(ERROR); \
        char msg[1024]={0}; \
        snprintf(msg,1024,logmsgformat,##__VA_ARGS__); \
        logger.Log(msg); \
    }while(0);
