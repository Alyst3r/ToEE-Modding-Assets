cmake_minimum_required(VERSION 3.25)

set(CMAKE_CXX_STANDARD 17)

project(ToEEModelViewer LANGUAGES CXX)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_subdirectory(libs/glad)
add_subdirectory(libs/glm)
add_subdirectory(libs/tinyfiledialogs)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "GLFW lib only" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "GLFW lib only" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "GLFW lib only" FORCE)
add_subdirectory(libs/glfw)

file(GLOB_RECURSE ToEEMV_sources CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp"
)

file(GLOB IMGUI_SOURCES
    libs/imgui/*.cpp
    libs/imgui/backends/imgui_impl_glfw.cpp
    libs/imgui/backends/imgui_impl_opengl3.cpp
)

if (WIN32)
    enable_language(RC)
    set(APP_ICON_RESOURCE ${CMAKE_CURRENT_SOURCE_DIR}/resources/res.rc)
    set(ToEEMV_res ${APP_ICON_RESOURCE})
endif()

add_executable(ToEEModelViewer
    ${ToEEMV_sources}
    ${IMGUI_SOURCES}
    ${ToEEMV_res}
)
target_include_directories(ToEEModelViewer PRIVATE
    libs/imgui
    libs/imgui/backends
)
target_link_libraries(ToEEModelViewer PRIVATE glm glad glfw tinyfiledialogs)
source_group(TREE "${CMAKE_SOURCE_DIR}/src" PREFIX "Model Viewer" FILES ${ToEEMV_sources})
