cmake_minimum_required(VERSION 3.10) # 4.2 is very high/uncommon, 3.10 is safer
project(math_prog_extended)

set(CMAKE_CXX_STANDARD 20) # Use C++17 standard

# 1. Setup PPL
find_path(PPL_INCLUDE_DIR ppl.hh)
find_library(PPL_LIBRARY NAMES ppl)

# 2. Define all your SOURCE files (.cpp)
# Note: You don't need to list .h files in add_executable
set(SOURCES
        main.cpp
        src/solvers.cpp
        src/readers.cpp
        src/gap_related_functions.cpp
        src/utils.cpp
)

# 3. Create the executable using the SOURCES list
add_executable(math_prog_extended ${SOURCES})

# 4. Handle Include Directories (The -I flags)
target_include_directories(math_prog_extended PRIVATE
        include
        ${PPL_INCLUDE_DIR}
        /usr/include/
        /home/vercee/libraries/concorde/build
        # /usr/local/include/lemon

)

# 5. Handle Linking (The -L and -l flags)
target_link_libraries(math_prog_extended PRIVATE
        ${PPL_LIBRARY}
        scip              # If installed in /usr/lib, you can just use the name
        /home/vercee/libraries/concorde/build/concorde.a
        /home/vercee/libraries/concorde/cplex/libcplex.a
        m                 # Math library
        pthread           # Thread library (often required by SCIP/CPLEX)
        # /home/vercee/libraries/lemon-1.3/build/lemon/libemon.a
)

# 6. Set output name
set_target_properties(math_prog_extended PROPERTIES OUTPUT_NAME "solver")