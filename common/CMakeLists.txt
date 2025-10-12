message("[VKMGL] Common:")

set(VKMGL_COMMON_DIR ${PROJECT_SOURCE_DIR}/common)

set(SOURCES
	${VKMGL_COMMON_DIR}/file_read.cpp
	${VKMGL_COMMON_DIR}/glad_impl.c
	${VKMGL_COMMON_DIR}/gl_error_handle.cpp
)

# Set the target's include directories
target_include_directories(${PROJECT_NAME} PUBLIC
	${VKMGL_COMMON_DIR}
)

# Add the source files to the target
target_sources(${PROJECT_NAME} PRIVATE ${SOURCES})
