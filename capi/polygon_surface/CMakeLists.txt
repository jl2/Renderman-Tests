cmake_minimum_required( VERSION 2.8 )
project( psurf )

#=====
# General
set( CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/../../config/cmake )

# try to find a rman lib (set by the RMAN envvar)
find_package( 3Delight REQUIRED )

include_directories(
  ${CMAKE_SOURCE_DIR}
  ${3Delight_INCLUDE_DIR}
  )

add_executable(psurf main.c
  )

set_target_properties( psurf
  PROPERTIES
  PREFIX ""
  OUTPUT_NAME "psurf"
  COMPILE_FLAGS "${3Delight_COMPILE_FLAGS}"
  )

target_link_libraries( psurf
  ${3Delight_LIBRARIES}
  )
