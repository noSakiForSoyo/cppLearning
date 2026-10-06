cmake_minimum_required(VERSION 3.16)
project(test_demo LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)

add_executable(demo src/main.cpp)
add_library(math src/math_util.cpp)
target_include_directories(math PUBLIC include/)

target_link_libraries(demo PRIVATE math)