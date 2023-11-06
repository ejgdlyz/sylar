#include "sylar/config.h"
#include "sylar/log.h"

sylar::ConfigVar<int>::ptr g_int_value_config  = 
    sylar::Config::Lookup("system.port", (int)8080, "system port");

sylar::ConfigVar<float>::ptr g_float_value_config  = 
    sylar::Config::Lookup("system.value", (float)3.2f, "system value");

sylar::ConfigVar<std::string>::ptr g_invalid_value_config  = 
    sylar::Config::Lookup("#invalid.value", (std::string)("invalid"), "invalid value");

int main(int argc, char const *argv[])
{
    SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << g_int_value_config->getValue();
    SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << g_float_value_config->toString();

    // 不合法 
    SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << g_invalid_value_config->getValue();

    return 0;
}
