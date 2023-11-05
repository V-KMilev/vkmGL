cmake_minimum_required(VERSION 3.21.2)
project(vkmGL VERSION 0.1.0 LANGUAGES C CXX)

# Set the C++ standard to 17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

message("[BUILDING:VKMGL] VKM-GL")

# Find OpenGL. Required for the library
find_package(OpenGL REQUIRED)

# Main target
add_library(${PROJECT_NAME} STATIC empty.cpp)

# Include the sources from subdirectories
add_subdirectory(${PROJECT_SOURCE_DIR}/modules)
add_subdirectory(${PROJECT_SOURCE_DIR}/common)
add_subdirectory(${PROJECT_SOURCE_DIR}/core)

# Link the main target with external libraries
target_link_libraries(${PROJECT_NAME} PUBLIC
	OpenGL::GL
)
