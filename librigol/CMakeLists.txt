
find_package(spdlog REQUIRED)

add_library(librigol
	src/scope.cpp
	src/connection.cpp
	src/tcp_connection.cpp
	src/scpi_command.cpp
)

target_include_directories(librigol PUBLIC include/)
target_link_libraries(librigol spdlog)
