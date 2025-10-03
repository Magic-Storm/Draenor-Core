////////////////////////////////////////////////////////////////////////////////
//
// Project-Hellscream https://hellscream.org
// Copyright (C) 2018-2020 Project-Hellscream-6.2
// Discord https://discord.gg/CWCF3C9
//
////////////////////////////////////////////////////////////////////////////////

/// \addtogroup Trinityd
/// @{
/// \file

#ifndef _RASOCKET_H
#define _RASOCKET_H

#include "Common.h"


/// Remote Administration socket
class RASocket : public ACE_Svc_Handler<ACE_SOCK_STREAM, ACE_MT_SYNCH>
{
    public:
        RASocket();
        virtual ~RASocket() { }

        virtual int svc() override;
        virtual int open(void* = 0) override;
        virtual int handle_close(ACE_HANDLE = ACE_INVALID_HANDLE, ACE_Reactor_Mask = ACE_Event_Handler::ALL_EVENTS_MASK) override;

    private:
        int recv_line(std::string& outLine);
        int recv_line(ACE_Message_Block& buffer);
        int process_command(const std::string& command);
        int authenticate();
        int subnegotiate();     /// Used by telnet protocol RFC 854 / 855
        int check_access_level(const std::string& user);
        int check_password(const std::string& user, const std::string& pass);
        int send(const std::string& line);

        static void zprint(void* callbackArg, const char* szText);
        static void commandFinished(void* callbackArg, bool success);

    private:
        uint8 _minLevel; ///< Minimum security level required to connect
        ACE_Atomic_Op<ACE_Thread_Mutex, bool> _commandExecuting;
};

#endif

/// @}
