# Copyright (C) 2008-2016 TrinityCore <http://www.trinitycore.org/>
#
# This file is free software; as a special exception the author gives
# unlimited permission to copy and/or distribute it, with or without
# modifications, as long as this notice is preserved.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY, to the extent permitted by law; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

########### bnetserver ###############

# Include directories
include_directories(
  ${CMAKE_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}/dep/g3dlite/include
  ${CMAKE_SOURCE_DIR}/src/server/shared
  ${CMAKE_SOURCE_DIR}/src/server/shared/Database
  ${CMAKE_SOURCE_DIR}/src/server/shared/Debugging
  ${CMAKE_SOURCE_DIR}/src/server/shared/Packets
  ${CMAKE_SOURCE_DIR}/src/server/shared/Cryptography
  ${CMAKE_SOURCE_DIR}/src/server/shared/Cryptography/Authentication
  ${CMAKE_SOURCE_DIR}/src/server/shared/Logging
  ${CMAKE_SOURCE_DIR}/src/server/shared/Threading
  ${CMAKE_SOURCE_DIR}/src/server/shared/Utilities
  ${CMAKE_SOURCE_DIR}/src/server/shared/Reporting
  ${CMAKE_SOURCE_DIR}/src/server/authserver/Realms
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${ACE_INCLUDE_DIR}
  ${MYSQL_INCLUDE_DIR}
  ${OPENSSL_INCLUDE_DIR}
  ${EASYJSON_INCLUDE_DIR}
  ${CURL_INCLUDE_DIR}
  ${MSFRAMEWORK_INCLUDE_DIR}
)

# Collect source files
file(GLOB_RECURSE PRIVATE_SOURCES
  "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/*.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp"
)

# Remove precompiled headers from sources
list(REMOVE_ITEM PRIVATE_SOURCES
  "${CMAKE_CURRENT_SOURCE_DIR}/PrecompiledHeaders/bnetPCH.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/PrecompiledHeaders/bnetPCH.h"
)

if (WIN32)
  list(APPEND PRIVATE_SOURCES ${sources_windows})
  if (MSVC)
    list(APPEND PRIVATE_SOURCES bnetserver.rc)
  endif()
endif()

if (USE_COREPCH)
  set(PRIVATE_PCH_HEADER PrecompiledHeaders/bnetPCH.h)
  set(PRIVATE_PCH_SOURCE PrecompiledHeaders/bnetPCH.cpp)
endif()

GroupSources(${CMAKE_CURRENT_SOURCE_DIR})

add_executable(bnetserver
  ${PRIVATE_SOURCES}
  ${PRIVATE_PCH_SOURCE}
)

if (NOT WIN32)
  set_target_properties(bnetserver PROPERTIES
    COMPILE_DEFINITIONS _TRINITY_BNET_CONFIG="${CONF_DIR}/bnetserver.conf"
  )
endif()

target_link_libraries(bnetserver
  PUBLIC
    shared)

# Collect include directories
file(GLOB_RECURSE PUBLIC_INCLUDES
  "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp"
)

# Remove precompiled headers from includes
list(REMOVE_ITEM PUBLIC_INCLUDES
  "${CMAKE_CURRENT_SOURCE_DIR}/PrecompiledHeaders/bnetPCH.h"
)

# Convert to directories
set(PUBLIC_INCLUDE_DIRS "")
foreach(INCLUDE_FILE ${PUBLIC_INCLUDES})
  get_filename_component(INCLUDE_DIR ${INCLUDE_FILE} DIRECTORY)
  list(APPEND PUBLIC_INCLUDE_DIRS ${INCLUDE_DIR})
endforeach()
list(REMOVE_DUPLICATES PUBLIC_INCLUDE_DIRS)

target_include_directories(bnetserver
  PUBLIC
    ${PUBLIC_INCLUDE_DIRS}
  PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR})

if (Boost_FOUND)
  include_directories(${Boost_INCLUDE_DIRS})
endif()

set_target_properties(bnetserver
    PROPERTIES
      FOLDER
        "server")

if (WIN32)
  if (MSVC)
    add_custom_command(TARGET bnetserver
      POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/bnetserver.conf.dist ${CMAKE_BINARY_DIR}/bin/$(ConfigurationName)/
      COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/bnetserver.cert.pem ${CMAKE_BINARY_DIR}/bin/$(ConfigurationName)/
      COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/bnetserver.key.pem ${CMAKE_BINARY_DIR}/bin/$(ConfigurationName)/
    )
  elseif (MINGW)
    add_custom_command(TARGET bnetserver
      POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/bnetserver.conf.dist ${CMAKE_BINARY_DIR}/bin/
      COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/bnetserver.cert.pem ${CMAKE_BINARY_DIR}/bin/
      COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/bnetserver.key.pem ${CMAKE_BINARY_DIR}/bin/
    )
  endif()
endif()

if (UNIX)
  install(TARGETS bnetserver DESTINATION bin)
  install(FILES bnetserver.conf.dist DESTINATION ${CONF_DIR})
  install(FILES bnetserver.cert.pem bnetserver.key.pem DESTINATION bin)
elseif (WIN32)
  install(TARGETS bnetserver DESTINATION "${CMAKE_INSTALL_PREFIX}")
  install(FILES bnetserver.conf.dist DESTINATION "${CMAKE_INSTALL_PREFIX}")
  install(FILES bnetserver.cert.pem bnetserver.key.pem DESTINATION "${CMAKE_INSTALL_PREFIX}")
endif()

# Generate precompiled header
if (USE_COREPCH)
  add_cxx_pch(bnetserver ${PRIVATE_PCH_HEADER} ${PRIVATE_PCH_SOURCE})
endif()
