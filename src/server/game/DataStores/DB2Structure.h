#ifndef DB2STRUCTURE_H
#define DB2STRUCTURE_H

#include "Common.h"

// Minimal stubs so TrinityCore 6.2.4 WorldPackets compile against Draenor DBC/DB2 stores.
struct GarrFollowerEntry { uint32 ID = 0; };
struct GarrMissionEntry { uint32 ID = 0; };
struct GarrBuildingEntry { uint32 ID = 0; };

#endif
