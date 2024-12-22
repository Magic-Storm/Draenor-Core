# Copyright (C) 2008-2020 TrinityCore <http://www.trinitycore.org/>
# Copyright (C) 2005-2009 MaNGOS <http://getmangos.com/>
#
# This program is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by the
# Free Software Foundation; either version 2 of the License, or (at your
# option) any later version.
#
# This program is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
# FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
# more details.
#
# You should have received a copy of the GNU General Public License along
# with this program. If not, see <http://www.gnu.org/licenses/>.

# Set projectname (must be done AFTER setting configurationtypes)
add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

project(Trinity)

# CMake policies (can not be handled elsewhere)
 cmake_minimum_required(VERSION 3.20.4)
 cmake_policy(SET CMP0005 OLD)
 if(POLICY CMP0043)
   cmake_policy(SET CMP0043 OLD) # Disable 'Ignore COMPILE_DEFINITIONS_<Config> properties'
 endif(POLICY CMP0043)

# Set RPATH-handing (CMake parameters)
set(CMAKE_SKIP_BUILD_RPATH 0)
set(CMAKE_BUILD_WITH_INSTALL_RPATH 0)
if ( CMAKE_INSTALL_PREFIX MATCHES "/usr/local" )
  set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib/firestorm")
else()
  set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
endif()
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH 1)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# set macro-directory
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/macros")

# build in Release-mode by default if not explicitly set
if( NOT CMAKE_BUILD_TYPE )
  set(CMAKE_BUILD_TYPE "Release")
endif()

if( UNIX )
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
endif()

if ( WIN32 )
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DACE_HAS_POSIX_TIME -DACE_LACKS_TIMESPEC_T")
endif()

include(CheckCXXSourceRuns)
include(CheckIncludeFiles)

# set default buildoptions and print them
include(cmake/options.cmake)

# turn off PCH totally if enabled (hidden setting, mainly for devs)
if( NOPCH )
  set(USE_COREPCH 0)
  set(USE_SCRIPTPCH 0)
endif()

include(CheckPlatform)
include(GroupSources)

# basic packagesearching and setup (further support will be needed, this is a preliminary release!)
find_package(PCHSupport)
find_package(ACE REQUIRED)
find_package(OpenSSL REQUIRED)
# FIND_PACKAGE(msFramework REQUIRED)
find_package(MySQL REQUIRED)

if( UNIX )
  find_package(Readline)
  find_package(ZLIB)
  find_package(BZip2)
endif()

set(Boost_USE_STATIC_LIBS ON)
find_package(Boost COMPONENTS system locale filesystem thread regex serialization
    REQUIRED
)

if( NOT DISABLEGITCHECK )
  find_package(Git)
endif()

# Find revision ID and hash of the sourcetree
include(cmake/genrev.cmake)

# print out the results before continuing
include(cmake/showoptions.cmake)

# add dependencies
add_subdirectory(dep)

# add core sources
add_subdirectory(src)
