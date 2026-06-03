cmake_minimum_required(VERSION 3.10)
project(math_prog_extended)

set(CMAKE_CXX_STANDARD 20)

# ======================
# Setup Gurobi - IMPROVED
# ======================
# Check if GUROBI_HOME is set
if(NOT DEFINED ENV{GUROBI_HOME})
    message(FATAL_ERROR "GUROBI_HOME environment variable is not set. Please set it to your Gurobi installation directory.")
endif()

# TODO should work, but as I am DEVASTATED
# set(GUROBI_DIR $ENV{GUROBI_HOME})
set(GUROBI_DIR "/opt/gurobi1302/linux64")
message(STATUS "GUROBI_HOME is set to: ${GUROBI_DIR}")

# Verify that the include directory exists
if(NOT EXISTS "${GUROBI_DIR}/include/gurobi_c.h")
    message(FATAL_ERROR "Cannot find gurobi_c.h in ${GUROBI_DIR}/include. Please check your GUROBI_HOME setting.")
endif()

# Determine the Gurobi library name based on version
# For Gurobi 13.0, the library is gurobi130/home/vercee/Documents
# Adjust this based on your actual Gurobi version
set(GUROBI_CXX_LIBRARY "gurobi_c++")
set(GUROBI_LIBRARY "gurobi130")

# ======================
# Setup PPL
# ======================
find_path(PPL_INCLUDE_DIR ppl.hh)
find_library(PPL_LIBRARY NAMES ppl)

# ======================
# Define SOURCE files
# ======================
set(SOURCES
        main.cpp
        src/readers.cpp
        src/utils.cpp
        src/solvers.cpp
        src/opt_plus.cpp
        src/GB_algorithm.cpp
)

# ======================
# Create executable
# ======================
add_executable(math_prog_extended ${SOURCES})

# ======================
# Include Directories
# ======================
target_include_directories(math_prog_extended PRIVATE
        include
        ${GUROBI_DIR}/include     # Gurobi headers
        ${PPL_INCLUDE_DIR}
        /usr/include/
)

# ======================
# Link Libraries (Modern CMake approach)
# ======================
target_link_directories(math_prog_extended PRIVATE
        ${GUROBI_DIR}/lib
)

target_link_libraries(math_prog_extended PRIVATE
        ${GUROBI_CXX_LIBRARY}    # Gurobi C++ library
        ${GUROBI_LIBRARY}        # Gurobi C library (IMPORTANT!)
        ${PPL_LIBRARY}

)

# ======================
# Set output name
# ======================
set_target_properties(math_prog_extended PROPERTIES OUTPUT_NAME "solver")
