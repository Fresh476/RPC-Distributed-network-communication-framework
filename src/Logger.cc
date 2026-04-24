#include <time.h>
#include <iostream>
#include "Logger.h"

using std::cout;
using std::endl;

// 获取唯一实例
Logger& Logger::GetInstance(){
    static Logger logger;
    return logger;
}

// 构造函数
Logger::Logger(){
    // 启动专门的写日志线程,只有该线程可以读取待写入的日志并完成将其写入文件的操作
    thread writeTask([&](){
        for(;;){
            // 获取当前日期,把日志信息写入相应的日志文件当中
            time_t now=time(nullptr);
            struct tm *nowtm=localtime(&now);
            char file_name[128]={0};
            // 日志文件的命名格式:年-月-日-log.txt
            sprintf(file_name,"%d-%d-%d-log.txt",
                nowtm->tm_year+1900,nowtm->tm_mon+1,nowtm->tm_mday);
            FILE *pf=fopen(file_name,"a+"); // 不存在则创建,存在则追加
            if(pf==nullptr){
                cout<<"logger file: "<<file_name<<" open failed!"<<endl;
                exit(EXIT_FAILURE);
            }
            // 从日志缓冲队列中取出消息并写入文件
            string msg=m_lckQue.pop();
            char time_buf[128]={0};
            sprintf(time_buf,"%02d:%02d:%02d => [%s]",
                nowtm->tm_hour,nowtm->tm_min,nowtm->tm_sec,
                m_loglevel==LogLevel::INFO?"INFO":"ERROR");
            msg=time_buf+msg;
            msg.append("\n");
            fputs(msg.c_str(),pf);
            fclose(pf);
        }
    });
    writeTask.detach();
}

// 设置日志级别
void Logger::setLogLevel(LogLevel level){
    m_loglevel=level;
}

// 写日志,把日志信息写入lockqueue缓冲区中
void Logger::Log(string msg){
    m_lckQue.push(msg); // 把消息放入lockqueue成员中,由该写日志线程完成写入操作
}

