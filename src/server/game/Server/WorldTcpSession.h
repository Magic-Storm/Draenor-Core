#ifndef __WORLDTCPSESSION_H__
#define __WORLDTCPSESSION_H__

#include "WorldSocket.h"

/// Draenor historically used WorldTcpSession; 6.2.4.21742 uses TrinityCore WorldSocket (realm + instance).
using WorldTcpSession = WorldSocket;

#endif
