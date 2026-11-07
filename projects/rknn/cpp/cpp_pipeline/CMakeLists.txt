cmake_minimum_required(VERSION 3.16)
project(cpp_pipeline LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenCV REQUIRED)
find_package(Threads REQUIRED)

add_subdirectory(src)
add_subdirectory(tests)
add_subdirectory(apps)
