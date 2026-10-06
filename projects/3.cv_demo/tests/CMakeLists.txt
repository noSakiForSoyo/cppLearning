find_package(OpenCV REQUIRED)
message(STATUS "OpenCV ${OpenCV_VERSION} @ ${OpenCV_DIR}")  

add_executable(tests ${PROJECT_SOURCE_DIR}/tests/tests.cpp)
target_compile_options(tests PRIVATE -Wall -Wextra)

target_link_directories(tests PRIVATE ${OpenCV_INUCLUDE_DIRS})
target_link_libraries(tests PRIVATE ${OpenCV_LIBS})