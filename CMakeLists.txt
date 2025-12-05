cmake_minimum_required(VERSION 3.25.0)

set(PROJECT_NAME vkmGL)
set(PROJECT_VERSION_MAJOR 0)
set(PROJECT_VERSION_MINOR 0)
set(PROJECT_VERSION_PATCH 2)
set(PROJECT_ROOT_DIR ${CMAKE_SOURCE_DIR})

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
	${CMAKE_CURRENT_SOURCE_DIR}/gl_object.cpp

	${CMAKE_CURRENT_SOURCE_DIR}/buffer/gl_vertex_buffer_layout.cpp

	${CMAKE_CURRENT_SOURCE_DIR}/buffer/memory/gl_buffer.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/buffer/memory/gl_vertex_buffer.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/buffer/memory/gl_index_buffer.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/buffer/memory/gl_uniform_buffer.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/buffer/memory/gl_shader_storage_buffer.cpp

	${CMAKE_CURRENT_SOURCE_DIR}/buffer/state/gl_vertex_array.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/buffer/state/gl_frame_buffer.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/buffer/state/gl_render_buffer.cpp

	${CMAKE_CURRENT_SOURCE_DIR}/core/gl_render.cpp

	${CMAKE_CURRENT_SOURCE_DIR}/shader/gl_shader.cpp

	${CMAKE_CURRENT_SOURCE_DIR}/texture/gl_texture.cpp

	${CMAKE_CURRENT_SOURCE_DIR}/utils/gl_error_handle.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/utils/gl_debug.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/utils/glad_impl.c
)

add_library(${PROJECT_NAME} SHARED ${SOURCES})

target_include_directories(${PROJECT_NAME} PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}/buffer
	${CMAKE_CURRENT_SOURCE_DIR}/buffer/memory
	${CMAKE_CURRENT_SOURCE_DIR}/buffer/state
	${CMAKE_CURRENT_SOURCE_DIR}/core
	${CMAKE_CURRENT_SOURCE_DIR}/shader
	${CMAKE_CURRENT_SOURCE_DIR}/texture
	${CMAKE_CURRENT_SOURCE_DIR}/components
	${CMAKE_CURRENT_SOURCE_DIR}/utils
)

# External dependencies (submodules in modules/)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/modules)

target_include_directories(${PROJECT_NAME} PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}/modules/glfw/include
	${CMAKE_CURRENT_SOURCE_DIR}/modules/glm
	${CMAKE_CURRENT_SOURCE_DIR}/modules/stb
)

target_link_libraries(${PROJECT_NAME} PRIVATE
	vkmLog
)

target_link_libraries(${PROJECT_NAME} PUBLIC
	OpenGL::GL
	glfw
	glm
)
