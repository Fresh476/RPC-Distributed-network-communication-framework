#include "test.pb.h"
#include <iostream>
#include <string>

using namespace MyRPC;
using std::cout;
using std::endl;
using std::string;

/*
    protobuf使用方法: xxx代表要处理的字段名
    1.给message对象的字段(基础类型)设置值:调用对应的set_xxx()方法
    2.从message对象中解析字段(基础类型)值:调用对应的xxx()方法
    3.给message对象的字段(单个message类型)设置值:调用其mutable_xxx()方法获取指针,该指针为对应的嵌套message指针
        然后调用该指针的set_xxx()方法给其每个字段设置值
    4.从message对象中解析字段(单个message类型)值:调用xxx()方法获取对应的message常对象,根据此对象获取其每个字段的值
    5.给message对象的字段(列表message类型)设置值:调用对应的add_xxx方法获取一个message指针
        然后调用该指针的set_xxx()方法给每个字段设置值
    6.从message对象中解析字段(列表message类型)值:先调用对应的xxx_size()获取列表长度,然后使用for循环遍历
        调用该对象的xxx(i)方法获取列表中的第i个message对象(常引用),然后访问其每个字段的值
    7.message序列化方法:Serialize系列成员方法,常用SerializeToString(string *)
    8.message反序列化方法:ParseFrom系列方法,常用ParseFromString(const string&)
*/
int main(){
    /*
    LoginResponse rsp;
    rsp.set_success(1);
    ResultCode *result=rsp.mutable_result();
    result->set_errcode(-1);
    result->set_errmsg("这是错误消息");
    string send_str;
    cout<<result->Utf8DebugString()<<endl;
    cout<<"==================================="<<endl;
    if(rsp.SerializeToString(&send_str)){
        cout<<send_str<<endl;
    }
    cout<<"==================================="<<endl;
    LoginResponse rspB;
    if(rspB.ParseFromString(send_str)){
        ResultCode res;
        cout<<rspB.result().errcode()<<endl;
        cout<<rspB.result().errmsg()<<endl;
        cout<<rspB.success()<<endl;
    }
    */
    GetFriendListResponse rsp;
    ResultCode *rc=rsp.mutable_result();
    rc->set_errcode(0);
    rc->set_errmsg("no error");
    User *user1 = rsp.add_friend_list();
    user1->set_name("zhang san");
    user1->set_age(20);
    user1->set_sex(User::MAN);
    User *user2=rsp.add_friend_list();
    user2->set_name("li si");
    user2->set_age(22);
    user2->set_sex(User::WOMAN);

    cout<<rsp.friend_list_size()<<endl;
    for(int i=0;i<rsp.friend_list_size();i++){
        User user_i=rsp.friend_list(i);
        cout<<user_i.name()<<" "<<user_i.age()<<" "<<user_i.sex()<<endl;
    }
    return 0;
}