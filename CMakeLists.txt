cmake_minimum_required(VERSION 4.1)
project(Minesweeper)

enable_language(OBJCXX)
set(CMAKE_CXX_STANDARD 23)

file(GLOB_RECURSE SOURCES "src/*.mm" "src/*.cpp")
add_executable(Minesweeper ${SOURCES})

find_library(COCOA_FRAMEWORK Cocoa)

if(COCOA_FRAMEWORK)
    target_link_libraries(Minesweeper PRIVATE ${COCOA_FRAMEWORK})
else()
    message(FATAL_ERROR "Cocoa not found!")
endif()