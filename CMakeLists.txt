cmake_minimum_required(VERSION 3.10)
project(math_prog_extended)

set(CMAKE_CXX_STANDARD 20)

# For Boost
cmake_policy(SET CMP0167 NEW)

# ======================
# Setup Gurobi - IMPROVED
# ======================
# Check if GUROBI_HOME is set
if(NOT DEFINED ENV{GUROBI_HOME})
    message(FATAL_ERROR "GUROBI_HOME environment variable is not set. Please set it to your Gurobi installation directory.")
endif()

# TODO should work, but as I am DEVASTATED, I set in manually
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
# Setup GMP and GMPXX (using pkg-config)
# ======================
find_package(PkgConfig REQUIRED)
pkg_check_modules(GMP REQUIRED gmp)
pkg_check_modules(GMPXX REQUIRED gmpxx)

message(STATUS "GMP found: ${GMP_LIBRARIES}")
message(STATUS "GMPXX found: ${GMPXX_LIBRARIES}")

# ======================
# Setup PPL
# ======================
find_path(PPL_INCLUDE_DIR ppl.hh)
find_library(PPL_LIBRARY NAMES ppl)

if(NOT PPL_LIBRARY)
    message(FATAL_ERROR "PPL library not found. Install libppl-dev.")
endif()

message(STATUS "PPL library found: ${PPL_LIBRARY}")
message(STATUS "PPL include dir: ${PPL_INCLUDE_DIR}")


# ======================
# Setup Nauty
# ======================
## TODO this is ugly but what I am suppose to do? which nauty does not work
#set(NAUTY_DIR "/home/vercee/libraries/nauty2_9_3")
#
#find_path(NAUTY_INCLUDE_DIR nauty.h
#        PATHS ${NAUTY_DIR}
#        NO_DEFAULT_PATH
#)
#
#find_library(NAUTY_LIBRARY NAMES nauty
#        PATHS ${NAUTY_DIR}
#        PATH_SUFFIXES lib
#        NO_DEFAULT_PATH
#)
#
## If not found as library, check for object file
#if(NOT NAUTY_LIBRARY)
#    if(EXISTS "${NAUTY_DIR}/nauty.a")
#        set(NAUTY_LIBRARY "${NAUTY_DIR}/nauty.a")
#    elseif(EXISTS "${NAUTY_DIR}/nauty.o")
#        set(NAUTY_LIBRARY "${NAUTY_DIR}/nauty.o")
#    endif()
#endif()
#
#if(NOT NAUTY_INCLUDE_DIR OR NOT NAUTY_LIBRARY)
#    message(WARNING "Nauty not found. Isomorphism checking will be disabled.")
#    message(STATUS "  Expected header: ${NAUTY_DIR}/nauty.h")
#    message(STATUS "  Expected library: ${NAUTY_DIR}/nauty.a or ${NAUTY_DIR}/nauty.so")
#else()
#    message(STATUS "Nauty include dir: ${NAUTY_INCLUDE_DIR}")
#    message(STATUS "Nauty library: ${NAUTY_LIBRARY}")
#endif()

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
        src/vertices_generator.cpp
        src/graphs.cpp
)

# ======================
# Create executable
# ======================
add_executable(math_prog_extended ${SOURCES})


# ===========================
# Set up Boost
# =====================
find_package(Boost REQUIRED)
target_include_directories(math_prog_extended PRIVATE ${Boost_INCLUDE_DIRS})

# ======================
# Include Directories
# ======================
target_include_directories(math_prog_extended PRIVATE
        include
        ${GUROBI_DIR}/include     # Gurobi headers
        ${PPL_INCLUDE_DIR}
        /usr/include/
        ${GMP_INCLUDE_DIRS}
        ${GMPXX_INCLUDE_DIRS}
)

## Add Nauty include if found
#if(NAUTY_INCLUDE_DIR)
#    target_include_directories(math_prog_extended PRIVATE ${NAUTY_INCLUDE_DIR})
#    add_compile_definitions(HAVE_NAUTY)  # Define macro so code can check if nauty is available
#endif()

# ======================
# Link Directories
# ======================
target_link_directories(math_prog_extended PRIVATE
        ${GUROBI_DIR}/lib
)

# ======================
# Link Libraries (ORDER MATTERS!)
# ======================
target_link_libraries(math_prog_extended PRIVATE
        ${GMP_LIBRARIES}
        ${GMPXX_LIBRARIES}
        ${PPL_LIBRARY}
        ${GUROBI_CXX_LIBRARY}
        ${GUROBI_LIBRARY}
)

## Add Nauty library if found
#if(NAUTY_LIBRARY)
#    target_link_libraries(math_prog_extended PRIVATE ${NAUTY_LIBRARY})
#endif()

# ======================
# Set output name
# ======================
set_target_properties(math_prog_extended PROPERTIES OUTPUT_NAME "solver")
