cmake_minimum_required(VERSION 3.10)

project(yuki VERSION 1.0)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
# SET(CMAKE_BUILD_TYPE "Release")

INCLUDE (CheckIncludeFiles)
if(UNIX AND NOT APPLE)
    CHECK_INCLUDE_FILES ("unistd.h;sys/types.h;sys/stat.h" LINUX_OK_H) # Linux关于文件夹处理的头文件
    if(NOT LINUX_OK_H)
        message(FATAL_ERROR "Not Found headers: unistd.h;sys/types.h;sys/stat.h")
    endif(NOT LINUX_OK_H)
    # linux下安装openssl和curl，如果只安装curl会无法解析https，假设都安到~/program
    # openssl安装见https://github.com/openssl/openssl/blob/master/NOTES-Unix.md
    # curl安装见https://curl.se/docs/install.html
    include_directories("~/program/curl/include")
    link_directories("~/program/curl/lib") # 链接curl的路径
    # 下载json解析源码（不需要安装，是个单头文件库，一起参加编译即可），假设下载到了~/setup
    # 见https://github.com/nlohmann/json
    include_directories("~/setup/json/single_include") # 实际上只有nlohmann/json.hpp
elseif(WIN32)
    CHECK_INCLUDE_FILES("io.h;direct.h;errno.h;windows.h" WIN_OK_H) # Win关于文件夹处理的头文件
    if(NOT WIN_OK_H)
        message(FATAL_ERROR "Not Found headers: io.h;direct.h;errno.h;windows.h")
    endif(NOT WIN_OK_H)
    # 下载json解析源码（不需要安装，是个单头文件库，一起参加编译即可），假设下载到了与yuki同级的目录
    # 见https://github.com/nlohmann/json
    include_directories("${CMAKE_CURRENT_SOURCE_DIR}/../json/single_include") # 实际上只有nlohmann/json.hpp
endif()

CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/yuki.h.in yuki.h) # 配置版本号
aux_source_directory("./" src_files) # 加入本项目源码
add_executable(yuki ${src_files})
target_include_directories(yuki PUBLIC "${PROJECT_BINARY_DIR}") # 给出yuki.h.in生成的yuki.h的目录

if(UNIX AND NOT APPLE)
    target_link_libraries(yuki PUBLIC "curl") # 链接curl
elseif(WIN32)
    # win下openssl和curl安装很麻烦，借助vcpkg管理安装
    # 见https://github.com/curl/curl/blob/master/docs/INSTALL.md#building-using-vcpkg
    # 假设安装后的位置在C:/dev/vcpkg/installed/x86-windows
    # 使用set提供CURLConfig.camke的路径比自己写配置更便捷
    set(CURL_DIR "C:/dev/vcpkg/installed/x86-windows/share/curl")
    find_package(CURL CONFIG REQUIRED)
    target_link_libraries(yuki PRIVATE CURL::libcurl) # 链接curl
endif()