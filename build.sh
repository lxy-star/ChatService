#!/bin/bash
set -e

# 构建目录
BUILD_DIR="build/"

echo "正在构建 ChatService 工程..."

# 清理并创建目录
rm -rf $BUILD_DIR
mkdir -p $BUILD_DIR
cd $BUILD_DIR

cmake .. -DCMAKE_BUILD_TYPE=Debug
cmake --build .

echo "构建完成，输出位于 $BUILD_DIR"