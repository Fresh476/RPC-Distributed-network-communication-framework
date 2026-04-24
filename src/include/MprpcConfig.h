#pragma once

#include <unordered_map>
#include <string>

using std::unordered_map;
using std::string;

// 框架读取配置文件类
class MprpcConfig{
public:
    void LoadConfigFile(const char* config_file); // 加载并解析配置文件
    string Load(const string& key); // 查询配置项信息
private:
    void Trim(string &src_buf); // 去掉字符串前后的空格
private:
    unordered_map<string,string> m_configMap; // 存储所有的配置项的值
};
