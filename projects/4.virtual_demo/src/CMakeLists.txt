add_library(pipe STATIC pipe_line.cpp)
target_include_directories(pipe PUBLIC ${CMAKE_SOURCE_DIR}/include ${OpenCV_INCLUDE_DIRS})
target_link_libraries(pipe PUBLIC ${OpenCV_LIBS})

add_library(gauss STATIC gauss_blur.cpp)
target_include_directories(gauss PUBLIC ${PROJECT_SOURCE_DIR}/include ${OpenCV_INCLUDE_DIRS})
target_link_libraries(gauss PUBLIC ${OpenCV_LIBS})

add_library(gray STATIC gray_scale.cpp)
target_include_directories(gray PUBLIC ${PROJECT_SOURCE_DIR}/include ${OpenCV_INCLUDE_DIRS})
target_link_libraries(gray PUBLIC ${OpenCV_LIBS})
