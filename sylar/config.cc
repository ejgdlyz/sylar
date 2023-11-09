#include "config.h"

namespace sylar {

// Config::ConfigVarMap Config::s_data;    

ConfigVarBase::ptr Config::LookupBase(const std::string& name) {  // 查找当前命名的项
    auto it = GetData().find(name);
    return it == GetData().end() ? nullptr : it->second;
}

/*
"A.B", 10,    即 key = "A.B", value = 100, 对应的 yaml
A:
    B: 10
    C: str
*/
static void ListAllMember(const std::string& prefix,
        const YAML::Node& node, std::list<std::pair<std::string, const YAML::Node>>& output) {
    
    if (prefix.find_first_not_of("abcdefghigklmnopqrstuvwxyz.012345678_") 
            != std::string::npos) {
        SYLAR_LOG_ERROR(SYLAR_LOG_ROOT()) << "config invalid name: " << prefix << " : " << node;
        return;
    }
    output.push_back(std::make_pair(prefix, node));  
    if (node.IsMap()) {  // map，继续遍历
        for (auto it = node.begin(); it != node.end(); ++it) {
            ListAllMember(prefix.empty() ? it->first.Scalar()
                    : prefix + "." + it->first.Scalar(), it->second, output);
        }
    }

}

void Config::loadFromYaml(const YAML::Node& root) {
    std::list<std::pair<std::string, const YAML::Node>> all_nodes;
    ListAllMember("", root, all_nodes);  // 层级结构打平

    for (auto& node: all_nodes) {
        std::string key = node.first;
        if (key.empty()) {
            continue;

        }

        std::transform(key.begin(), key.end(), key.begin(), ::tolower);
        ConfigVarBase::ptr var = LookupBase(key);

        if (var) {
            if (node.second.IsScalar()) {    // string
                var->fromString(node.second.Scalar());
            } else {
                std::stringstream ss;
                ss << node.second;
                var->fromString(ss.str());
            }
        }

    }
}


}
