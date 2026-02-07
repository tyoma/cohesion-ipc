cmake_minimum_required(VERSION 3.13)

project(cohesion-ipc)

enable_testing()

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(COIPC_NO_TESTS "Do not build test modules." OFF)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/_lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/_bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/_bin)

if (UNIX)
	set(MP_MT_GENERIC 1)
	add_definitions(-DMP_MT_GENERIC)
endif()

# add_definitions(-D_SCL_SECURE_NO_WARNINGS -D_CRT_SECURE_NO_WARNINGS -D_SILENCE_TR1_NAMESPACE_DEPRECATION_WARNING)

set(GCC_CLANG_COMPILER_OPTIONS_CXX -std=c++11 -fPIC -Wall -fno-rtti)
set(GCC_CLANG_COMPILER_OPTIONS_CXX ${GCC_CLANG_COMPILER_OPTIONS_CXX}
	-Wall -Wextra -Wpedantic
	-Wno-missing-field-initializers
	-Wno-implicit-fallthrough
)

add_compile_options(
	"$<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:${GCC_CLANG_COMPILER_OPTIONS_CXX}>"
	"$<$<CXX_COMPILER_ID:MSVC>:-W4;-wd4512;-wd4355>"
	"$<$<CXX_COMPILER_ID:MSVC>:$<$<CONFIG:Release,RelWithDebInfo>:-Ox;-Ob2>>"
)

if (GNU)
	set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--no-undefined")
endif()

if (WIN32)
	set(UWCPP "win32.cpp")
elseif (UNIX)
	set(UWCPP "unix.cpp")
else ()
    message(FATAL_ERROR "Unsupported platform")
endif()


if (NOT TARGET strmd)
	set(STRMD_NO_TESTS ON)
	add_subdirectory(libraries/strmd)
endif()

if (NOT TARGET tasker)
	set(TASKER_NO_TESTS ON)
	add_subdirectory(libraries/tasker)
endif()

add_subdirectory(src)

if (NOT COIPC_NO_TESTS)
	if (NOT TARGET utee)
		set(UTEE_NO_TESTS ON)
		add_subdirectory(libraries/utee)
	endif()
	add_subdirectory(tests)
	add_utee_test(cohesion-ipc.tests)
endif()
