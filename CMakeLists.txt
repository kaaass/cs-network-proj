cmake_minimum_required(VERSION 3.17)
project(cs_network)

set(CMAKE_CXX_STANDARD 17)

# Setup externals
include(externals/CMakeLists.txt)

# Find source code
file(GLOB_RECURSE src_files "src/*.c" "src/*.cc" "src/*.cpp" "src/*.h" "src/*.hpp")

# Main Binary
add_executable(cs_network main.cpp ${src_files})
add_coverage(cs_network)

# Raw socket Binary
add_executable(raw_socket raw_socket.cpp ${src_files})
add_coverage(raw_socket)

# Unit Test
enable_testing()
option(BUILD_TEST "Build the unit tests" ON)
if (BUILD_TEST)
    add_subdirectory(test)
endif ()
coverage_evaluate()