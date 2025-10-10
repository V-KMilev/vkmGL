set(MODULES_DIR ${PROJECT_SOURCE_DIR}/modules)

set(GLFW_DIR  ${MODULES_DIR}/glfw)
set(GLM_DIR   ${MODULES_DIR}/glm)
set(STB_DIR   ${MODULES_DIR}/stb)

message("[VKMGL] Modules:")
message("\t- glfw | Path: ${GLFW_DIR} (https://github.com/glfw/glfw)")
message("\t- glm  | Path: ${GLM_DIR} (https://github.com/g-truc/glm)")
message("\t- stb  | Path: ${STB_DIR} (https://github.com/nothings/stb)")

# GLAD is header-only, no source file needed
# set(SOURCES
# 	${GLFW_DIR}/deps/glad_gl.c
# )
set(HEADERS
	${GLFW_DIR}/deps/glad/gl.h

	${STB_DIR}
)

# Define compile definitions
target_compile_definitions(${PROJECT_NAME} PUBLIC
	GLM_ENABLE_EXPERIMENTAL
)

add_subdirectory(${GLFW_DIR})
add_subdirectory(${GLM_DIR})

# Set the target's include directories
target_include_directories(${PROJECT_NAME} PUBLIC
	${GLFW_DIR}/deps/glad
	${GLFW_DIR}/deps
	${GLFW_DIR}/include

	${GLM_DIR}/glm

	${STB_DIR}
)

# Add the header files to the target (for IDE visibility)
target_sources(${PROJECT_NAME} PRIVATE ${HEADERS})
