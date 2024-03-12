#!/bin/sh
# 创建一个新的项目目录和命名空间

# 执行传入的命令，执行失败则立即退出
command_error_exit() {
    $*                  # 特殊变量, 所有位置参数列表，将其作为一个命令串来执行
    if [ $? -ne 0 ]       # $?: 最后执行命令的退出状态, -ne: 即 !=
    then
        exit 1
    fi
}

if [ $# -lt 2 ]    # $#: 参数数量, -lt: 即 <.
then 
    echo "use [ $0 project_name namespace ]"
    exit 0
fi

project_name=$1     # 脚本第一个参数赋值给 project_name
namespace=$2        # 脚本第二个参数赋值给 namespace

command_error_exit mkdir $project_name                                      # 创建一个用户自定义的新文件夹 $project_name
command_error_exit cd $project_name                                         # 进入 $project_name
command_error_exit git clone git@github.com:ejgdlyz/sylar.git
command_error_exit cp sylar/Makefile .
command_error_exit cp -rf sylar/template/* .
command_error_exit cp -rf sylar/template/* .
command_error_exit mv template ${namespace}
command_error_exit sed -i "s/project_name/${project_name}/g" CMakeLists.txt # 将 CMakeLists.txt 文件中的所有出现 project_name 替换为 $project_name
command_error_exit sed -i "s/template/${namespace}/g" CMakeLists.txt       
command_error_exit sed -i "s/project_name/${project_name}/g" move.sh
command_error_exit cd ${namespace}                                          # 进入 ${namespace}
command_error_exit sed -i "s/name_space/${namespace}/g" `ls .`              # 将 ${namespace} 下所有文件中出现的 name_space 替换为 ${namespace}
command_error_exit sed -i "s/project_name/${project_name}/g" `ls .`         


echo "create ${project_name} -- ${namespace} ok"
