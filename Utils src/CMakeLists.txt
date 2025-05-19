cmake_minimum_required(VERSION 3.22)
Include(FetchContent)
set(CMAKE_GENERATOR_PLATFORM x64)
cmake_policy(SET CMP0048 NEW)

project(DAGTools)

add_subdirectory(DAGHeaderParser)
add_subdirectory(DAGtoObjConverter)
add_subdirectory(ToEEModelViewer)
