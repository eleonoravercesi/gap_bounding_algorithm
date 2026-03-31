# Gemini helped with this

cmake_minimum_required(VERSION 4.2)
project(math_prog_extended)

set(CMAKE_CXX_STANDARD 20)

# 1. Find the headers
find_path(PPL_INCLUDE_DIR ppl.hh)
include_directories(${PPL_INCLUDE_DIR})

# 2. Find the library file (usually libppl)
find_library(PPL_LIBRARY NAMES ppl)

add_executable(math_prog_extended main.cpp)

# 3. Link the library to your executable
if(PPL_LIBRARY)
    target_link_libraries(math_prog_extended PRIVATE ${PPL_LIBRARY})
else()
    message(FATAL_ERROR "PPL library not found!")
endif()