cmake_minimum_required( VERSION 2.8 )
project( terrain )

set( CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/../../config/cmake )

# try to find a rman lib (set by the RMAN envvar)
find_package( 3Delight REQUIRED )

include_directories(
  ${CMAKE_SOURCE_DIR}
  ${3Delight_INCLUDE_DIR}
  )

add_executable(terrain main.c trimesh.c
  )

set (COMPILE_C_FLAGS "${3Delight_COMPILE_FLAGS}")
if (MSVC10)
    set (COMPILE_C_FLAGS "\"${3Delight_COMPILE_FLAGS}\" /TC")
elseif (MSVC90)
    set (COMPILE_C_FLAGS "\"${3Delight_COMPILE_FLAGS}\" /TC")
elseif (MSVC80)
    set (COMPILE_C_FLAGS "\"${3Delight_COMPILE_FLAGS}\" /TC")
endif()

set_target_properties( terrain
  PROPERTIES
  PREFIX ""
  OUTPUT_NAME "terrain"
  COMPILE_FLAGS ${COMPILE_C_FLAGS}
  )

target_link_libraries( terrain
  ${3Delight_LIBRARIES}
  )
