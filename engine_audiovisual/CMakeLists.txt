set(module_id vsxu_engine_audiovisual)

message("configuring            " ${module_id})


project (${module_id})

################################################################################
# CMAKE PACKAGES ###############################################################
################################################################################
find_package(OpenGL REQUIRED)

################################################################################
# INCLUDES #####################################################################
################################################################################

include_directories(
  ${OPENGL_INCLUDE_DIR}
  ${CMAKE_SOURCE_DIR}/
  ${CMAKE_SOURCE_DIR}/engine_audiovisual/include
  ${CMAKE_SOURCE_DIR}/engine_graphics/include
  ${CMAKE_SOURCE_DIR}/engine/include
)


################################################################################
# DEFINITIONS ##################################################################
################################################################################

add_definitions(
 -DVSXU_EXE
 -DVSX_AUDOVISUAL_DLL
 -DVSXU_DEVELOPER
 -DCMAKE_INSTALL_PREFIX="${CMAKE_INSTALL_PREFIX}"
)

if (VSXU_DEBUG EQUAL 1)
  add_definitions( -DVSXU_DEBUG -DDEBUG )
endif (VSXU_DEBUG EQUAL 1)

################################################################################
# SOURCES ######################################################################
################################################################################

set(SOURCES
  src/vsx_statelist.cpp
  src/vsx_manager.cpp
)

add_library(${module_id} SHARED ${SOURCES})
install(TARGETS ${module_id} DESTINATION ${VSXU_INSTALL_LIB_DIR} COMPONENT engine)

######################################################################################
# Install the Headers so as to install VSXu as a library
if (UNIX)
  file(GLOB HEADERS "include/*.h")
  install (FILES ${HEADERS} DESTINATION ${CMAKE_INSTALL_PREFIX}/${VSXU_INSTALL_INCLUDE_DIR}/engine_audiovisual COMPONENT Devel)
endif()
################################################################################

target_link_libraries(${module_id}
  ${OPENGL_LIBRARIES}
  vsxu_engine
  vsxu_engine_graphics
)

if (WIN32)
  target_link_libraries(${module_id}
    gdi32
  )
endif()
