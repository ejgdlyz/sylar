#ifndef __SYLAR_CONFIG_H__
#define __SYLAR_CONFIG_H__

#include <memory>
#include <sstream>
#include <string>
#include <boost/lexical_cast.hpp>  // 类型转化
#include "sylar/log.h"

namespace sylar {

class ConfigVarBase {
public:
    typedef std::shared_ptr<ConfigVarBase> ptr;
    ConfigVarBase(const std::string& name, const std::string& description = "") 
        : m_name(name)
        , m_description(description) {
        
    }
    virtual ~ConfigVarBase() {}

    const std::string& getName() const { return m_name;}
    const std::string& getDescription() const { return m_description;}

    virtual std::string toString() = 0;
    virtual bool fromString(const std::string& val) = 0;  // 解析，初始化成员 
protected:
    std::string m_name;
    std::string m_description;

};

template <class T>
class ConfigVar : public ConfigVarBase {
public:
    typedef std::shared_ptr<ConfigVar> ptr; 
    ConfigVar(const std::string& name, const T& default_value, const std::string& description = "") 
        :ConfigVarBase(name, description)
        ,m_val(default_value) {

    }

    std::string toString() override {
        try {
            return boost::lexical_cast<std::string>(m_val);    // member-type is converted to string
        } catch (std::exception& e) {
            SYLAR_LOG_ERROR(SYLAR_LOG_ROOT()) << "ConfigVar::toString exception" 
                << e.what() << ", failed to convert: " << typeid(m_val).name() << " to string";
        }
        return "";
    }
    
    bool fromString(const std::string& val) override {
        try {
            m_val = boost::lexical_cast<T>(val);               // string is converted to member-type
            return true;
        } catch (std::exception& e) {
            SYLAR_LOG_ERROR(SYLAR_LOG_ROOT()) << "ConfigVar::fromString exception" 
                << e.what() << ", failed to convert: string to " << typeid(m_val).name();
        }
        return false;
    }

    const T getValue() const { return m_val;}
    void setValue(const T& v) { m_val = v;}
private:
    T m_val;
};

// 管理类
class Config {
public:
    typedef std::map<std::string, ConfigVarBase::ptr> ConfigValMap;

    
    template <class T>
    static typename ConfigVar<T>::ptr Lookup(const std::string& name,     // 定义时赋值,没有则创建
        const T& default_value, const std::string& description = "") {
        auto tmp = Lookup<T>(name);
        if (tmp) {
            SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "Lookup name = " << name << " exists.";
            return tmp;
        }
        if (name.find_first_not_of("abcdefghigklmnopqrstuvwxyz.012345678_ABCDEFGHIJKLMNOPQRSTUVWXYZ")
                != std::string::npos) {
            SYLAR_LOG_ERROR(SYLAR_LOG_ROOT()) << "Lookuped name invalid: " << name;

            throw std::invalid_argument(name);
        }

        typename ConfigVar<T>::ptr v(new ConfigVar<T>(name, default_value, description));
        s_data[name] = v;
        return v;
    }

    template <class T>
    static typename ConfigVar<T>::ptr Lookup(const std::string& name ) {            // 直接查找
        auto it = s_data.find(name);
        if (it == s_data.end()) {
            return nullptr;
        }
        return std::dynamic_pointer_cast<ConfigVar<T>>(it->second);                 // ConfigVarBase -> ConfigVar<T>
    } 
private:
    static ConfigValMap s_data;
};
} // namespace sylar

#endif // __SYLAR_CONFIG_H__