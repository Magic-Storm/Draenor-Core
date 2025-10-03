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

#ifndef _MASTER_H
#define _MASTER_H

#include <boost/asio/io_service.hpp>
#include "Common.h"
#include "RASession.h"

template<typename T>
class AsyncAcceptor;

/// Start the server
class Master
{
    private:
        Master();
        ~Master();

    public:
        static Master* instance()
        {
            static Master* instance = new Master();
            return instance;
        }
        
        int Run();

    private:
        bool _StartDB();
        void _StopDB();

        void ClearOnlineAccounts();
        AsyncAcceptor<RASession>* StartRaSocketAcceptor(boost::asio::io_service& ioService);
        void ExecutePendingRequests();
};

#define sMaster Master::instance()

#endif

/// @}
