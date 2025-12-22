cmake_minimum_required(VERSION 3.10)
project(SortStrategyProject)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 包含目录
include_directories(include)

# 源文件
set(SOURCES
    src/BubbleSort.cpp
    src/BucketSort.cpp
    src/HeapSort.cpp
    src/MergeSort.cpp
    src/QuickSort.cpp
    src/Sorter.cpp
    main.cpp
)

# 创建可执行文件
add_executable(SortStrategyDemo ${SOURCES})

# 设置编译器选项
if(MSVC)
    target_compile_options(SortStrategyDemo PRIVATE /W4)
else()
    target_compile_options(SortStrategyDemo PRIVATE -Wall -Wextra -pedantic)
endif()