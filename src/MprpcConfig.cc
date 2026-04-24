#include "MprpcConfig.h"
#include <iostream>

using std::cout;
using std::endl;

// 去掉字符串前后的空格
void MprpcConfig::Trim(string &src_buf){
    size_t idx=0;
    idx=src_buf.find_first_not_of(' '); // 从该行字符串中查找第一个不是空格的下标,失败返回-1
    if(idx!=-1){
        src_buf=src_buf.substr(idx,src_buf.size()-idx); // 截取有效部分
    }
    // 去掉每一行后面多余的空格
    idx=src_buf.find_last_not_of(' '); // 从后往前找到一个不是空格的下标,失败返回-1
    if(idx!=-1){
        src_buf=src_buf.substr(0,idx+1);
    }
}

void MprpcConfig::LoadConfigFile(const char* config_file){
    FILE* pf=fopen(config_file,"r"); // 以只读方式打开该文件
    if(pf==nullptr){
        cout<<"config_file is invalid!"<<endl;
        exit(EXIT_FAILURE);
    }
    // 循环读取该文件
    // 1.注释(以#开头)或空行 2.每行开头多余的空格 3.正确的配置选项
    while(!feof(pf)){
        char buf[512]={0};
        string key,value;
        fgets(buf,512,pf); // 读取一行保存到buf中
        string read_buf(buf);

        Trim(read_buf); // 去掉空格
        size_t idx=0;
        // 判断#注释和空行,直接跳过,读取下一行
        if(read_buf[0]=='#'||read_buf.empty()){
            continue;
        }

        // 如果是正确的配置,解析配置项
        idx=read_buf.find('='); // 查找"="的位置
        // 找不到"=",配置项不合法,读取下一行
        if(idx==-1){
            continue;
        }
        key=read_buf.substr(0,idx); // 读取配置项的名称
        Trim(key); // 去掉key前后的空格
        // 读取配置项的值
        size_t endidx=read_buf.find('\n',idx); // 查找行尾的回车并删除
        // 如果行末有回车,则截取到回车符之前的部分作为value
        if(endidx!=-1){
            value=read_buf.substr(idx+1,endidx-idx-1);
        }
        // 如果行末没有回车,则截取到行末作为value
        else{
            value=read_buf.substr(idx+1,read_buf.size()-idx);
        }
        Trim(value); // 去掉value前后的空格

        m_configMap.insert({key,value}); // 保存到map中
    }
    fclose(pf);
}

// 查询配置项key的值
string MprpcConfig::Load(const string& key){
    auto it=m_configMap.find(key);
    // 该配置项不存在,则返回空字符串
    if(it==m_configMap.end()){
        return "";
    }
    return it->second;
}