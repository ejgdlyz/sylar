#include "sylar/config.h"
#include "sylar/log.h"
#include "sylar/env.h"
#include <yaml-cpp/yaml.h>
#include <iostream>

#if 0
sylar::ConfigVar<int>::ptr g_int_value_config  = 
    sylar::Config::Lookup("system.port", (int)8080, "system port");

// key 相同但是类型不同   -- 应该报错
sylar::ConfigVar<float>::ptr g_int_valuex_config  = 
    sylar::Config::Lookup("system.port", (float)8080, "system port");

sylar::ConfigVar<float>::ptr g_float_value_config  = 
    sylar::Config::Lookup("system.value", (float)3.2f, "system value");

// sylar::ConfigVar<std::string>::ptr g_invalid_value_config  = 
//     sylar::Config::Lookup("#invalid.value", (std::string)("invalid"), "invalid value");

// 修改为 LexicalCast 后的测试 vector
sylar::ConfigVar<std::vector<int>>::ptr g_int_vec_value_config = 
    sylar::Config::Lookup("system.int_vec", std::vector<int>{1,2}, "system int list");

sylar::ConfigVar<std::list<int>>::ptr g_int_list_value_config = 
    sylar::Config::Lookup("system.int_list", std::list<int>{1,2}, "system int list");

sylar::ConfigVar<std::set<int>>::ptr g_int_set_value_config = 
    sylar::Config::Lookup("system.int_set", std::set<int>{1,2}, "system int set");

sylar::ConfigVar<std::unordered_set<int>>::ptr g_int_uset_value_config = 
    sylar::Config::Lookup("system.int_uset", std::unordered_set<int>{1,2}, "system int uset");

sylar::ConfigVar<std::map<std::string, int>>::ptr g_str_int_map_value_config = 
    sylar::Config::Lookup("system.str_int_map", std::map<std::string, int>{{"k",2}}, "system str int map");

sylar::ConfigVar<std::unordered_map<std::string, int>>::ptr g_str_int_umap_value_config = 
    sylar::Config::Lookup("system.str_int_umap", std::unordered_map<std::string, int>{{"k",2}}, "system str int umap");


void print_yaml(const YAML::Node& node, int level) {
    if (node.IsScalar()) {                                                  // 简单类型 
        SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << std::string(level * 4, ' ') 
            << node.Scalar() << " - " << node.Type() << " - " << level;
    } else if (node.IsNull()) {
        SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << std::string(level * 4, ' ') 
            << "NULL" << " - " << node.Type() << " - " << level;
    } else if (node.IsMap()) {                                             // map, first(string), second(YAML::Node)
        for (auto it = node.begin(); it != node.end(); ++it) {
            SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << std::string(level * 4, ' ') 
                << it->first << " - " << it->second.Type() << " - " << level;
            print_yaml(it->second, level + 1);
        }
    } else if (node.IsSequence()) {                                        // 数组, 
        for (size_t i = 0; i < node.size(); ++i) {
            SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << std::string(level * 4, ' ')
                << i << " - " << node[i].Type() << " - " << level;
            print_yaml(node[i], level + 1);
        }
    }

}

void test_yaml() {
    YAML::Node root = YAML::LoadFile("/home/lambda/workspace/sylar/bin/conf/test.yml");

    // SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << root;
    print_yaml(root, 0);
}

void test_config() {
    SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "before: " <<  g_int_value_config->getValue();
    SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "before: " << g_float_value_config->toString();

#define XX(g_var, name, prefix) \
    { \
        auto& v = g_var->getValue(); \
        for (auto& i : v) { \
            SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << #prefix " " #name ": " << i;  \
        } \
        SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << #prefix " " #name " yaml: " << g_var->toString();  \
    } 

#define XX_M(g_var, name, prefix) \
    { \
        auto& v = g_var->getValue(); \
        for (auto& i : v) { \
            SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << #prefix " " #name ": {"  \
                    << i.first << " - " << i.second << "}";  \
        } \
        SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << #prefix " " #name " yaml: " << g_var->toString();  \
    } 

    XX(g_int_vec_value_config, int_vec, before);
    XX(g_int_list_value_config, int_list, before);
    XX(g_int_set_value_config, int_set, before);
    XX(g_int_uset_value_config, int_uset, before);
    XX_M(g_str_int_map_value_config, str_int_map, before); 
    XX_M(g_str_int_umap_value_config, str_int_umap, before);


    YAML::Node root = YAML::LoadFile("/home/lambda/workspace/sylar/bin/conf/test.yml");  
    sylar::Config::loadFromYaml(root);

    // 通过文件名加载后的值
    SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "after: " <<  g_int_value_config->getValue();
    SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "after: " << g_float_value_config->toString();

    XX(g_int_vec_value_config, int_vec, after);
    XX(g_int_list_value_config, int_list, after);
    XX(g_int_set_value_config, int_set, after);
    XX(g_int_uset_value_config, int_uset, after);
    XX_M(g_str_int_map_value_config, str_int_map, after);
    XX_M(g_str_int_umap_value_config, str_int_umap, before);
}
#endif

struct Person {
    std::string m_name;
    int m_age = 0;
    bool m_sex = 0;

    std::string toString() const {
        std::stringstream ss;
        ss << "[Person name = " << m_name
            << " age = " << m_age 
            << " sex = " << m_sex
            << "]";
        return ss.str();
    }

    bool operator==(const Person& rhs) const {
        return m_name == rhs.m_name && m_age == rhs.m_age && m_sex == rhs.m_sex;
    }
};

namespace sylar {
template<>
class LexicalCast<std::string, Person> {
public:
    Person operator() (const std::string& v) {
        YAML::Node node = YAML::Load(v);  // string -> YAML node
        Person p; 
        p.m_name = node["name"].as<std::string>();
        p.m_age = node["age"].as<int>();
        p.m_sex = node["sex"].as<bool>();
        return p;
    }

};

template<>
class LexicalCast<Person, std::string> {
public:
    std::string operator() (const Person& p) {
        YAML::Node node;  
        node["name"] = p.m_name;
        node["age"] = p.m_age;
        node["sex"] = p.m_sex;
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};

}  // namespace sylar

sylar::ConfigVar<Person>::ptr g_person = 
    sylar::Config::Lookup("class.person", Person(), "system person");

sylar::ConfigVar<std::map<std::string, Person>>::ptr g_person_map = 
    sylar::Config::Lookup("class.map", std::map<std::string, Person>(), "system map");

sylar::ConfigVar<std::map<std::string, std::vector<Person>>>::ptr g_person_vec_map = 
    sylar::Config::Lookup("class.vec_map", std::map<std::string, std::vector<Person>>(), "system vec map");

void test_class () {
    SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "before: " << g_person->getValue().toString() << " - " << g_person->toString();

#define XX_PM(g_var, prefix) \
    {  \
        auto m = g_person_map->getValue(); \
        for (auto& v : m) {  \
            SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << prefix << ": " << v.first << " - " << v.second.toString();  \
        }  \
        SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << prefix << ": size = " << m.size();  \
    }

    XX_PM(g_person_map, "class.map before");
    SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "before: " << g_person_vec_map->toString();
    
    g_person->addListener([](const Person& old_value, const Person& new_value) {  // 回调函数测试 
        SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "old_value=" << old_value.toString()
                << ", new_value=" << new_value.toString();
    });

    YAML::Node root = YAML::LoadFile("/home/lambda/workspace/sylar/bin/conf/test.yml");  
    sylar::Config::loadFromYaml(root);

    SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "after: " << g_person->getValue().toString() <<" - " << g_person->toString();
    XX_PM(g_person_map, "class.map after");
    SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "after: " << g_person_vec_map->toString();

}

void test_logger_config() {

    static sylar::Logger::ptr system_log = SYLAR_LOG_NAME("system");   // 申请一个 system logger, 此时没有加载 log.yml, 
    SYLAR_LOG_INFO(system_log) << "hello system log" << std::endl;     // 所以，新建的 system logger 使用默认的 formatter 和 StdoutLogAppender， 解析和输出日志内容

    std::cout << sylar::LoggerMgr::GetInstance()->toYamlString() << std::endl;   // 输出目前所有 logger 配置
    
    YAML::Node root = YAML::LoadFile("/home/lambda/workspace/sylar/bin/conf/log.yml");
    sylar::Config::loadFromYaml(root);          // 触发配置变化 -> 事件 -> logger 初始化  

    std::cout << "====================================================" << std::endl;
    std::cout << sylar::LoggerMgr::GetInstance()->toYamlString() << std::endl;

    // std::cout << "====================================================" << std::endl;
    // std::cout << root << std::endl;
    
    SYLAR_LOG_INFO(system_log) << "hello system log" << std::endl;

    system_log->setFormatter("%d - %m%n");
    SYLAR_LOG_INFO(system_log) << "hello system log" << std::endl;

}
void test_config_Visit() {
    // Config::Visit 测试
    std::cout << "********* Config::Visit *********" << std::endl;
    sylar::Config::Visit([](sylar::ConfigVarBase::ptr var){
        SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "name = " << var->getName() 
                << ", description = " << var->getDescription()
                << ", typename = " << var->getTypeName()        // 类型
                << ", value = " << var->toString();             // 每个类型对应的 YAML结果
    });  
}

void test_loadconf() {
    sylar::Config::loadFromConfDir("conf");
}

int main(int argc, char *argv[])
{
    // SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << g_int_value_config->getValue();
    // SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << g_float_value_config->toString();

    // 不合法 
    // SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << g_invalid_value_config->getValue();


    // test_yaml();

    // test_config();

    // test_class();

    test_logger_config();

    // test_config_Visit();

    // sylar::EnvMgr::GetInstance()->init(argc, argv);
    // test_loadconf();
    // std::cout << " ===== " << std::endl;
    // sleep(10);
    // test_loadconf();

    return 0;
}
