cmake_minimum_required( VERSION 2.8 )
project( objtest )

#=====
# General
set( CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/../../config/cmake )

# try to find a rman lib (set by the RMAN envvar)
find_package( 3Delight REQUIRED )

include_directories(
  ${CMAKE_SOURCE_DIR}
  ${3Delight_INCLUDE_DIR}
  )

add_executable(objtest readobj.c
  )

set_target_properties( objtest
  PROPERTIES
  PREFIX ""
  OUTPUT_NAME "objtest"
  COMPILE_FLAGS "${3Delight_COMPILE_FLAGS}"
  )

target_link_libraries( objtest
  ${3Delight_LIBRARIES}
  )
