cmake_minimum_required(VERSION 3.22)

project(DAGHeaderParser)

FILE(GLOB dag-header-parser-sources ${CMAKE_CURRENT_SOURCE_DIR}/*)
add_executable(DAGHeaderParser ${dag-header-parser-sources})
set_property(TARGET DAGHeaderParser PROPERTY CXX_STANDARD 17)
