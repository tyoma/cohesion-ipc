cmake_minimum_required(VERSION 3.13)

include_directories($<$<PLATFORM_ID:Windows>:${CMAKE_CURRENT_LIST_DIR}/../compat/win32>)

set(IPC_SOURCES
	client_endpoint.cpp
	client_endpoint_sockets.cpp
	client_endpoint_spawn.cpp
	client_endpoint_spawn.${UWCPP}
	client_session.cpp
	misc.cpp
	server_endpoint.cpp
	server_endpoint_sockets.cpp
	server_session.cpp
	socket_helpers.${UWCPP}
	stream.cpp
)

if (WIN32)
	set(IPC_SOURCES ${IPC_SOURCES}
		client_endpoint_com.cpp
		com_init.cpp
		server_endpoint_com.cpp
	)
endif()

add_library(cohesion-ipc STATIC ${IPC_SOURCES})
target_include_directories(cohesion-ipc PUBLIC ${CMAKE_CURRENT_LIST_DIR}/..)
target_link_libraries(cohesion-ipc tasker strmd $<$<PLATFORM_ID:Windows>:ws2_32>)

add_library(cohesion-ipc.spawn STATIC server_endpoint_spawn.cpp)
target_include_directories(cohesion-ipc.spawn PUBLIC ${CMAKE_CURRENT_LIST_DIR}/..)
target_link_libraries(cohesion-ipc mt)
