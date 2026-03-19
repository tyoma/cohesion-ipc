cmake_minimum_required(VERSION 3.13)

include_directories($<$<PLATFORM_ID:Windows>:${CMAKE_CURRENT_LIST_DIR}/../compat/win32>)

set(IPC_SOURCES
	cancellable_read.${UWCPP}
	client_endpoint.cpp
	client_endpoint_sockets.cpp
	client_endpoint_spawn.cpp
	client_endpoint_spawn.${UWCPP}
	misc.cpp
	server_endpoint.cpp
	server_endpoint_stream.cpp
	server_endpoint_sockets.cpp
	socket_helpers.${UWCPP}
)

if (NOT COIPC_NO_STRMD)
	set(IPC_SOURCES ${IPC_SOURCES}
		strmd/client_session.cpp
		strmd/server_session.cpp
		strmd/stream.cpp
	)
endif()

if (WIN32)
	set(IPC_SOURCES ${IPC_SOURCES}
		client_endpoint_com.cpp
		com_init.cpp
		server_endpoint_com.cpp
	)
endif()

add_library(cohesion-ipc STATIC ${IPC_SOURCES})
target_include_directories(cohesion-ipc PUBLIC ${CMAKE_CURRENT_LIST_DIR}/..)
target_link_libraries(cohesion-ipc tasker $<$<PLATFORM_ID:Windows>:ws2_32>)
if (NOT COIPC_NO_STRMD)
	target_link_libraries(cohesion-ipc strmd)
endif()

add_library(cohesion-ipc.spawn STATIC server_endpoint_spawn.cpp)
target_include_directories(cohesion-ipc.spawn PUBLIC ${CMAKE_CURRENT_LIST_DIR}/..)
target_link_libraries(cohesion-ipc mt)
