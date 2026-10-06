cmake_minimum_required(VERSION 3.16) 
project(my_vec LANGUAGES CXX) 
set(CMAKE_CXX_STANDARD 17)
find_package(OpenCV REQUIRED)

add_subdirectory(src)
add_subdirectory(tests)

add_subdirectory(app)