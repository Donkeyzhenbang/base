# CMake 最低版本要求
cmake_minimum_required(VERSION 3.10)

# 项目名称
project(check_game)

# 指定 C 和 C++ 编译器
# set(CMAKE_C_COMPILER "/usr/bin/gcc")
# set(CMAKE_CXX_COMPILER "/usr/bin/g++")
set(CMAKE_C_COMPILER "/usr/bin/x86_64-w64-mingw32-gcc")
set(CMAKE_CXX_COMPILER "/usr/bin/x86_64-w64-mingw32-g++")

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++")

# 设置编译输出路径
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

# 链接库文件目录
link_directories(${PROJECT_SOURCE_DIR}/lib)

# 包含头文件目录
include_directories(${PROJECT_SOURCE_DIR}/inc)

# 可执行文件 target 定义
add_executable(check_game src/main.cpp)

# 链接静态库
target_link_libraries(check_game easyx)
