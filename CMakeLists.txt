cmake_minimum_required(VERSION 3.5)

project(GlibcDebug LANGUAGES CXX)

# 1. C++ 环境相关配置
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_VERBOSE_MAKEFILE ON)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_definitions(-Wall -pedantic)

if(CMAKE_BUILD_TYPE MATCHES "Debug" OR CMAKE_BUILD_TYPE MATCHES "RelWithDebInfo")
    add_definitions(-O0 -Wall -g2)#-fno-limit-debug-info
endif()
set(CMAKE_THREAD_LIBS_INIT "-lpthread")
find_package(Threads)

set(LOCAL_LINK_LIB pthread)

#find_package(spdlog QUIET CONFIG)
if(spdlog_FOUND)
    list(APPEND LOCAL_LINK_LIB spdlog::spdlog)
else()
    include_directories(Lib/spdlog)
endif()

#find_package(nlohmann_json QUIET CONFIG)
if(nlohmann_json_FOUND)
    list(APPEND LOCAL_LINK_LIB nlohmann_json::nlohmann_json)
else()
    include_directories(Lib/nlohmann)
endif()
## 指定头文件、库查找路径
include_directories(Include)
link_directories(Lib)
link_directories(/home/layton/Tools/glibc-debug/usr/lib)
link_directories(/home/layton/Tools/glibc-debug/elf/)
message(STATUS "LOCAL_LINK_LIB = ${LOCAL_LINK_LIB}")

add_subdirectory(test)
