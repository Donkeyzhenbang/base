# CMake 最低版本要求
cmake_minimum_required(VERSION 3.10)

# 项目名称
project(design_pattern)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# 设置编译输出路径
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/build/bin)

# 包含头文件目录
include_directories(${PROJECT_SOURCE_DIR}/inc)

# 可执行文件 target 定义
add_executable(observer_pattern1 src/observer_pattern1.cpp)
add_executable(observer_pattern2 src/observer_pattern2.cpp)
add_executable(factory_pattern src/factory_pattern.cpp)
add_executable(virtual_template src/virtual_template.cpp)
add_executable(virtual_test src/virtual_test.cpp)