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

file(GLOB_RECURSE sources_rest REST/*.cpp REST/*.h)
file(GLOB_RECURSE sources_server Server/*.cpp Server/*.h)
file(GLOB_RECURSE sources_services Services/*.cpp Services/*.h)
file(GLOB sources_localdir *.cpp *.h)

if (USE_COREPCH)
  set(bnetserver_PCH_HDR PrecompiledHeaders/bnetPCH.h)
  set(bnetserver_PCH_SRC PrecompiledHeaders/bnetPCH.cpp)
endif()

set(bnetserver_SRCS
  ${sources_rest}
  ${sources_server}
  ${sources_services}
  ${sources_localdir}
)

if (WIN32)
  if (MSVC)
    list(APPEND bnetserver_SRCS bnetserver.rc)
  endif()
endif()

include_directories(
  ${CMAKE_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}/dep/gsoap
  ${CMAKE_SOURCE_DIR}/src/server/shared
  ${CMAKE_SOURCE_DIR}/src/server/shared/Database
  ${CMAKE_SOURCE_DIR}/src/server/shared/Debugging
  ${CMAKE_SOURCE_DIR}/src/server/shared/Packets
  ${CMAKE_SOURCE_DIR}/src/server/shared/Cryptography
  ${CMAKE_SOURCE_DIR}/src/server/shared/Cryptography/Authentication
  ${CMAKE_SOURCE_DIR}/src/server/shared/Logging
  ${CMAKE_SOURCE_DIR}/src/server/shared/Networking
  ${CMAKE_SOURCE_DIR}/src/server/shared/Threading
  ${CMAKE_SOURCE_DIR}/src/server/shared/Utilities
  ${CMAKE_SOURCE_DIR}/src/server/shared/Reporting
  ${CMAKE_SOURCE_DIR}/src/server/shared/JSON
  ${CMAKE_SOURCE_DIR}/src/server/shared/Realm
  ${CMAKE_SOURCE_DIR}/src/server/proto
  ${CMAKE_SOURCE_DIR}/src/server/proto/Client
  ${CMAKE_SOURCE_DIR}/src/server/proto/Login
  ${CMAKE_SOURCE_DIR}/src/server/proto/RealmList
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/REST
  ${CMAKE_CURRENT_SOURCE_DIR}/Server
  ${CMAKE_CURRENT_SOURCE_DIR}/Services
  ${ACE_INCLUDE_DIR}
  ${MYSQL_INCLUDE_DIR}
  ${OPENSSL_INCLUDE_DIR}
)

if (Boost_FOUND)
  include_directories(${Boost_INCLUDE_DIRS})
endif()

GroupSources(${CMAKE_CURRENT_SOURCE_DIR})

add_executable(bnetserver
  ${bnetserver_SRCS}
  ${bnetserver_PCH_SRC}
)
set_property(TARGET bnetserver PROPERTY FOLDER "server")

add_dependencies(bnetserver revision.h)

if (NOT WIN32)
  set_target_properties(bnetserver PROPERTIES
    COMPILE_DEFINITIONS _TRINITY_BNET_CONFIG="${CONF_DIR}/bnetserver.conf"
  )
endif()

target_link_libraries(bnetserver
  shared
  proto
  gsoap
  ${MYSQL_LIBRARY}
  ${OPENSSL_LIBRARIES}
  ${OPENSSL_EXTRA_LIBRARIES}
  ${ZLIB_LIBRARIES}
  ${Boost_LIBRARIES}
)

if (WIN32)
  if (MSVC)
    add_custom_command(TARGET bnetserver
      POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/bnetserver.conf.dist ${CMAKE_BINARY_DIR}/bin/$(ConfigurationName)/
      COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/bnetserver.cert.pem ${CMAKE_BINARY_DIR}/bin/$(ConfigurationName)/
      COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/bnetserver.key.pem ${CMAKE_BINARY_DIR}/bin/$(ConfigurationName)/
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

if (USE_COREPCH)
  add_cxx_pch(bnetserver ${bnetserver_PCH_HDR} ${bnetserver_PCH_SRC})
endif()
