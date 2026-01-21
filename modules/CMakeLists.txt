set(MODULES_DIR ${PROJECT_SOURCE_DIR}/modules)

set(GLFW_DIR  ${MODULES_DIR}/glfw)
set(GLM_DIR   ${MODULES_DIR}/glm)
set(STB_DIR   ${MODULES_DIR}/stb)
set(GLEW_DIR  ${MODULES_DIR}/glew)
set(VKMLOG_DIR ${MODULES_DIR}/vkmLog)

message("[VKMGL] Modules:")
message("\t- glfw | Path: ${GLFW_DIR} (https://github.com/glfw/glfw)")
message("\t- glm  | Path: ${GLM_DIR} (https://github.com/g-truc/glm)")
message("\t- stb  | Path: ${STB_DIR} (https://github.com/nothings/stb)")
message("\t- glew | Path: ${GLEW_DIR} (https://github.com/nigels-com/glew)")
message("\t- vkmLog | Path: ${VKMLOG_DIR} (https://github.com/V-KMilev/vkmLog)")

set(HEADERS
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

# GLEW - Auto-download pre-built release if needed
if (TARGET glew)
  message(STATUS "Target glew exists, will link to it.")
else()
  # Check if GLEW source files exist
  if (NOT EXISTS "${GLEW_DIR}/src/glew.c" OR NOT EXISTS "${GLEW_DIR}/include/GL/glew.h")
    message(STATUS "GLEW source files not found, downloading pre-built release...")

    set(GLEW_ZIP "${CMAKE_CURRENT_BINARY_DIR}/glew-2.3.0.zip")
    set(GLEW_URL "https://github.com/nigels-com/glew/releases/download/glew-2.3.0/glew-2.3.0.zip")

    # Download GLEW
    if (NOT EXISTS "${GLEW_ZIP}")
      message(STATUS "Downloading ${GLEW_URL}...")
      file(DOWNLOAD "${GLEW_URL}" "${GLEW_ZIP}"
           STATUS DOWNLOAD_STATUS
           SHOW_PROGRESS)
      list(GET DOWNLOAD_STATUS 0 DOWNLOAD_ERROR)
      if (NOT DOWNLOAD_ERROR EQUAL 0)
        message(FATAL_ERROR "Failed to download GLEW from ${GLEW_URL}")
      endif()
    endif()

    # Extract GLEW to temporary location
    message(STATUS "Extracting GLEW...")
    set(GLEW_TEMP "${CMAKE_CURRENT_BINARY_DIR}/glew-temp")
    file(ARCHIVE_EXTRACT INPUT "${GLEW_ZIP}" DESTINATION "${GLEW_TEMP}")

    # Copy contents from glew-2.3.0 subdirectory to the actual GLEW_DIR
    file(GLOB GLEW_CONTENTS "${GLEW_TEMP}/glew-2.3.0/*")
    foreach(item ${GLEW_CONTENTS})
      get_filename_component(item_name "${item}" NAME)
      # Skip .git directory from submodule
      if (NOT item_name STREQUAL ".git")
        file(COPY "${item}" DESTINATION "${GLEW_DIR}/")
      endif()
    endforeach()

    # Clean up
    file(REMOVE_RECURSE "${GLEW_TEMP}")

    message(STATUS "GLEW extracted successfully to ${GLEW_DIR}")
  else()
    message(STATUS "GLEW source files found")
  endif()

  set(BUILD_UTILS OFF CACHE BOOL "" FORCE)
  add_subdirectory(${GLEW_DIR}/build/cmake)
endif()

# Set the target's include directories
target_include_directories(${PROJECT_NAME} PUBLIC
	${GLFW_DIR}/include
	${GLM_DIR}/glm
	${STB_DIR}
	${GLEW_DIR}/include
)

# Add the header files to the target (for IDE visibility)
target_sources(${PROJECT_NAME} PRIVATE ${HEADERS})
