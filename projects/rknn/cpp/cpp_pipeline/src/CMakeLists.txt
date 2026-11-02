
add_library(pipeline STATIC pipeline.cpp video_file_source.cpp resize_stage.cpp video_sink.cpp)

target_include_directories(pipeline PUBLIC
    ${PROJECT_SOURCE_DIR}/include
    ${OpenCV_INCLUDE_DIRS})

target_link_libraries(pipeline PUBLIC
    ${OpenCV_LIBS}
    Threads::Threads)