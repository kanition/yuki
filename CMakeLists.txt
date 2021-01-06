# 配置说明：包括Linux和Windows下的说明

# Linux下的说明
# 安装支持SSL的curl，一般是依次安装openssl和curl，否则无法解析https
# 基本都是./configure和make及make install的做法
# 1. openssl安装见https://github.com/openssl/openssl/blob/master/NOTES-Unix.md
# 2. curl安装见https://curl.se/docs/install.html
#    安装curl时指定openssl库和安装路径的示例:
#    env PKG_CONFIG_PATH=/path/to/OpenSSL/lib/pkgconfig ./configure --with-ssl --prefix=/path/to/curl/tree
# 3. release使用cmake --build . -DCMAKE_BUILD_TYPE=Release,否则默认为debug版

# Windows下的说明
# Windows下支持SSL的curl安装很麻烦，借助vcpkg管理安装则简单得多
# 1. 先安装vcpkg本身，路径不要包含中文和空格，详见https://github.com/Microsoft/vcpkg/#quick-start-windows
# 2. 然后安装curl静态库(-static)而非动态库，否则最终生成的是动态链接版的多个dll和exe集合:
#    见https://github.com/curl/curl/blob/master/docs/INSTALL.md#building-using-vcpkg
#    安装命令vcpkg install curl[tool]:x86-windows-static
# 3. 在文件夹build下使用命令cmake命令为
#    cmake .. -DCMAKE_TOOLCHAIN_FILE=path\to\vcpkg\scripts\buildsystems\vcpkg.cmake -DVCPKG_TARGET_TRIPLET=x86-windows-static
#    然后release使用cmake --build . --config Release,否则默认为debug版

cmake_minimum_required(VERSION 3.10)
project(yuki VERSION 1.0)


set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_DEBUG_POSTFIX d)
string(COMPARE EQUAL "${CMAKE_BUILD_TYPE}" "Debug" DEBUG)

INCLUDE (CheckIncludeFiles)
if(UNIX AND NOT APPLE)
    CHECK_INCLUDE_FILES ("unistd.h;sys/types.h;sys/stat.h" LINUX_OK_H) # Linux关于文件夹处理的头文件
    if(NOT LINUX_OK_H)
        message(FATAL_ERROR "Not Found headers: unistd.h;sys/types.h;sys/stat.h")
    endif(NOT LINUX_OK_H)
    include_directories("~/program/curl/include")
    link_directories("~/program/curl/lib") # 链接curl的路径
elseif(WIN32)
    CHECK_INCLUDE_FILES("io.h;direct.h;errno.h;windows.h" WIN_OK_H) # Win关于文件夹处理的头文件
    if(NOT WIN_OK_H)
        message(FATAL_ERROR "Not Found headers: io.h;direct.h;errno.h;windows.h")
    endif(NOT WIN_OK_H)
endif()

CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/yuki.h.in yuki.h) # 配置版本号和系统等
CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/util.h.in util.h) # 配置版本号和系统等
aux_source_directory("./" src_files) # 加入本项目源码
add_executable(yuki ${src_files})
target_include_directories(yuki
    PUBLIC "${PROJECT_BINARY_DIR}"# 给出yuki.h.in生成的yuki.h的目录
    PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/json/single_include") # 见https://github.com/nlohmann/json
set_target_properties(yuki PROPERTIES DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX})

if(UNIX AND NOT APPLE)
    target_link_libraries(yuki PRIVATE "curl") # 链接curl
elseif(WIN32)
    find_package(CURL CONFIG REQUIRED)
    target_link_libraries(yuki PRIVATE CURL::libcurl) # 链接curl
endif()

install(TARGETS yuki DESTINATION bin)

include(InstallRequiredSystemLibraries)
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set(CPACK_PACKAGE_VERSION_MAJOR "${YUKI_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${YUKI_VERSION_MINOR}")
include(CPack)