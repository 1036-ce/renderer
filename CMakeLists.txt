cmake_minimum_required (VERSION 3.10)
project(renderer VERSION 0.0.1)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(src)

find_package(OpenMP)
if(OPENMP_FOUND)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS} ")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS} ")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS} ")
endif()


if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

enable_testing()

file(GLOB SOURCES ./src/* main.cpp)

add_executable(${PROJECT_NAME} ${SOURCES})

# add_subdirectory(test)
add_subdirectory(examples/example_msaa)
add_subdirectory(examples/example_shadow)
add_subdirectory(examples/example_ambient_occlusion)
