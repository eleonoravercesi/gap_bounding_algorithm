cmake_minimum_required(VERSION 3.10)
project(gap_bounding_algorithm)

set(CMAKE_CXX_STANDARD 20)

## For Boost
#cmake_policy(SET CMP0167 NEW)


# ==================================================
# Setup Gurobi
# ==================================================
message(STATUS "Gurobi Setup:")

if(NOT DEFINED ENV{GUROBI_HOME})
    message(FATAL_ERROR
            "GUROBI_HOME environment variable is not set.\n"
            "Please set it to your Gurobi installation directory.\n")
endif()

find_path(GUROBI_INCLUDE_DIR
        NAMES gurobi_c++.h
        PATHS "$ENV{GUROBI_HOME}/include"
        REQUIRED
)
message(STATUS "    include dir: ${GUROBI_INCLUDE_DIR}")

find_library(GUROBI_LIBRARY
        NAMES gurobi130 gurobi120 gurobi110 gurobi100
        PATHS "$ENV{GUROBI_HOME}/lib"
        REQUIRED
)
message(STATUS "    library: ${GUROBI_LIBRARY}")

find_library(GUROBI_CXX_LIBRARY
        NAMES gurobi_c++
        PATHS "$ENV{GUROBI_HOME}/lib"
        REQUIRED
)
message(STATUS "    c++ library: ${GUROBI_CXX_LIBRARY}")


# ==================================================
# Setup GMP
# ==================================================
message(STATUS "GMP Setup:")

find_path(GMP_INCLUDE_DIR
        NAMES gmp.h
        REQUIRED
)
message(STATUS "    include dir: ${GMP_INCLUDE_DIR}")

find_library(GMP_LIBRARY
        NAMES gmp
        REQUIRED
)
message(STATUS "    library: ${GMP_LIBRARY}")

find_path(GMPXX_INCLUDE_DIR
        NAMES gmpxx.h
        REQUIRED
)
message(STATUS "    c++ include dir: ${GMPXX_INCLUDE_DIR}")

find_library(GMPXX_LIBRARY
        NAMES gmpxx
        REQUIRED
)
message(STATUS "    c++ library: ${GMPXX_LIBRARY}")


# ==================================================
# Setup PPL
# ==================================================
message(STATUS "PPL Setup:")

find_path(PPL_INCLUDE_DIR
        NAMES ppl.hh
        REQUIRED
)
message(STATUS "    include dir: ${PPL_INCLUDE_DIR}")

find_library(PPL_LIBRARY
        NAMES ppl
        REQUIRED
)
message(STATUS "    library: ${PPL_LIBRARY}")


# ==================================================
# Define SOURCE files
# ==================================================
set(SOURCES
        main.cpp
        src/readers.cpp
        src/utils.cpp
        src/solvers.cpp
        src/opt_plus.cpp
        src/GB_algorithm.cpp
        src/vertices_generator.cpp
        src/graphs.cpp
)


# ==================================================
# Create executable
# ==================================================
add_executable(gap_bounding_algorithm ${SOURCES})


# ==================================================
# Include Directories
# ==================================================
target_include_directories(gap_bounding_algorithm PRIVATE
        include
        /usr/include/
        ${GUROBI_INCLUDE_DIR}
        ${PPL_INCLUDE_DIR}
        ${GMP_INCLUDE_DIR}
        ${GMPXX_INCLUDE_DIR}
)


# ==================================================
# Link Libraries (ORDER MATTERS!)
# ==================================================
target_link_libraries(gap_bounding_algorithm PRIVATE
        ${PPL_LIBRARY}
        ${GMPXX_LIBRARY}
        ${GMP_LIBRARY}
        ${GUROBI_CXX_LIBRARY}
        ${GUROBI_LIBRARY}
)


# ==================================================
# Set output name
# ==================================================
set_target_properties(gap_bounding_algorithm PROPERTIES OUTPUT_NAME "GB")
