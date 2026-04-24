#pragma once

#include "google/protobuf/service.h"
#include <string>

using std::string;

// 用于管理RPC请求的类,继承自RpcController基类,并重写其虚函数
class MprpcController:public google::protobuf::RpcController{
public:
    MprpcController();

    void Reset();
    bool Failed()const;
    string ErrorText()const;
    void SetFailed(const string &reason);

    // 以下方法尚未实现
    void StartCancel();
    bool IsCanceled()const;
    void NotifyOnCancel(google::protobuf::Closure *callback);
private:
    bool m_failed; // RPC方法执行过程中的状态(是否出错)
    string m_errText; // RPC方法执行过程中的错误信息
};