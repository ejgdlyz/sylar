#!/bin/sh

# 将一个项目(库或可执行文件)复制到特定的目录
if [ ! -d bin/module ]                       # -d 检查 bin/module 是否存在, ! 即不存在 
then 
    mkdir bin/module
else 
    unlink bin/project_name                 # 删除当前目录下 bin/project_name 的符号链接
    unlink bin/module/libproject_name.so    # 删除 bin/module/libproject_name.so 符号链接
fi

cp sylar/bin/sylar bin/project_name         # 将 sylar/bin/sylar 复制为 bin/project_name
cp lib/libproject_name.so bin/module/       # 将 lib/libproject_name.so 复制到 bin/module/