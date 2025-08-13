cmake_minimum_required(VERSION 2.8.12)
# 工程名
project(dlib_test)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -O2 -DDLIB_JPEG_SUPPORT")
 
IF(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Weverything")
ELSEIF(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
ENDIF()

# # 包含OpenCV
# find_package(OpenCV REQUIRED)
# if (OpenCV_FOUND)
#    include_directories(${OpenCV_INCLUDE_DIRS})
#    message("OpenCV found")
# endif()

# 包含X11
find_package(X11 REQUIRED)
if (X11_FOUND)
   include_directories(${X11_INCLUDE_DIR})
   message("X11 found")
   message(STATUS "X11_INCLUDE_DIR: ${X11_INCLUDE_DIR}")
endif()

#  添加dlib
find_package(dlib REQUIRED)
if (dlib_FOUND)
   include_directories(${dlib_INCLUDE_DIR})
   message("dlib found")
   message(STATUS "dlib_INCLUDE_DIR: ${dlib_INCLUDE_DIR}")
endif()
# include_directories(/home/code/cpp/dlib)
# link_directories(/home/code/cpp/dlib/build/dlib/)

# 添加代码
add_executable(dlib_test 3d_point_cloud_ex.cpp)
# 链接库
# libdlib.a - lpthread - lX11分别表示链接dlib，多线程，X11
target_link_libraries(dlib_test ${OpenCV_INCLUDE_LIBS} ${X11_LIBRARIES} libdlib.a -lpthread -lX11)
