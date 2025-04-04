FILE(GLOB dag-to-obj-conv-sources ${CMAKE_CURRENT_SOURCE_DIR}/*)
add_executable(DAGtoObjConverter ${dag-to-obj-conv-sources})
set_property(TARGET DAGtoObjConverter PROPERTY CXX_STANDARD 17)
