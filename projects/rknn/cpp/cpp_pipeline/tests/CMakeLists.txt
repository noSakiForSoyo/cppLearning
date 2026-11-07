add_executable(tests tests.cpp)

target_compile_options(tests PRIVATE -Wall -Wextra)

target_link_libraries(tests PRIVATE ${OpenCV_LIBS} pipeline)