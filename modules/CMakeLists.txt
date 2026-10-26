set(MODULES_DIR ${PROJECT_SOURCE_DIR}/modules)

set(GLEW_DIR  ${MODULES_DIR}/glew)

# glm and vkm_log are the consuming project's to provide: vkm_gl links them but is not
# their only user - glm crosses this library's own API - so a copy in here would be a
# second one, and two copies of glm across a module boundary is a layout mismatch
# waiting to happen. Fail with the reason rather than with whatever error a missing
# target produces three files later.
foreach(_vkm_gl_required glm vkm_log)
	if(NOT TARGET ${_vkm_gl_required})
		message(FATAL_ERROR
			"vkm_gl requires the target '${_vkm_gl_required}', which the consuming "
			"project must add before add_subdirectory(vkmGL). vkm_gl vendors only "
			"glew; see the engine's modules/CMakeLists.txt for the expected order.")
	endif()
endforeach()

message("[VKM_GL] Modules:")
message("\t- glew | Path: ${GLEW_DIR} (https://github.com/nigels-com/glew)")

# Define compile definitions
target_compile_definitions(${PROJECT_NAME} PUBLIC
	GLM_ENABLE_EXPERIMENTAL
)




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

  set(BUILD_UTILS  OFF CACHE BOOL "" FORCE)
  set(GLEW_INSTALL OFF CACHE BOOL "" FORCE)
  # EXCLUDE_FROM_ALL keeps glew's own install rules from firing: it installs its
  # headers and pkgconfig unconditionally, and a parent packaging an SDK decides
  # what ships rather than inheriting whatever a dependency felt like adding.
  add_subdirectory(${GLEW_DIR}/build/cmake EXCLUDE_FROM_ALL)
endif()

# Set the target's include directories
# glew's headers are vkm_gl's own; glm's and vkm_log's come with their targets.
# Split so the path resolves against this tree while building and against an
# installed prefix when vkm_gl travels inside an SDK.
target_include_directories(${PROJECT_NAME} PUBLIC
	$<BUILD_INTERFACE:${GLEW_DIR}/include>
	$<INSTALL_INTERFACE:include>
)

