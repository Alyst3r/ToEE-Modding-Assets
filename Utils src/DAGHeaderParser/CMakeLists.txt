FILE(GLOB patcher-sources ${CMAKE_CURRENT_SOURCE_DIR}/*)
add_executable(DAGHeaderParser ${patcher-sources})
set_property(TARGET DAGHeaderParser PROPERTY CXX_STANDARD 17)
