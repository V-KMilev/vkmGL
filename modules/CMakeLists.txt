set(MODULES_DIR ${PROJECT_SOURCE_DIR}/modules)

set(GLFW_DIR  ${MODULES_DIR}/glfw)
set(GLM_DIR   ${MODULES_DIR}/glm)
set(STB_DIR   ${MODULES_DIR}/stb)
set(VKMLOG_DIR ${MODULES_DIR}/vkmLog)

message("[VKMGL] Modules:")
message("\t- glfw | Path: ${GLFW_DIR} (https://github.com/glfw/glfw)")
message("\t- glm  | Path: ${GLM_DIR} (https://github.com/g-truc/glm)")
message("\t- stb  | Path: ${STB_DIR} (https://github.com/nothings/stb)")
message("\t- vkmLog | Path: ${VKMLOG_DIR} (https://github.com/V-KMilev/vkmLog)")

# GLAD is header-only, no source file needed
set(HEADERS
	${GLFW_DIR}/deps/glad/gl.h

	${STB_DIR}
)

# Define compile definitions
target_compile_definitions(${PROJECT_NAME} PUBLIC
	GLM_ENABLE_EXPERIMENTAL
)

if (TARGET glfw)
  message(STATUS "Target glfw exists, will link to it.")
else()
  add_subdirectory(${GLFW_DIR})
endif()

if (TARGET glm)
  message(STATUS "Target glm exists, will link to it.")
else()
  add_subdirectory(${GLM_DIR})
endif()

if (TARGET vkmLog)
  message(STATUS "Target vkmLog exists, will link to it.")
else()
  add_subdirectory(modules/vkmLog)
endif()

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
