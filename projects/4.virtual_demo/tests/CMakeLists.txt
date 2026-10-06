find_package(OpenCV REQUIRED)
message(STATUS "OpenCV version: ${OpenCV_VERSION}")

add_executable(tests tests.cpp)

target_compile_options(tests PRIVATE -Wall -Wextra)

target_include_directories(tests PRIVATE ${OpenCV_INCLUDE_DIRS})
target_link_libraries(tests PRIVATE ${OpenCV_LIBS} gauss gray pipe)