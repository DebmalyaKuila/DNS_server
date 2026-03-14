cmake_minimum_required(VERSION 3.16)
project(DNS_SERVER VERSION 1.0.0 LANGUAGES C CXX)

if(NOT DEFINED DEBUG)
    set(DEBUG 0)
endif()
add_compile_definitions(DEBUG=${DEBUG})
add_subdirectory(includes)
add_subdirectory(src)

