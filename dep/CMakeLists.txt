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


if( MSVC )
  add_definitions(/W0)
else()
  add_definitions(-w)
endif()

#add_subdirectory(threads)

if(CMAKE_SYSTEM_NAME MATCHES "Linux")
  if(SERVERS AND NOT NOJEM)
    add_subdirectory(jemalloc)
#    add_subdirectory(valgrind)
  endif()
endif()

if(CMAKE_SYSTEM_NAME MATCHES "Windows")
  if(TOOLS)
    add_subdirectory(bzip2)
  endif()
  if(SERVERS OR TOOLS)
    add_subdirectory(zlib)
  endif()
endif()

if(SERVERS OR TOOLS)
  add_subdirectory(g3dlite)
  add_subdirectory(recastnavigation)
  add_subdirectory(boost)
  add_subdirectory(process)
  add_subdirectory(fmt)
endif()

if(SERVERS)
  add_subdirectory(gsoap)
  add_subdirectory(protobuf)
  add_subdirectory(rapidjson)
endif()

if(TOOLS)
  add_subdirectory(CascLib)
endif()
