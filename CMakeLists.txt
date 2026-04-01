# Gemini helped with this

# WARNING: All the paths are hard-coded, you might want to change them to fit your system setup.

cmake_minimum_required(VERSION 4.2)
project(math_prog_extended)

set(CMAKE_CXX_STANDARD 20)

# 1. Find the headers
find_path(PPL_INCLUDE_DIR ppl.hh)
include_directories(${PPL_INCLUDE_DIR})

# 2. Find the library file (usually libppl)
find_library(PPL_LIBRARY NAMES ppl)

add_executable(math_prog_extended main.cpp
        include/readers.h
        include/solvers.h)

# 3. Link the library to your executable
if(PPL_LIBRARY)
    target_link_libraries(math_prog_extended PRIVATE ${PPL_LIBRARY})
else()
    message(FATAL_ERROR "PPL library not found!")
endif()

# Path to header files (.h, .hpp)
target_include_directories(math_prog_extended PRIVATE
        ${PPL_INCLUDE_DIR}
        /usr/include/
        /home/vercee/libraries/concorde/build # For Concorde headers
        /home/vercee/libraries/concorde/cplex # For Concorde CPLEX headers
)

# Path to lib files (.so, .a)
target_link_libraries(math_prog_extended PRIVATE
        ${PPL_LIBRARY}
        /usr/lib/libscip.so
        /home/vercee/libraries/concorde/build/concorde.a # For Concorde library
        /home/vercee/libraries/concorde/cplex/libcplex.a # For CPLEX library
)

# Pick a name for the executable
set_target_properties(math_prog_extended PROPERTIES OUTPUT_NAME "./solver")