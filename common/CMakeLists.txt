message("[BUILDING:VKMGL] Common")

set(VKMGL_COMMON_DIR ${PROJECT_SOURCE_DIR}/common)

set(SOURCES
	${VKMGL_COMMON_DIR}/error/error_handle.cpp
	${VKMGL_COMMON_DIR}/files/file_read.cpp
)

# Set the target's include directories
target_include_directories(${PROJECT_NAME} PUBLIC
	${VKMGL_COMMON_DIR}/error
	${VKMGL_COMMON_DIR}/files
)

# Add the source files to the target
target_sources(${PROJECT_NAME} PRIVATE ${SOURCES})
