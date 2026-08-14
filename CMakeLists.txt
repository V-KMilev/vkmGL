cmake_minimum_required(VERSION 3.25.0)

set(PROJECT_NAME vkmGL)
set(PROJECT_VERSION_MAJOR 1)
set(PROJECT_VERSION_MINOR 0)
set(PROJECT_VERSION_PATCH 0)

project(
  ${PROJECT_NAME}
  VERSION ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}
  LANGUAGES C CXX
)

# Set the C++ standard to 17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

message("[VKMGL] Building vkmGL v${PROJECT_VERSION}")

# Find OpenGL
find_package(OpenGL REQUIRED)

set(SOURCES
	${CMAKE_CURRENT_SOURCE_DIR}/src/core/gl_object.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/core/gl_context.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/core/gl_error_handle.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/core/gl_debug.cpp

	${CMAKE_CURRENT_SOURCE_DIR}/src/buffer/gl_buffer.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/buffer/gl_vertex_buffer.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/buffer/gl_index_buffer.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/buffer/gl_uniform_buffer.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/buffer/gl_shader_storage_buffer.cpp

	${CMAKE_CURRENT_SOURCE_DIR}/src/vertex/gl_vertex_array.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/vertex/gl_vertex_buffer_layout.cpp

	${CMAKE_CURRENT_SOURCE_DIR}/src/target/gl_frame_buffer.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/target/gl_render_buffer.cpp

	${CMAKE_CURRENT_SOURCE_DIR}/src/shader/gl_shader_base.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/shader/gl_shader_preprocess.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/shader/gl_shader_reload.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/shader/gl_shader.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/shader/gl_compute_shader.cpp

	${CMAKE_CURRENT_SOURCE_DIR}/src/texture/gl_texture.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/texture/gl_texture_3d.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/texture/gl_sampler.cpp
)

add_library(${PROJECT_NAME} SHARED ${SOURCES})

# Warnings are on for our own sources only - the submodules under modules/ keep
# their own level. The set matches the engine's vkm_warnings so a file reads the
# same on either side of the boundary, per docs/guides/code-style.md.
set(_vkmgl_warnings
	-Wall
	-Wextra
	-Wno-unused-value      # VKM_ASSERT expands to `true` in release
	-Wno-unused-parameter  # interface stubs, placeholders
)
target_compile_options(${PROJECT_NAME} PRIVATE
	$<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:${_vkmgl_warnings}>
)

target_include_directories(${PROJECT_NAME} PUBLIC
	# src/ itself, so a consumer can also qualify: "texture/gl_texture.h".
	${CMAKE_CURRENT_SOURCE_DIR}/src

	${CMAKE_CURRENT_SOURCE_DIR}/src/core
	${CMAKE_CURRENT_SOURCE_DIR}/src/buffer
	${CMAKE_CURRENT_SOURCE_DIR}/src/vertex
	${CMAKE_CURRENT_SOURCE_DIR}/src/target
	${CMAKE_CURRENT_SOURCE_DIR}/src/texture
	${CMAKE_CURRENT_SOURCE_DIR}/src/shader
)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/modules)

# Tests cover only what runs without a GL context, so this binary stays usable
# on a build machine with no GPU. Built when vkmGL is the top-level project, or
# when a parent asks for it - the engine does not build them by default.
option(VKMGL_BUILD_TESTS "Build the vkmGL test suite" ${PROJECT_IS_TOP_LEVEL})

if(VKMGL_BUILD_TESTS)
	enable_testing()
	add_executable(vkmgl_tests ${CMAKE_CURRENT_SOURCE_DIR}/tests/vkmgl_tests.cpp)
	target_link_libraries(vkmgl_tests PRIVATE ${PROJECT_NAME})
	target_compile_options(vkmgl_tests PRIVATE
		$<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:${_vkmgl_warnings}>
	)
	add_test(NAME vkmgl COMMAND vkmgl_tests)
endif()


target_link_libraries(${PROJECT_NAME} PUBLIC
	OpenGL::GL
	glfw
	glm
	vkmLog
	glew
)
