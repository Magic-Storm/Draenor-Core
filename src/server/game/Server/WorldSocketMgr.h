////////////////////////////////////////////////////////////////////////////////
//
// Project-Hellscream https://hellscream.org
// Copyright (C) 2018-2020 Project-Hellscream-6.2
// Discord https://discord.gg/CWCF3C9
//
////////////////////////////////////////////////////////////////////////////////

/** \addtogroup u2w User to World Communication
 *  @{
 *  \file WorldSocketMgr.h
 *  \author Derex <derex101@gmail.com>
 */

#ifndef CROSS
#ifndef __WORLDSOCKETMGR_H
#define __WORLDSOCKETMGR_H

#include "Common.h"

class WorldSocket;
class ReactorRunnable;
class ACE_Event_Handler;

/// Manages all sockets connected to peers and network threads
class WorldSocketMgr
{
private:
    friend class WorldSocket;
    WorldSocketMgr();
    virtual ~WorldSocketMgr();

public:
    static WorldSocketMgr* instance()
    {
        static WorldSocketMgr* instance = new WorldSocketMgr();
        return instance;
    }

    /// Start network, listen at address:port .
    int StartNetwork(ACE_UINT16 port, const char* address);

    /// Stops all network threads, It will wait for all running threads .
    void StopNetwork();

    /// Wait untill all network threads have "joined" .
    void Wait();

private:
    int OnSocketOpen(WorldSocket* sock);

    int StartReactiveIO(ACE_UINT16 port, const char* address);

    ReactorRunnable* m_NetThreads;
    size_t m_NetThreadsCount;

    int m_SockOutKBuff;
    int m_SockOutUBuff;
    bool m_UseNoDelay;

    class WorldSocketAcceptor* m_Acceptor;
};

#define sWorldSocketMgr WorldSocketMgr::instance()

#endif
/// @}
#endif
