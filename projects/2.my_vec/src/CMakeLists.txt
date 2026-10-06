add_library(vec STATIC myVec.cpp)
target_include_directories(vec PUBLIC ${PROJECT_SOURCE_DIR}/include/myVec)

add_library(vec_unique STATIC myVec_unique.cpp)
target_include_directories(vec_unique PUBLIC ${PROJECT_SOURCE_DIR}/include/myVec_unique)

add_library(myVec INTERFACE)
target_include_directories(myVec INTERFACE ${PROJECT_SOURCE_DIR}/include/myVecT)
target_sources(myVec INTERFACE ${PROJECT_SOURCE_DIR}/include/myVecT/myVecT.hpp)
