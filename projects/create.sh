mkdir include
mkdir tests
mkdir assets
mkdir src
mkdir app

touch CMakeLists.txt
touch tests/CMakeLists.txt
touch src/CMakeLists.txt
touch app/CMakeLists.txt

echo "cmake_minimum_required(VERSION 3.16) 
project(my_vec LANGUAGES CXX) 
set(CMAKE_CXX_STANDARD 17)
" > CMakeLists.txt

echo "find_package(OpenCV REQUIRED)
message(STATUS "OpenCV ${OpenCV_VERSION} @ ${OpenCV_DIR}")
add_executable(tests ${PROJECT_SOURCE_DIR}/tests/tests.cpp)
target_compile_options(tests PRIVATE -Wall -Wextra)
target_link_directories(tests PRIVATE ${OpenCV_INUCLUDE_DIRS})
target_link_libraries(tests PRIVATE ${OpenCV_LIBS})
" > tests/CMakeLists.txt