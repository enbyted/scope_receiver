
add_library(librigol
	src/scope.cpp
	src/connection.cpp
	src/tcp_connection_unix.cpp
	src/tcp_connection_windows.cpp
	src/scpi_command.cpp
)

set_target_properties(librigol PROPERTIES CXX_STANDARD 17)
target_include_directories(librigol PUBLIC include/)
target_link_libraries(librigol spdlog)

if(WIN32)
	target_link_libraries(librigol Ws2_32.lib Mswsock.lib AdvApi32.lib)
endif()