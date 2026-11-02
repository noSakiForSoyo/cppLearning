add_executable(tests tests.cpp)

target_compile_options(tests PRIVATE -Wall -Wextra)

add_executable(benchmark benchmark.cpp)
target_link_libraries(benchmark PRIVATE pipeline)
target_compile_options(benchmark PRIVATE -O2 -Wall -Wextra)

target_link_libraries(tests PRIVATE ${OpenCV_LIBS} pipeline)