cmake_minimum_required(VERSION 3.7)

project(shell)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED YES)
set(CMAKE_CXX_FLAGS "-Wall -g")

set(CMAKE_EXPORT_COMPILE_COMMANDS YES)

set(SOURCE_FILES main.cpp )

add_executable(${PROJECT_NAME} ${SOURCE_FILES})
set(CMAKE_INSTALL_PREFIX ./)
