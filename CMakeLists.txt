cmake_minimum_required(VERSION 3.20)
project(libvtermcpp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(sources.cmake)
list(TRANSFORM LIBVTERMCPP_SOURCES PREPEND "src/")

add_library(vtermcpp STATIC ${LIBVTERMCPP_SOURCES})

target_include_directories(vtermcpp
    PUBLIC  ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
)

option(LIBVTERMCPP_BUILD_TESTS "Build tests" ON)

if(LIBVTERMCPP_BUILD_TESTS)
    add_subdirectory(test)
endif()
