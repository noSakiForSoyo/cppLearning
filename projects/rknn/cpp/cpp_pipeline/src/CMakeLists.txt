
add_library(pipeline STATIC 
        pipeline.cpp 
        video_file_source.cpp 
        resize_stage.cpp 
        video_sink.cpp
        camera_source.cpp
        postprocess/postprocess.cpp
        detect/detect_stage.cpp)

target_include_directories(pipeline PUBLIC
    ${PROJECT_SOURCE_DIR}/include
    ${OpenCV_INCLUDE_DIRS})

find_library(RKNNRT_LIB NAMES rknnrt)
if (RKNNRT_LIB)
    message(STATUS "rknnrt: ${RKNNRT_LIB}")
else()
    message(WARNING "rknnrt NOT found -> detect_stage.cpp 里的 rknn_* 符号将无法链接")
endif()

target_link_libraries(pipeline PUBLIC
    ${OpenCV_LIBS}
    ${RKNNRT_LIB}
    Threads::Threads)