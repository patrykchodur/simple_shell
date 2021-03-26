cmake_minimum_required(VERSION 3.7)

project(shell)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED YES)
set(CMAKE_CXX_FLAGS "-Wall -g")

set(CMAKE_C_FLAGS "-Wall -g")

set(CMAKE_EXPORT_COMPILE_COMMANDS YES)

set(SOURCE_FILES src/main.cpp src/lexer.cpp src/parser.cpp src/low_level.c src/built_in.c)

add_executable(${PROJECT_NAME} ${SOURCE_FILES})
target_include_directories(${PROJECT_NAME} PUBLIC include)
set(CMAKE_INSTALL_PREFIX ./)
