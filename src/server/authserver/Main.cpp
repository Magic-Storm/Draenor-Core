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

#include <ace/Dev_Poll_Reactor.h>
#include <ace/TP_Reactor.h>
#include <iostream>
#include <cstdlib>
#include <chrono>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <openssl/opensslv.h>
#include <openssl/crypto.h>
#include <thread>
#if defined(OPENSSL_VERSION_MAJOR) && (OPENSSL_VERSION_MAJOR >= 3)
#include <openssl/provider.h>
#endif

//#include <boost/dll/runtime_symbol_info.hpp>
//#include <Reporting/Reporter.hpp>

#include "Common.h"
#include "GitRevision.h"

#include "Database/DatabaseEnv.h"
#include "Configuration/Config.h"
#include "Log.h"
#include "Util.h"
#include "RealmList.h"
#include "AuthServer.h"

#include "Bnet2/WoWModules/PasswordAuth.hpp"
#include "Bnet2/WoWModules/RiskFingerprintAuth.hpp"
#include "Bnet2/WoWModules/ThumbprintAuth.hpp"
#include "Bnet2/WoWModules/SelectGameAccountAuth.hpp"
#include "Bnet2/Session.hpp"

#ifdef __linux__
#include <sched.h>
#include <sys/resource.h>
#define PROCESS_HIGH_PRIORITY -15 // [-20, 19], default is 0
#endif

#ifndef _TRINITY_REALM_CONFIG
# define _TRINITY_REALM_CONFIG  "authserver.conf"
#endif

bool StartDB();
void StopDB();
void SetProcessPriority();

boost::asio::io_context _ioContext;
boost::asio::steady_timer _dbPingTimer(_ioContext);
uint32 _dbPingInterval;

LoginDatabaseWorkerPool LoginDatabase;                      // Accessor to the authserver database

using boost::asio::ip::tcp;

// Handle authserver's termination signals
void SignalHandler(const boost::system::error_code& error, int signalNumber)
{
//TC_LOG_ERROR("server.authserver", "SIGNAL HANDLER WORKING");
if (!error)
{
    switch (signalNumber)
    {
    case SIGINT:
    case SIGTERM:
        _ioContext.stop();
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

        _dbPingTimer.expires_after(std::chrono::minutes(_dbPingInterval));
        _dbPingTimer.async_wait(KeepDatabaseAliveHandler);
    }
}

/// Print out the usage string for this program on the console.
void usage(const char* prog)
{
    TC_LOG_INFO("server.authserver", "Usage: \n %s [<options>]\n"
        "    -c config_file           use config_file as configuration file\n\r",
        prog);
}

void RegisterBNet2Components()
{
    BNet2::AuthComponentManager::GetSingleton()->Allow(37165, BNet2::BATTLENET2_PROGRAM_BNET, BNet2::BATTLENET2_PROGRAM_ALL, BNet2::BATTLENET2_LOCALE_NONE);

    QueryResult l_Result = LoginDatabase.PQuery("SELECT build, version FROM bnet_allowed_build");
    if (l_Result)
    {
        do
        {
            Field* l_Fields = l_Result->Fetch();
            uint32 l_AllowedBuild = l_Fields[0].GetUInt32();

            BNet2::g_VersionStrByBuild[l_AllowedBuild] = l_Fields[1].GetString();

            BNet2::AuthComponentManager::GetSingleton()->Allow(l_AllowedBuild, BNet2::BATTLENET2_PROGRAM_ALL_CLIENTS, BNet2::BATTLENET2_PLATFORM_ALL, BNet2::BATTLENET2_LOCALE_ALL);
        }
        while (l_Result->NextRow());
    }
}

void RegisterBNet2WoWModules()
{
    BNet2::ModuleManager::GetSingleton()->RegisterModule(BNet2::Module::Ptr(new BNet2::WoWModules::PasswordAuthWin32));
    BNet2::ModuleManager::GetSingleton()->RegisterModule(BNet2::Module::Ptr(new BNet2::WoWModules::PasswordAuthWin64));
    BNet2::ModuleManager::GetSingleton()->RegisterModule(BNet2::Module::Ptr(new BNet2::WoWModules::PasswordAuthMac64));

    BNet2::ModuleManager::GetSingleton()->RegisterModule(BNet2::Module::Ptr(new BNet2::WoWModules::RiskFingerprintAuthWin32));
    BNet2::ModuleManager::GetSingleton()->RegisterModule(BNet2::Module::Ptr(new BNet2::WoWModules::RiskFingerprintAuthWin64));
    BNet2::ModuleManager::GetSingleton()->RegisterModule(BNet2::Module::Ptr(new BNet2::WoWModules::RiskFingerprintAuthMac64));

    BNet2::ModuleManager::GetSingleton()->RegisterModule(BNet2::Module::Ptr(new BNet2::WoWModules::ThumbprintAuthWin32));
    BNet2::ModuleManager::GetSingleton()->RegisterModule(BNet2::Module::Ptr(new BNet2::WoWModules::ThumbprintAuthWin64));
    BNet2::ModuleManager::GetSingleton()->RegisterModule(BNet2::Module::Ptr(new BNet2::WoWModules::ThumbprintAuthMac64));

    BNet2::ModuleManager::GetSingleton()->RegisterModule(BNet2::Module::Ptr(new BNet2::WoWModules::SelectGameAccountAuthWin32));
    BNet2::ModuleManager::GetSingleton()->RegisterModule(BNet2::Module::Ptr(new BNet2::WoWModules::SelectGameAccountAuthWin64));
    BNet2::ModuleManager::GetSingleton()->RegisterModule(BNet2::Module::Ptr(new BNet2::WoWModules::SelectGameAccountAuthMac64));
}

/// Launch the auth server
int main(int argc, char** argv)
{
    ACE_Based::Thread::current()->setName("MainThread");

    // Command line parsing to get the configuration file name
    char const* configFile = _TRINITY_REALM_CONFIG;
    int count = 1;
    while (count < argc)
    {
        if (strcmp(argv[count], "-c") == 0)
        {
            if (++count >= argc)
            {
                printf("Runtime-Error: -c option requires an input argument");
                usage(argv[0]);
                return 1;
            }
            else
                configFile = argv[count];
        }
        ++count;
    }

    if (!sConfigMgr->LoadInitial(configFile))
    {
        printf("Invalid or missing configuration file : %s", configFile);
        printf("Verify that the file exists and has \'[authserver]\' written in the top of the file!");
        return 1;
    }

    TC_LOG_INFO("server.authserver", "%s (authserver)", GitRevision::GetFullVersion());
    TC_LOG_INFO("server.authserver", "<Ctrl-C> to stop.\n");
    TC_LOG_INFO("server.authserver", "Using configuration file %s.", configFile);

    TC_LOG_INFO("server.authserver", "%s (Library: %s)", OPENSSL_VERSION_TEXT, OpenSSL_version(OPENSSL_VERSION));

    // authserver PID file creation
    std::string pidFile = sConfigMgr->GetStringDefault("PidFile", "");
    if (!pidFile.empty())
    {
        uint32 pid = CreatePIDFile(pidFile);
        if (!pid)
        {
            TC_LOG_ERROR("server.authserver", "Cannot create PID file %s.\n", pidFile.c_str());
            return 1;
        }
        TC_LOG_INFO("server.authserver", "Daemon PID: %u\n", pid);
    }

    // Initialize the database connection
    if (!StartDB())
        return 1;

    RegisterBNet2Components();
    RegisterBNet2WoWModules();
	
    
    // Get the list of realms for the server
    sRealmList->Initialize(sConfigMgr->GetIntDefault("RealmsStateUpdateDelay", 20));
    if (sRealmList->size() == 0)
    {
        TC_LOG_ERROR("server.authserver", "No valid realms specified.");
        return 1;
    }

    int32 rmport = sConfigMgr->GetIntDefault("RealmServerPort", 3724);
    if (rmport < 0 || rmport > 0xFFFF)
    {
        TC_LOG_ERROR("server.authserver", "Specified port out of allowed range (1-65535)");
        return 1;
    }

    std::string bindIp = sConfigMgr->GetStringDefault("BindIP", "0.0.0.0");

    AuthServer authServer(_ioContext, bindIp, rmport);

    // Set signal handlers
    boost::asio::signal_set signals(_ioContext, SIGINT, SIGTERM);
    signals.async_wait(SignalHandler);

    SetProcessPriority();

    _dbPingInterval = sConfigMgr->GetIntDefault("MaxPingTime", 30);

    _dbPingTimer.expires_after(std::chrono::minutes(_dbPingInterval));
    _dbPingTimer.async_wait(KeepDatabaseAliveHandler);

    // Start the io context
    _ioContext.run();

    // Close the Database Pool and library
    StopDB();

    TC_LOG_INFO("server.authserver", "Halting process...");
    return 0;
}

/// Initialize connection to the database
bool StartDB()
{
    MySQL::Library_Init();

    std::string dbstring = sConfigMgr->GetStringDefault("LoginDatabaseInfo", "");
    if (dbstring.empty())
    {
        TC_LOG_ERROR("server.authserver", "Database not specified");
        return false;
    }

    int32 worker_threads = sConfigMgr->GetIntDefault("LoginDatabase.WorkerThreads", 1);
    if (worker_threads < 1 || worker_threads > 32)
    {
        TC_LOG_ERROR("server.authserver", "Improper value specified for LoginDatabase.WorkerThreads, defaulting to 1.");
        worker_threads = 1;
    }

    int32 synch_threads = sConfigMgr->GetIntDefault("LoginDatabase.SynchThreads", 1);
    if (synch_threads < 1 || synch_threads > 32)
    {
        TC_LOG_ERROR("server.authserver", "Improper value specified for LoginDatabase.SynchThreads, defaulting to 1.");
        synch_threads = 1;
    }

    // NOTE: While authserver is singlethreaded you should keep synch_threads == 1. Increasing it is just silly since only 1 will be used ever.
    if (!LoginDatabase.Open(dbstring.c_str(), uint8(worker_threads), uint8(synch_threads)))
    {
        TC_LOG_ERROR("server.authserver", "Cannot connect to database");
        return false;
    }

    TC_LOG_INFO("server.authserver", "Started auth database connection pool.");
    sLog->SetRealmId(0); // Enables DB appenders when realm is set.
    return true;
}

void StopDB()
{
    LoginDatabase.Close();
    MySQL::Library_End();
}

void SetProcessPriority()
{
#if defined(_WIN32) || defined(__linux__)

    ///- Handle affinity for multiple processors and process priority
    uint32 affinity = sConfigMgr->GetIntDefault("UseProcessors", 0);
    bool highPriority = sConfigMgr->GetBoolDefault("ProcessPriority", false);

#ifdef _WIN32 // Windows

    HANDLE hProcess = GetCurrentProcess();
    if (affinity > 0)
    {
        ULONG_PTR appAff;
        ULONG_PTR sysAff;

        if (GetProcessAffinityMask(hProcess, &appAff, &sysAff))
        {
            // remove non accessible processors
            ULONG_PTR currentAffinity = affinity & appAff;

            if (!currentAffinity)
                TC_LOG_ERROR("server.authserver", "Processors marked in UseProcessors bitmask (hex) %x are not accessible for the authserver. Accessible processors bitmask (hex): %x", affinity, appAff);
            else if (SetProcessAffinityMask(hProcess, currentAffinity))
                TC_LOG_INFO("server.authserver", "Using processors (bitmask, hex): %x", currentAffinity);
            else
                TC_LOG_ERROR("server.authserver", "Can't set used processors (hex): %x", currentAffinity);
        }
    }

    if (highPriority)
    {
        if (SetPriorityClass(hProcess, HIGH_PRIORITY_CLASS))
            TC_LOG_INFO("server.authserver", "authserver process priority class set to HIGH");
        else
            TC_LOG_ERROR("server.authserver", "Can't set authserver process priority class.");
    }

#else // Linux

    if (affinity > 0)
    {
        cpu_set_t mask;
        CPU_ZERO(&mask);

        for (unsigned int i = 0; i < sizeof(affinity) * 8; ++i)
            if (affinity & (1 << i))
                CPU_SET(i, &mask);

        if (sched_setaffinity(0, sizeof(mask), &mask))
            TC_LOG_ERROR("server.authserver", "Can't set used processors (hex): %x, error: %s", affinity, strerror(errno));
        else
        {
            CPU_ZERO(&mask);
            sched_getaffinity(0, sizeof(mask), &mask);
            TC_LOG_INFO("server.authserver", "Using processors (bitmask, hex): %lx", *(__cpu_mask*)(&mask));
        }
    }

    if (highPriority)
    {
        if (setpriority(PRIO_PROCESS, 0, PROCESS_HIGH_PRIORITY))
            TC_LOG_ERROR("server.authserver", "Can't set authserver process priority class, error: %s", strerror(errno));
        else
            TC_LOG_INFO("server.authserver", "authserver process priority class set to %i", getpriority(PRIO_PROCESS, 0));
    }

#endif
#endif
}
