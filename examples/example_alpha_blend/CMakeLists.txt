cmake_minimum_required (VERSION 3.10)

project(example_alpha_blend)

# C++ 17 is required
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(../../src)

# message(${})
# set execute file output path
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/../../bin)

file(GLOB SOURCES ../../src/* main.cpp)
# Create the executable
add_executable(${PROJECT_NAME} ${SOURCES})
