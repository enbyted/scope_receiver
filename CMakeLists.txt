cmake_minimum_required(VERSION 3.1)
project (scope_receiver)

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(spdlog)
add_subdirectory(librigol)
add_subdirectory(cxxopts)

add_executable(scope_receiver
	src/main.cpp
	src/mat_writer.cpp
)

target_link_libraries(scope_receiver librigol spdlog cxxopts)
