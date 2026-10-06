cmake_minimum_required(VERSION 3.16)
project(my_vec LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)


# add_executable(test src/test.cpp)
# target_compile_options(test PRIVATE -Wall -Wextra)

add_subdirectory(src)
add_subdirectory(tests)
# add_library(myvec INTERFACE)
# add_library(vec STATIC src/myVec.cpp)
# add_library(vec_unique STATIC src/myVec_unique.cpp)
# target_include_directories(myvec INTERFACE include/myVecT)
# target_include_directories(vec PUBLIC include/myVec)
# target_include_directories(vec_unique PUBLIC include/myVec_unique)

# target_sources(myvec INTERFACE include/myVecT/myVecT.hpp)

# target_link_libraries(test PRIVATE myvec vec vec_unique)