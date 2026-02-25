cmake_minimum_required(VERSION 3.20)
project(libvtermcpp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(vtermcpp STATIC
    src/terminal.cpp
    src/parser.cpp
    src/unicode.cpp
    src/encoding.cpp
    src/pen.cpp
    src/state.cpp
    src/screen.cpp
    src/scrollback.cpp
    src/keyboard.cpp
    src/mouse.cpp
)

target_include_directories(vtermcpp
    PUBLIC  ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
)

option(LIBVTERMCPP_BUILD_TESTS "Build tests" ON)
if(LIBVTERMCPP_BUILD_TESTS)
    add_subdirectory(test)
endif()
