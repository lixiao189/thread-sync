cmake_minimum_required(VERSION 3.0.0)
project(thread-sync VERSION 0.1.0)

# Use CPP 11
set(CMAKE_CXX_STANDARD 11)
# Set leak detection when using clang compiler 
message("The compiler is ${CMAKE_CXX_COMPILER_ID}")
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" AND CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_CXX_FLAGS "-fsanitize=leak")
endif()

# Add the the third part header file
include_directories(include)

# Add cpp files to target
set(SRC_LIST src/main.cpp)
add_executable(thread-sync ${SRC_LIST})
