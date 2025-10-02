////////////////////////////////////////////////////////////////////////////////
//
// Project-Hellscream https://hellscream.org
// Copyright (C) 2018-2020 Project-Hellscream-6.2
// Discord https://discord.gg/CWCF3C9
//
////////////////////////////////////////////////////////////////////////////////

/** \file
    \ingroup Trinityd
 */

#include "Common.h"
#include "Config.h"
#include "Log.h"
#include "RARunnable.h"
#include "World.h"

#include <ace/Reactor_Impl.h>
#include <ace/TP_Reactor.h>
#include <ace/Dev_Poll_Reactor.h>
#include <ace/Acceptor.h>
#include <ace/SOCK_Acceptor.h>

#include "RASocket.h"

void RemoteAccessThread()
{
    ACE_Reactor_Impl* imp = nullptr;

#if defined (ACE_HAS_EVENT_POLL) || defined (ACE_HAS_DEV_POLL)
    imp = new ACE_Dev_Poll_Reactor();
    imp->max_notify_iterations(128);
    imp->restart(1);
#else
    imp = new ACE_TP_Reactor();
    imp->max_notify_iterations(128);
#endif

    ACE_Reactor* reactor = new ACE_Reactor(imp, 1);

    if (!sConfigMgr->GetBoolDefault("Ra.Enable", false))
        return;

    ACE_Acceptor<RASocket, ACE_SOCK_ACCEPTOR> acceptor;

    uint16 raPort = uint16(sConfigMgr->GetIntDefault("Ra.Port", 3443));
    std::string stringIp = sConfigMgr->GetStringDefault("Ra.IP", "0.0.0.0");
    ACE_INET_Addr listenAddress(raPort, stringIp.c_str());

    if (acceptor.open(listenAddress, reactor) == -1)
    {
        TC_LOG_ERROR("server.worldserver", "Trinity RA can not bind to port %d on %s", raPort, stringIp.c_str());
        return;
    }

    TC_LOG_INFO("server.worldserver", "Starting Trinity RA on port %d on %s", raPort, stringIp.c_str());

    while (!World::IsStopped())
    {
        // don't be too smart to move this outside the loop
        // the run_reactor_event_loop will modify interval
        ACE_Time_Value interval(0, 100000);
        if (reactor->run_reactor_event_loop(interval) == -1)
            break;
    }

    delete imp;
    delete reactor;

    TC_LOG_DEBUG("server.worldserver", "Trinity RA thread exiting");
}
