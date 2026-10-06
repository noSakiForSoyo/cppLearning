find_package(OpenCV REQUIRED)
find_package(Threads REQUIRED)
message(STATUS "OpenCV version: ${OpenCV_VERSION}")

add_executable(app main.cpp)

target_compile_options(app PRIVATE -Wall -Wextra)

target_include_directories(app PRIVATE ${OpenCV_INCLUDE_DIRS})
target_link_libraries(app PRIVATE ${OpenCV_LIBS} gauss gray pipe)