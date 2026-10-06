find_package(Threads REQUIRED)

add_executable(tests ${PROJECT_SOURCE_DIR}/tests/test.cpp)
target_compile_options(tests PRIVATE -Wall -Wextra)

target_link_libraries(tests PRIVATE myVec vec vec_unique Threads::Threads)
