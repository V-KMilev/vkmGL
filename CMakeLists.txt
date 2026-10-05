cmake_minimum_required(VERSION 3.25.0)

set(PROJECT_NAME vkm_gl)
set(PROJECT_VERSION_MAJOR 1)
set(PROJECT_VERSION_MINOR 1)
set(PROJECT_VERSION_PATCH 6)

project(
  ${PROJECT_NAME}
  VERSION ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}
  LANGUAGES C CXX
)

# Set the C++ standard to 17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

message("[VKM_GL] Building vkm_gl v${PROJECT_VERSION}")

# Find OpenGL
find_package(OpenGL REQUIRED)

# Enumerated as one sweep rather than as a list: a list has to be edited to add
# a file, and the edit is invisible when it is forgotten - the new .cpp simply
# does not link, and the first sign is a missing symbol somewhere else.
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

add_library(${PROJECT_NAME} SHARED ${SOURCES})

# The public surface is not annotated with __declspec(dllexport), so on Windows
# CMake generates the export table from the object files instead - the same
# property vkm_core carries, and for the same reason. Without it an MSVC build
# links a DLL that exports nothing.
set_target_properties(${PROJECT_NAME} PROPERTIES
	WINDOWS_EXPORT_ALL_SYMBOLS ON
)

# Warnings are on for our own sources only - the submodules under modules/ keep
# their own level. The set matches the engine's vkm_warnings so a file reads the
# same on either side of the boundary, per docs/guides/code-style.md.
set(_vkm_gl_warnings
	-Wall
	-Wextra
	-Wno-unused-value      # VKM_ASSERT expands to `true` in release
	-Wno-unused-parameter  # interface stubs, placeholders
)
target_compile_options(${PROJECT_NAME} PRIVATE
	$<$<CXX_COMPILER_ID:GNU,Clang>:${_vkm_gl_warnings}>
)

# One spelling for a vkmGL header, and it is the flat one: #include "gl_texture.h".
# Exporting src/ as well let the same header be reached two ways, and both were
# in use - which meant a reader could not tell, from an include line, whether
# two files were including the same thing.
target_include_directories(${PROJECT_NAME} PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}/src/core
	${CMAKE_CURRENT_SOURCE_DIR}/src/buffer
	${CMAKE_CURRENT_SOURCE_DIR}/src/vertex
	${CMAKE_CURRENT_SOURCE_DIR}/src/target
	${CMAKE_CURRENT_SOURCE_DIR}/src/texture
	${CMAKE_CURRENT_SOURCE_DIR}/src/shader
)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/modules)

# Tests cover only what runs without a GL context, so this binary stays usable
# on a build machine with no GPU. Built when vkm_gl is the top-level project, or
# when a parent asks for it - the engine does not build them by default.
option(VKM_GL_BUILD_TESTS "Build the vkm_gl test suite" ${PROJECT_IS_TOP_LEVEL})

if(VKM_GL_BUILD_TESTS)
	enable_testing()
	add_executable(vkm_gl_tests ${CMAKE_CURRENT_SOURCE_DIR}/tests/vkm_gl_tests.cpp)
	target_link_libraries(vkm_gl_tests PRIVATE ${PROJECT_NAME})
	target_compile_options(vkm_gl_tests PRIVATE
		$<$<CXX_COMPILER_ID:GNU,Clang>:${_vkm_gl_warnings}>
	)
	add_test(NAME vkm_gl COMMAND vkm_gl_tests)
endif()

target_link_libraries(${PROJECT_NAME} PUBLIC
	OpenGL::GL
	glew
	glm
	vkm_log
)
