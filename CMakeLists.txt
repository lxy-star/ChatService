cmake_minimum_required(VERSION 3.15)
project(ChatService)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 源代码目录和头文件目录
set(PROJECT_INC
    ${CMAKE_SOURCE_DIR}/controller/include
    ${CMAKE_SOURCE_DIR}/infrastructure/include
    ${CMAKE_SOURCE_DIR}/logic/include
)

file(GLOB CONTROLLER_SRC ${CMAKE_SOURCE_DIR}/controller/src/*.cpp)
file(GLOB INFRA_SRC     ${CMAKE_SOURCE_DIR}/infrastructure/src/*.cpp)
file(GLOB LOGIC_SRC     ${CMAKE_SOURCE_DIR}/logic/src/*.cpp)

# 查找 MySQL Connector/C++ 库
find_library(MYSQL_LIB mysqlcppconn PATHS /usr/lib /usr/lib/x86_64-linux-gnu)

# 查找 pthread 库
find_package(Threads REQUIRED)

# nlohmann_json，如果你是用系统包管理安装的
find_package(nlohmann_json REQUIRED)

find_library(REDIS_LIB hiredis PATHS /usr/lib /usr/local/lib)

add_executable(MyServer main.cpp ${CONTROLLER_SRC} ${INFRA_SRC} ${LOGIC_SRC})

target_include_directories(MyServer PRIVATE
    ${PROJECT_INC}
    /usr/include/cppconn
)

target_link_libraries(MyServer PRIVATE
    ${MYSQL_LIB}
    Threads::Threads
    nlohmann_json::nlohmann_json
    ${REDIS_LIB}
)

# 输出目录
set(OUTPUT_DIR ${CMAKE_BINARY_DIR}/${PROJECT_NAME})
set_target_properties(MyServer PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR})

# 编译选项（可选）
target_compile_options(MyServer PRIVATE -Wall -Wextra -Wpedantic)

message(STATUS "INFRA_SRC = ${INFRA_SRC}")