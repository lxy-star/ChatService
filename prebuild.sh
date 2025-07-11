#!/bin/bash
set -e

#安装MYSQL,redis,asio,nlohmann/json（如果未安装）
echo "检查并安装 MYSQL,redis,asio,nlohmann/json ..."
sudo apt update
sudo apt install -y mysql-server libmysqlcppconn-dev redis-server libhiredis-dev libasio-dev nlohmann-json3-dev

echo "安装完成！"