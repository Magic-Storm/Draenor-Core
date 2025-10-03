/*
 * Copyright (C) 2008-2012 TrinityCore <http://www.trinitycore.org/>
 * Copyright (C) 2005-2009 MaNGOS <http://getmangos.com/>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

/**
* @file main.cpp
* @brief Authentication Server main program
*
* This file contains the main program for the
* authentication server
*/

#include <cstdlib>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <iostream>
#include <openssl/opensslv.h>
#include <openssl/crypto.h>

#include "AsyncAcceptor.h"
#include "AuthSession.h"
#include "Common.h"
#include "Configuration/Config.h"
#include "Database/DatabaseEnv.h"
#include "Log.h"
#include "ProcessPriority.h"
#include "RealmList.h"
#include "SystemConfig.h"
#include "Util.h"


#ifndef _TRINITY_REALM_CONFIG
# define _TRINITY_REALM_CONFIG  "authserver.conf"
#endif

using boost::asio::ip::tcp;

bool StartDB();
void StopDB();
void SignalHandler(const boost::system::error_code& error, int signalNumber);
void KeepDatabaseAliveHandler(const boost::system::error_code& error);
void usage(const char* prog);

boost::asio::io_context _ioService;
boost::asio::deadline_timer _dbPingTimer(_ioService);
uint32 _dbPingInterval;
LoginDatabaseWorkerPool LoginDatabase;

/// Launch the auth server
int main(int argc, char** argv)
{
    // Command line parsing to get the configuration file name
    char const* cfg_file = _TRINITY_REALM_CONFIG;
    int c = 1;
    while (c < argc)
    {
        if (strcmp(argv[c], "-c") == 0)
        {
            if (++c >= argc)
            {
                printf("Runtime-Error: -c option requires an input argument");
                usage(argv[0]);
                return 1;
            }
            else
                cfg_file = argv[c];
        }
        ++c;
    }

    std::string error;
    if (!sConfigMgr->LoadInitial(cfg_file, error))
    {
        printf("Invalid or missing configuration file : %s", cfg_file);
        printf("Verify that the file exists and has \'[authserver]\' written in the top of the file!");
        return 1;
    }

    TC_LOG_INFO("authserver", "%s (authserver)", _FULLVERSION);
    TC_LOG_INFO("authserver", "<Ctrl-C> to stop.\n");
    TC_LOG_INFO("authserver", "Using configuration file %s.", cfg_file);
    TC_LOG_WARN("authserver", "%s (Library: %s)", OPENSSL_VERSION_TEXT, SSLeay_version(SSLEAY_VERSION));

    // authserver PID file creation
    std::string pidfile = sConfigMgr->GetStringDefault("PidFile", "");
    if (!pidfile.empty())
    {
        uint32 pid = CreatePIDFile(pidfile);
        if (!pid)
        {
            TC_LOG_ERROR("authserver", "Cannot create PID file %s.\n", pidfile.c_str());
            return 1;
        }
        TC_LOG_INFO("authserver", "Daemon PID: %u\n", pid);
    }

    // Initialize the database connection
    if (!StartDB())
        return 1;

    sLog->SetRealmId(0);                                               // ensure we've set realm to 0 (authserver realmid)

    // Get the list of realms for the server
    sRealmList.Initialize(sConfigMgr->GetIntDefault("RealmsStateUpdateDelay", 20));
    if (sRealmList.size() == 0)
    {
        TC_LOG_ERROR("authserver", "No valid realms specified.");
        return 1;
    }

    // Start the listening port (acceptor) for auth connections
    int32 port = sConfigMgr->GetIntDefault("RealmServerPort", 3724);
    if (port < 0 || port > 0xFFFF)
    {
        TC_LOG_ERROR("authserver", "Specified port out of allowed range (1-65535)");
        return 1;
    }

    std::string bindIp = sConfigMgr->GetStringDefault("BindIP", "0.0.0.0");

    AsyncAcceptor<AuthSession> authServer(_ioService, bindIp, port);

    // Set signal handlers
    boost::asio::signal_set signals(_ioService, SIGINT, SIGTERM);
    signals.async_wait(SignalHandler);

    // Set process priority according to configuration settings
    SetProcessPriority("server.authserver");

    // Enabled a timed callback for handling the database keep alive ping
    _dbPingInterval = sConfigMgr->GetIntDefault("MaxPingTime", 30);
    _dbPingTimer.expires_from_now(boost::posix_time::seconds(_dbPingInterval));
    _dbPingTimer.async_wait(KeepDatabaseAliveHandler);

    // Start the io service worker loop
    _ioService.run();

    // Close the Database Pool and library
    StopDB();

    TC_LOG_INFO("authserver", "Halting process...");
    sLog->SetRealmId(0);
    return 0;
}

/// Initialize connection to the database
bool StartDB()
{
    MySQL::Library_Init();

    std::string dbstring = sConfigMgr->GetStringDefault("LoginDatabaseInfo", "");
    if (dbstring.empty())
    {
        TC_LOG_ERROR("authserver", "Database not specified");
        return false;
    }

    int32 worker_threads = sConfigMgr->GetIntDefault("LoginDatabase.WorkerThreads", 1);
    if (worker_threads < 1 || worker_threads > 32)
    {
        TC_LOG_ERROR("authserver", "Improper value specified for LoginDatabase.WorkerThreads, defaulting to 1.");
        worker_threads = 1;
    }

    int32 synch_threads = sConfigMgr->GetIntDefault("LoginDatabase.SynchThreads", 1);
    if (synch_threads < 1 || synch_threads > 32)
    {
        TC_LOG_ERROR("authserver", "Improper value specified for LoginDatabase.SynchThreads, defaulting to 1.");
        synch_threads = 1;
    }

    // NOTE: While authserver is singlethreaded you should keep synch_threads == 1. Increasing it is just silly since only 1 will be used ever.
    if (!LoginDatabase.Open(dbstring.c_str(), uint8(worker_threads), uint8(synch_threads)))
    {
        TC_LOG_ERROR("authserver", "Cannot connect to database");
        return false;
    }

    TC_LOG_INFO("authserver", "Started auth database connection pool.");
    return true;
}

void StopDB()
{
    LoginDatabase.Close();
    MySQL::Library_End();
}

void SignalHandler(const boost::system::error_code& error, int signalNumber)
{
    if (!error)
    {
        switch (signalNumber)
        {
        case SIGINT:
        case SIGTERM:
            _ioService.stop();
            break;
        }
    }
}

void KeepDatabaseAliveHandler(const boost::system::error_code& error)
{
    if (!error)
    {
        TC_LOG_INFO("server.authserver", "Ping MySQL to keep connection alive");
        LoginDatabase.KeepAlive();

        _dbPingTimer.expires_from_now(boost::posix_time::minutes(_dbPingInterval));
    }
}

/// Print out the usage string for this program on the console.
void usage(const char* prog)
{
    TC_LOG_INFO("server.authserver", "Usage: \n %s [<options>]\n"
        "    -c config_file           use config_file as configuration file\n\r",
        prog);
}
