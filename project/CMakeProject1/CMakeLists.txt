cmake_minimum_required (VERSION 3.8)
project ("CMakeProject1")

set(MY_VERSION 1.0)
message(STATUS "Project Version: ${MY_VERSION}")

set(SRC_FILES main.cpp mymath/mymath.cpp)
foreach(f ${SRC_FILES})
    message(STATUS "将要编译: ${f}")
endforeach()

if(MSVC)
    message(STATUS "正在用 Visual Studio 编译器")
else()
    message(STATUS "不是 MSVC 编译器")
endif()

set(OpenCV_DIR "D:/opencv/build" CACHE PATH "OpenCV 安装路径")
find_package(OpenCV REQUIRED)


add_library(test CMakeProject1/test/test.cpp)
target_include_directories(test PUBLIC CMakeProject1/test/)

add_executable(demo CMakeProject1/main.cpp)

target_link_libraries(demo PRIVATE ${OpenCV_LIBS})
target_link_libraries(demo PRIVATE test)

file(GLOB OPENCV_DLL "${OpenCV_DIR}/x64/vc16/bin/*.dll")
add_custom_command(TARGET demo POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    ${OPENCV_DLL} $<TARGET_FILE_DIR:demo>)