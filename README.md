# sylar

## 开发环境
    Centos 7
    gcc 9.1 
    cmake 

## 项目目录
- bin ------------ 二进制
- build ---------- 中间文件路径
- cmake ---------- cmake 函数文件夹
- CMakeLists.txt - cmake 定义文件
- lib ------------ 库的输出路径
- Makefile 
- sylar ---------- 源代码路径
- tests ---------- 测试代码路径

## 日志系统
1. 仿照 Log4J
   - Logger    定义日志类别（系统级日志、业务级日志）
   - Formatter 日志格式
   - Appendder 日志输出地（控制台、文件）

## 配置系统 
- Config  --> Yaml
- 基础版本
  - ConfigVarBase: 基类
  - ConfigVar : public ConfigVarBase 实例化类
  - Config 管理类 

- 安装 yaml-cpp
   ```git
   git clone https://github.com/jbeder/yaml-cpp.git

   cd yaml-cpp && mkdir build && cd build && cmake .. && make && sudo make install
   ```
- 配置原则 ： 约定优于配置


## 协程库封装

## socket 函数库

## http 协议开发

## 分布式协议

## 推荐系统
