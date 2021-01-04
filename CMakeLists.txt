cmake_minimum_required(VERSION 3.10)

project(yuki VERSION 0.1)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

INCLUDE (CheckIncludeFiles)
if(UNIX AND NOT APPLE)
    CHECK_INCLUDE_FILES ("unistd.h;sys/types.h;sys/stat.h" LINUX_OK_H)
    if(NOT LINUX_OK_H)
        message(FATAL_ERROR "Not Found headers: unistd.h;sys/types.h;sys/stat.h")
    endif(NOT LINUX_OK_H)
    include_directories("~/program/curl/include" "~/setup/json/single_include")
    link_directories("~/program/curl/lib")
elseif(WIN32)
    CHECK_INCLUDE_FILES("io.h;direct.h" WIN_OK_H)
    if(NOT WIN_OK_H)
        message(FATAL_ERROR "Not Found headers: io.h;direct.h")
    endif(NOT WIN_OK_H)
    include_directories("${CMAKE_CURRENT_SOURCE_DIR}/../curl/include" "${CMAKE_CURRENT_SOURCE_DIR}/../json/single_include")
    link_directories("${CMAKE_CURRENT_SOURCE_DIR}/../curl/lib")
endif()

CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/yuki.h.in yuki.h)


aux_source_directory("./" src_files)
add_executable(yuki ${src_files})
target_include_directories(yuki PUBLIC "${PROJECT_BINARY_DIR}")
target_link_libraries(yuki PUBLIC "curl")