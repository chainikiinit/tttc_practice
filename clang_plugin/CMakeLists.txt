cmake_minimum_required(VERSION 3.10)
project(UnusedVariablePlugin)

find_package(Clang REQUIRED CONFIG)

include_directories(${CLANG_INCLUDE_DIRS})

add_library(UnusedVariablePlugin SHARED clang_plugin/UnusedVariablePlugin.cpp)
target_include_directories(UnusedVariablePlugin PRIVATE ${CLANG_INCLUDE_DIRS})
target_link_libraries(UnusedVariablePlugin PRIVATE clang)











