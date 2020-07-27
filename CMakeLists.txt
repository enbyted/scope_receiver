cmake_minimum_required(VERSION 3.1)

project (scope_receiver)

find_package(ZLIB REQUIRED)

set(CXXOPTS_BUILD_EXAMPLES no)
set(SPDLOG_BUILD_EXAMPLE no)
set(CMAKE_CXX_STANDARD 17)

add_subdirectory(spdlog)
add_subdirectory(librigol)
add_subdirectory(cxxopts)

add_executable(scope_receiver
	src/main.cpp
	src/mat_writer.cpp
	src/mat_writer_compressed.cpp
)

set_target_properties(scope_receiver PROPERTIES CXX_STANDARD 17)
target_include_directories(scope_receiver PRIVATE ${ZLIB_INCLUDE_DIRS})
target_link_libraries(scope_receiver librigol spdlog cxxopts ${ZLIB_LIBRARIES})
