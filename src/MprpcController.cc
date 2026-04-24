#include "MprpcController.h"

// 构造函数
MprpcController::MprpcController(){
    m_failed=false; // 未出错
    m_errText=""; // 无错误信息
}

// 重置RpcController的状态
void MprpcController::Reset(){
    m_failed=false; // 未出错
    m_errText=""; // 无出错信息
}

// 判断当前RPC调用的执行是否出错
bool MprpcController::Failed()const{
    return m_failed;
}

// 返回出错信息
string MprpcController::ErrorText()const{
    return m_errText;
}

// 设置出错信息
void MprpcController::SetFailed(const string &reason){
    m_failed=true;
    m_errText=reason;
}

// 以下方法尚未实现
void MprpcController::StartCancel(){}
bool MprpcController::IsCanceled()const{return false;}
void MprpcController::NotifyOnCancel(google::protobuf::Closure *callback){}