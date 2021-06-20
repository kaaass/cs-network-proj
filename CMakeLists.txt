cmake_minimum_required(VERSION 3.9)
project(cs_network)

set(CMAKE_CXX_STANDARD 17)

# Setup externals
include(externals/CMakeLists.txt)

# Find source code
file(GLOB_RECURSE src_files "src/*.c" "src/*.cc" "src/*.cpp" "src/*.h" "src/*.hpp")

# Server Binary
add_executable(chat chat.cpp ${src_files})
target_link_libraries(chat Threads::Threads json ${glog_LIBRARY} ${Unwind_LIBRARY})
target_include_directories(chat PUBLIC ${glog_INCLUDE})
add_coverage(chat)

# Unit Test
enable_testing()
option(BUILD_TEST "Build the unit tests" ON)
if (BUILD_TEST)
    add_subdirectory(test)
endif ()
coverage_evaluate()