cmake_minimum_required(VERSION 3.25)
project(tinyfiledialogs)

add_library(tinyfiledialogs)
target_sources(tinyfiledialogs PRIVATE src/tinyfiledialogs.c)
target_include_directories(tinyfiledialogs PUBLIC include)
