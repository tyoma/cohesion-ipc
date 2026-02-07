cmake_minimum_required(VERSION 3.13)

include_directories($<$<PLATFORM_ID:Windows>:${CMAKE_CURRENT_LIST_DIR}/../compat/win32>)

if (WIN32)
	set(PCPP "win32.cpp")
elseif (APPLE)
	set(PCPP "macos.cpp")
elseif (UNIX)
	set(PCPP "linux.cpp")
endif()

set(IPC_TESTS_SOURCES
	ClientSessionTests.cpp
	EndpointSelectorTests.cpp
	helpers.cpp
	helpers_sockets.cpp
	module.${PCPP}
	path.cpp
	ServerSessionContinuationTests.cpp
	ServerSessionTests.cpp
	SocketsEndpointClientTests.cpp
	SocketsEndpointServerTests.cpp
	SpawnEndpointClientTests.cpp
	SpawnEndpointServerTests.cpp
	test_constants.cpp
)

if (WIN32)
	set(IPC_TESTS_SOURCES ${IPC_TESTS_SOURCES}
		COMEndpointClientTests.cpp
		COMEndpointServerTests.cpp
		helpers_com.cpp
	)
endif()

add_library(cohesion-ipc.tests SHARED ${IPC_TESTS_SOURCES})
target_link_libraries(cohesion-ipc.tests cohesion-ipc)

add_executable(guinea_ipc_spawn guinea_ipc_spawn.cpp)
target_link_libraries(guinea_ipc_spawn mt)

add_executable(guinea_ipc_spawn_server guinea_ipc_spawn_server.cpp)
target_link_libraries(guinea_ipc_spawn_server cohesion-ipc.spawn cohesion-ipc)
