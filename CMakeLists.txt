cmake_minimum_required(VERSION 3.9)
project(cs_network)

set(CMAKE_CXX_STANDARD 17)

# Setup externals
include(externals/CMakeLists.txt)

# Find source code
file(GLOB_RECURSE src_files "src/*.c" "src/*.cc" "src/*.cpp" "src/*.h" "src/*.hpp")

# Server Binary
add_executable(server server.cpp ${src_files})
target_link_libraries(server Threads::Threads json ${glog_LIBRARY} ${Unwind_LIBRARY})
target_include_directories(server PUBLIC ${glog_INCLUDE})
add_coverage(server)

# Client Binary
add_executable(client client.cpp ${src_files})
target_link_libraries(client Threads::Threads json ${glog_LIBRARY} ${Unwind_LIBRARY})
target_include_directories(client PUBLIC ${glog_INCLUDE})
add_coverage(client)

# Raw socket Binary
add_executable(raw_socket raw_socket.cpp)
target_link_libraries(raw_socket ${glog_LIBRARY} ${Unwind_LIBRARY})
target_include_directories(raw_socket PUBLIC ${glog_INCLUDE})
add_coverage(raw_socket)

# Unit Test
enable_testing()
option(BUILD_TEST "Build the unit tests" ON)
if (BUILD_TEST)
    add_subdirectory(test)
endif ()
coverage_evaluate()