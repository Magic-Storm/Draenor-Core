/*
* Copyright (C) 2008-2020 TrinityCore <http://www.trinitycore.org/>
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

#include <thread>

#include <ace/Sig_Handler.h>

#include "Common.h"
#include "GitRevision.h"
#include "SignalHandler.h"
#include "World.h"
#include "WorldRunnable.h"
#ifndef CROSS
#include "WorldSocket.h"
#include "WorldSocketMgr.h"
#endif /* not CROSS */
#include "Configuration/Config.h"
#include "Database/DatabaseEnv.h"
#include "Database/DatabaseWorkerPool.h"
#ifndef CROSS
#include "PlayerDump.h"
#endif /* not CROSS */
#include "Player.h"
#include "ObjectMgr.h"
#include "CinematicPathMgr.h"

#include <openssl/crypto.h>
#include <openssl/opensslv.h>
#if defined(OPENSSL_VERSION_MAJOR) && (OPENSSL_VERSION_MAJOR >= 3)
#include <openssl/provider.h>
#endif
#include <boost/dll/runtime_symbol_info.hpp>

#include "CliRunnable.h"
#include "Log.h"
#include "Master.h"
#include "RARunnable.h"
#include "TCSoap.h"
#include "Timer.h"
#include "Util.h"

#include "OpenSSLCrypto.h"
#include "BigNumber.h"

#ifdef CROSS
#include "Cross/IRSocketMgr.h"

#endif /* CROSS */
#ifdef _WIN32
#include "ServiceWin32.h"
extern int m_ServiceStatus;
#endif

#ifdef __linux__
#include <sched.h>
#include <sys/resource.h>
#define PROCESS_HIGH_PRIORITY -15 // [-20, 19], default is 0
#endif

enum RealmFlags
{
    REALM_FLAG_NONE                              = 0x00,
    REALM_FLAG_INVALID                           = 0x01,
    REALM_FLAG_OFFLINE                           = 0x02,
    REALM_FLAG_SPECIFYBUILD                      = 0x04,
    REALM_FLAG_UNK1                              = 0x08,
    REALM_FLAG_UNK2                              = 0x10,
    REALM_FLAG_RECOMMENDED                       = 0x20,
    REALM_FLAG_NEW                               = 0x40,
    REALM_FLAG_FULL                              = 0x80
};

/// Handle worldservers's termination signals
class WorldServerSignalHandler : public Trinity::SignalHandler
{
    public:
        virtual void HandleSignal(int sigNum)
        {
            switch (sigNum)
            {
                case SIGINT:
                    World::StopNow(RESTART_EXIT_CODE);
                    break;
                case SIGTERM:
                #ifdef _WIN32
                case SIGBREAK:
                    if (m_ServiceStatus != 1)
                #endif /* _WIN32 */
                    World::StopNow(SHUTDOWN_EXIT_CODE);
                    break;
            }
        }
};

void FreezeDetectorThread(uint32 delayTime, uint32 fdpid)
{
    if (!delayTime)
        return;

    sLog->outInfo(LOG_FILTER_WORLDSERVER, "Starting up anti-freeze thread (%u seconds max stuck time)...", delayTime / 1000);
    uint32 loops = 0;
    uint32 lastChange = 0;

    while (!World::IsStopped())
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        uint32 curtime = getMSTime();
        // normal work
        uint32 worldLoopCounter = World::m_worldLoopCounter.value();
        if (loops != worldLoopCounter)
        {
            lastChange = curtime;
            loops = worldLoopCounter;
        }
#ifndef AMARU_DEBUG
        // possible freeze
        else if (getMSTimeDiff(lastChange, curtime) > delayTime)
        {
            TC_LOG_ERROR("server.worldserver", "World Thread hangs, kicking out server!");
            if (fdpid)
            {
                char buffer[20];
                sprintf(buffer, "kill -11 %s", fdpid);
                system(buffer);
            }
            else
                ASSERT(false);
        }
#endif
    }
    sLog->outInfo(LOG_FILTER_WORLDSERVER, "Anti-freeze thread exiting without problems.");
}

Master::Master()
{
}

Master::~Master()
{
}

/// Main function
int Master::Run()
{
    OpenSSLCrypto::threadsSetup(boost::dll::program_location().remove_filename());
    BigNumber seed1;
    seed1.SetRand(16 * 8);

    TC_LOG_INFO("server.worldserver", "%s (worldserver-daemon)", GitRevision::GetFullVersion());
    TC_LOG_INFO("server.worldserver", "<Ctrl-C> to stop.\n");
    TC_LOG_INFO("server.worldserver", "################################################### ");
    TC_LOG_INFO("server.worldserver", " ______                       __");
    TC_LOG_INFO("server.worldserver", "/\\__  _\\       __          __/\\ \\__");
    TC_LOG_INFO("server.worldserver", "\\/_/\\ \\/ _ __ /\\_\\    ___ /\\_\\ \\, _\\  __  __");
    TC_LOG_INFO("server.worldserver", "   \\ \\ \\/\\`'__\\/\\ \\ /' _ `\\/\\ \\ \\ \\/ /\\ \\/\\ \\");
    TC_LOG_INFO("server.worldserver", "    \\ \\ \\ \\ \\/ \\ \\ \\/\\ \\/\\ \\ \\ \\ \\ \\_\\ \\ \\_\\ \\");
    TC_LOG_INFO("server.worldserver", "     \\ \\_\\ \\_\\  \\ \\_\\ \\_\\ \\_\\ \\_\\ \\__\\\\/`____ \\");
    TC_LOG_INFO("server.worldserver", "      \\/_/\\/_/   \\/_/\\/_/\\/_/\\/_/\\/__/ `/___/> \\");
    TC_LOG_INFO("server.worldserver", "                                 C O R E  /\\___/");
    TC_LOG_INFO("server.worldserver", "http://TrinityCore.org                    \\/__/\n");

    /// worldserver PID file creation
    std::string pidFile = sConfigMgr->GetStringDefault("PidFile", "");
    if (!pidFile.empty())
    {
        if (uint32 pid = CreatePIDFile(pidFile))
            TC_LOG_INFO("server.worldserver", "Daemon PID: %u\n", pid);
        else
        {
            TC_LOG_ERROR("server.worldserver", "Cannot create PID file %s.\n", pidFile.c_str());
            return 1;
        }
    }

    ///- Start the databases
    if (!_StartDB())
        return 1;

    ExecutePendingRequests();

    // set server offline (not connectable)
    LoginDatabase.DirectPExecute("UPDATE realmlist SET flag = (flag & ~%u) | %u WHERE id = '%d'", REALM_FLAG_OFFLINE, REALM_FLAG_INVALID, g_RealmID);

    ///- Initialize the World
    sWorld->SetInitialWorldSettings();

    ///- Initialize the signal handlers
    WorldServerSignalHandler signalINT, signalTERM;
#ifdef _WIN32
    WorldServerSignalHandler signalBREAK;
#endif

    ///- Register worldserver's signal handlers
    ACE_Sig_Handler handle;
    handle.register_handler(SIGINT, &signalINT);
    handle.register_handler(SIGTERM, &signalTERM);
    #ifdef _WIN32
    handle.register_handler(SIGBREAK, &signalBREAK);
    #endif /* _WIN32 */

    ///- Launch WorldRunnable thread
    std::thread worldThread(WorldThread);

    std::thread* cliThread = NULL;

#ifdef _WIN32
    if (sConfigMgr->GetBoolDefault("Console.Enable", true) && (m_ServiceStatus == -1)/* need disable console in service mode*/)
#else
    if (sConfigMgr->GetBoolDefault("Console.Enable", true))
#endif
    {
        ///- Launch CliRunnable thread
        cliThread = new std::thread(CliThread);
    }

    std::thread rarThread(RemoteAccessThread);

#if defined(_WIN32) || defined(__linux__)

    ///- Handle affinity for multiple processors and process priority
    uint32 affinity = sConfigMgr->GetIntDefault("UseProcessors", 0);
    bool highPriority = sConfigMgr->GetBoolDefault("ProcessPriority", false);

#ifdef _WIN32 // Windows
    {
        HANDLE hProcess = GetCurrentProcess();

        if (affinity > 0)
        {
            ULONG_PTR appAff;
            ULONG_PTR sysAff;

            if (GetProcessAffinityMask(hProcess, &appAff, &sysAff))
            {
                ULONG_PTR currentAffinity = affinity & appAff; // remove non accessible processors

                if (!currentAffinity)
                    TC_LOG_ERROR("server.worldserver", "Processors marked in UseProcessors bitmask (hex) %x are not accessible for the worldserver. Accessible processors bitmask (hex): %x", affinity, appAff);
                else if (SetProcessAffinityMask(hProcess, currentAffinity))
                    TC_LOG_INFO("server.worldserver", "Using processors (bitmask, hex): %x", currentAffinity);
                else
                    TC_LOG_ERROR("server.worldserver", "Can't set used processors (hex): %x", currentAffinity);
            }
        }

        if (highPriority)
        {
            if (SetPriorityClass(hProcess, HIGH_PRIORITY_CLASS))
                TC_LOG_INFO("server.worldserver", "worldserver process priority class set to HIGH");
            else
                TC_LOG_ERROR("server.worldserver", "Can't set worldserver process priority class.");
        }
    }
#elif __linux__ // Linux

    if (affinity > 0)
    {
        cpu_set_t mask;
        CPU_ZERO(&mask);

        for (unsigned int i = 0; i < sizeof(affinity) * 8; ++i)
            if (affinity & (1 << i))
                CPU_SET(i, &mask);

        if (sched_setaffinity(0, sizeof(mask), &mask))
            TC_LOG_ERROR("server.worldserver", "Can't set used processors (hex): %x, error: %s", affinity, strerror(errno));
        else
        {
            CPU_ZERO(&mask);
            sched_getaffinity(0, sizeof(mask), &mask);
            TC_LOG_INFO("server.worldserver", "Using processors (bitmask, hex): %x", *(uint32*)(&mask));
        }
    }

    if (highPriority)
    {
        if (setpriority(PRIO_PROCESS, 0, PROCESS_HIGH_PRIORITY))
            TC_LOG_ERROR("server.worldserver", "Can't set worldserver process priority class, error: %s", strerror(errno));
        else
            TC_LOG_INFO("server.worldserver", "worldserver process priority class set to %i", getpriority(PRIO_PROCESS, 0));
    }

#endif

    //Start soap serving thread
    std::thread* soapThread = nullptr;

    if (sConfigMgr->GetBoolDefault("SOAP.Enabled", false))
    {
        soapThread = new std::thread(TCSoapThread, sConfigMgr->GetStringDefault("SOAP.IP", "127.0.0.1"), uint16(sConfigMgr->GetIntDefault("SOAP.Port", 7878)));
    }

    std::thread* freezeDetectorThread = nullptr;

    ///- Start up freeze catcher thread
    if (uint32 freezeDelay = sConfigMgr->GetIntDefault("MaxCoreStuckTime", 0))
    {
        freezeDetectorThread = new std::thread(FreezeDetectorThread, freezeDelay, pid);
    }

#ifndef CROSS
    ///- Launch the world listener socket
    uint16 worldPort = uint16(sWorld->getIntConfig(CONFIG_PORT_WORLD));
    std::string bindIp = sConfigMgr->GetStringDefault("BindIP", "0.0.0.0");
#else /* CROSS */
    uint16 worldPort = ConfigMgr::GetIntDefault("InterRealmServer.Port", 12345);
#endif /* CROSS */

#ifndef CROSS
    if (sWorldSocketMgr->StartNetwork(worldPort, bindIp.c_str()) == -1)
#else /* CROSS */
    if (sIRSocketMgr->StartNetwork(worldPort) == -1)
#endif /* CROSS */
    {
#ifndef CROSS
        TC_LOG_ERROR("server.worldserver", "Failed to start network");
#else /* CROSS */
        TC_LOG_ERROR("interrealm", "Failed to start network");
#endif /* CROSS */
        World::StopNow(ERROR_EXIT_CODE);
        // go down and shutdown the server
    }
#ifndef CROSS

    // set server online (allow connecting now)
    LoginDatabase.DirectPExecute("UPDATE realmlist SET flag = flag & ~%u, population = 0 WHERE id = '%u'", REALM_FLAG_INVALID, g_RealmID);

    InterRealmSession* irt = new InterRealmSession();
    ACE_Based::Thread interrealm_thread(irt, "InterRealm");
    interrealm_thread.setPriority(ACE_Based::Highest);
    sWorld->SetInterRealmSession(irt);
#endif /* not CROSS */

    TC_LOG_INFO("server.worldserver", "%s (worldserver-daemon) ready...", GitRevision::GetFullVersion());

    // when the main thread closes the singletons get unloaded
    // since worldrunnable uses them, it will crash if unloaded after master
    worldThread.join();
    //rarThread.join();

    if (soapThread)
    {
        soapThread->join();
        delete soapThread;
    }

    // set server offline
    LoginDatabase.DirectPExecute("UPDATE realmlist SET flag = flag | %u WHERE id = '%d'", REALM_FLAG_OFFLINE, g_RealmID);

    ///- Clean database before leaving
    ClearOnlineAccounts();

    _StopDB();

    TC_LOG_INFO("server.worldserver", "Halting process...");

    if (cliThread != nullptr)
    {
        #ifdef _WIN32

        // this only way to terminate CLI thread exist at Win32 (alt. way exist only in Windows Vista API)
        //_exit(1);
        // send keyboard input to safely unblock the CLI thread
        INPUT_RECORD b[4];
        HANDLE hStdIn = GetStdHandle(STD_INPUT_HANDLE);
        b[0].EventType = KEY_EVENT;
        b[0].Event.KeyEvent.bKeyDown = TRUE;
        b[0].Event.KeyEvent.uChar.AsciiChar = 'X';
        b[0].Event.KeyEvent.wVirtualKeyCode = 'X';
        b[0].Event.KeyEvent.wRepeatCount = 1;

        b[1].EventType = KEY_EVENT;
        b[1].Event.KeyEvent.bKeyDown = FALSE;
        b[1].Event.KeyEvent.uChar.AsciiChar = 'X';
        b[1].Event.KeyEvent.wVirtualKeyCode = 'X';
        b[1].Event.KeyEvent.wRepeatCount = 1;

        b[2].EventType = KEY_EVENT;
        b[2].Event.KeyEvent.bKeyDown = TRUE;
        b[2].Event.KeyEvent.dwControlKeyState = 0;
        b[2].Event.KeyEvent.uChar.AsciiChar = '\r';
        b[2].Event.KeyEvent.wVirtualKeyCode = VK_RETURN;
        b[2].Event.KeyEvent.wRepeatCount = 1;
        b[2].Event.KeyEvent.wVirtualScanCode = 0x1c;

        b[3].EventType = KEY_EVENT;
        b[3].Event.KeyEvent.bKeyDown = FALSE;
        b[3].Event.KeyEvent.dwControlKeyState = 0;
        b[3].Event.KeyEvent.uChar.AsciiChar = '\r';
        b[3].Event.KeyEvent.wVirtualKeyCode = VK_RETURN;
        b[3].Event.KeyEvent.wVirtualScanCode = 0x1c;
        b[3].Event.KeyEvent.wRepeatCount = 1;
        DWORD numb;
        WriteConsoleInput(hStdIn, b, 4, &numb);

        cliThread->join();

        #endif

        delete cliThread;
    }

    delete freezeDetectorThread;

    // for some unknown reason, unloading scripts here and not in worldrunnable
    // fixes a memory leak related to detaching threads from the module
    //UnloadScriptingModule();

    OpenSSLCrypto::threadsCleanup();

    // Exit the process with specified return value
    return World::GetExitCode();
}

/// Initialize connection to the databases
bool Master::_StartDB()
{
    MySQL::Library_Init();

    std::string dbString;
    uint8 asyncThreads, synchThreads;

    dbString = sConfigMgr->GetStringDefault("WorldDatabaseInfo", "");
    if (dbString.empty())
    {
        TC_LOG_ERROR("server.worldserver", "World database not specified in configuration file");
        return false;
    }

    asyncThreads = uint8(sConfigMgr->GetIntDefault("WorldDatabase.WorkerThreads", 1));
    if (asyncThreads < 1 || asyncThreads > 32)
    {
        TC_LOG_ERROR("server.worldserver", "World database: invalid number of worker threads specified. "
            "Please pick a value between 1 and 32.");
        return false;
    }

    synchThreads = uint8(sConfigMgr->GetIntDefault("WorldDatabase.SynchThreads", 1));
    ///- Initialize the world database
    if (!WorldDatabase.Open(dbString, asyncThreads, synchThreads))
    {
        TC_LOG_ERROR("server.worldserver", "Cannot connect to world database %s", dbString.c_str());
        return false;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    ///- Get character database info from configuration file
    dbString = sConfigMgr->GetStringDefault("CharacterDatabaseInfo", "");
    if (dbString.empty())
    {
        TC_LOG_ERROR("server.worldserver", "Character database not specified in configuration file");
        return false;
    }

    asyncThreads = uint8(sConfigMgr->GetIntDefault("CharacterDatabase.WorkerThreads", 1));
    if (asyncThreads < 1 || asyncThreads > 32)
    {
        TC_LOG_ERROR("server.worldserver", "Character database: invalid number of worker threads specified. "
            "Please pick a value between 1 and 32.");
        return false;
    }

    synchThreads = uint8(sConfigMgr->GetIntDefault("CharacterDatabase.SynchThreads", 2));

    ///- Initialize the Character database
    if (!CharacterDatabase.Open(dbString, asyncThreads, synchThreads))
    {
        TC_LOG_ERROR("server.worldserver", "Cannot connect to Character database %s", dbString.c_str());
        return false;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    ///- Get login database info from configuration file
    dbString = sConfigMgr->GetStringDefault("LoginDatabaseInfo", "");
    if (dbString.empty())
    {
        TC_LOG_ERROR("server.worldserver", "Login database not specified in configuration file");
        return false;
    }

    asyncThreads = uint8(sConfigMgr->GetIntDefault("LoginDatabase.WorkerThreads", 1));
    if (asyncThreads < 1 || asyncThreads > 32)
    {
        TC_LOG_ERROR("server.worldserver", "Login database: invalid number of worker threads specified. "
            "Please pick a value between 1 and 32.");
        return false;
    }

    synchThreads = uint8(sConfigMgr->GetIntDefault("LoginDatabase.SynchThreads", 1));
    ///- Initialize the login database
    if (!LoginDatabase.Open(dbString, asyncThreads, synchThreads))
    {
        TC_LOG_ERROR("server.worldserver", "Cannot connect to login database %s", dbString.c_str());
        return false;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    if (sConfigMgr->GetBoolDefault("MopTransfer.enable", false))
    {
        ///- Get MoP login database info from configuration file
        dbString = sConfigMgr->GetStringDefault("LoginMoPDatabaseInfo", "");
        if (dbString.empty())
        {
            TC_LOG_ERROR("server.worldserver", "Login mop database not specified in configuration file");
            return false;
        }

        asyncThreads = uint8(sConfigMgr->GetIntDefault("LoginMoPDatabaseInfo.WorkerThreads", 1));
        if (asyncThreads < 1 || asyncThreads > 32)
        {
            TC_LOG_ERROR("server.worldserver", "Login mop database: invalid number of worker threads specified. "
                "Please pick a value between 1 and 32.");
            return false;
        }

        asyncThreads = uint8(sConfigMgr->GetIntDefault("LoginMoPDatabaseInfo.WorkerThreads", 1));
        ///- Initialize the login database
        if (!LoginMopDatabase.Open(dbString, asyncThreads, synchThreads))
        {
            TC_LOG_ERROR("server.worldserver", "Cannot connect to login mop database %s", dbString.c_str());
            return false;
        }
    }

    if (sConfigMgr->GetBoolDefault("WebDatabase.enable", false))
    {
        dbString = sConfigMgr->GetStringDefault("WebDatabaseInfo", "");
        if (dbString.empty())
        {
            TC_LOG_ERROR("server.worldserver", "Web database not specified in configuration file");
            return false;
        }

        asyncThreads = uint8(sConfigMgr->GetIntDefault("WebDatabaseInfo.WorkerThreads", 1));
        if (asyncThreads < 1 || asyncThreads > 32)
        {
            TC_LOG_ERROR("server.worldserver", "Web database: invalid number of worker threads specified. "
                "Please pick a value between 1 and 32.");
            return false;
        }

        synchThreads = uint8(sConfigMgr->GetIntDefault("WebDatabaseInfo.SynchThreads", 1));
        ///- Initialize the login database
        if (!WebDatabase.Open(dbString, asyncThreads, synchThreads))
        {
            TC_LOG_ERROR("server.worldserver", "Cannot connect to web database %s", dbString.c_str());
            return false;
        }
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    dbString = sConfigMgr->GetStringDefault("HotfixDatabaseInfo", "");
    if (dbString.empty())
    {
        TC_LOG_ERROR("server.worldserver", "Hotfix database not specified in configuration file");
        return false;
    }

    asyncThreads = uint8(sConfigMgr->GetIntDefault("HotfixDatabase.WorkerThreads", 1));
    if (asyncThreads < 1 || asyncThreads > 32)
    {
        TC_LOG_ERROR("server.worldserver", "Hotfix database: invalid number of worker threads specified. "
                       "Please pick a value between 1 and 32.");
        return false;
    }

    synchThreads = uint8(sConfigMgr->GetIntDefault("HotfixDatabase.SynchThreads", 1));

    ///- Initialize the hotfix database
    if (!HotfixDatabase.Open(dbString, asyncThreads, synchThreads))
    {
        TC_LOG_ERROR("server.worldserver", "Cannot connect to Hotfix database %s", dbString.c_str());
        return false;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    /// Get the realm Id from the configuration file
    g_RealmID = sConfigMgr->GetIntDefault("RealmID", 0);
    if (!g_RealmID)
    {
        TC_LOG_ERROR("server.worldserver", "Realm ID not defined in configuration file");
        return false;
    }
    TC_LOG_INFO("server.worldserver", "Realm running as realm ID %d", g_RealmID);

    /// Clean the database before starting
    ClearOnlineAccounts();

    /// Insert version info into DB
    WorldDatabase.PExecute("UPDATE version SET core_version = '%s', core_revision = '%s'", GitRevision::GetFullVersion(), GitRevision::GetHash());  ///< One-time query

    sWorld->LoadDBVersion();

    TC_LOG_INFO("server.worldserver", "Using World DB: %s", sWorld->GetDBVersion());
    return true;
}

void Master::_StopDB()
{
    CharacterDatabase.Close();
    WorldDatabase.Close();
    LoginDatabase.Close();

    MySQL::Library_End();
}

/// Clear 'online' status for all accounts with characters in this realm
void Master::ClearOnlineAccounts()
{
    // Reset online status for all accounts with characters on the current realm
    LoginDatabase.DirectPExecute("UPDATE account SET online = 0 WHERE online > 0 AND id IN (SELECT acctid FROM realmcharacters WHERE realmid = %d)", g_RealmID);

    // Reset online status for all characters
    CharacterDatabase.DirectExecute("UPDATE characters SET online = 0 WHERE online <> 0");

    // Battleground instance ids reset at server restart
    CharacterDatabase.DirectExecute("UPDATE character_battleground_data SET instanceId = 0");
}

/// Execute pending SQL requests
#define PENDING_SQL_FILENAME "PendingSQL_Characters.sql"

void Master::ExecutePendingRequests()
{
    if (FILE* l_PendingRequestsFile = fopen(PENDING_SQL_FILENAME, "rb"))
    {
        fseek(l_PendingRequestsFile, 0, SEEK_END);
        long l_Size = ftell(l_PendingRequestsFile);
        rewind(l_PendingRequestsFile);

        if (l_Size > 0)
        {
            std::vector<char> l_Content(l_Size + 1);
            fread(l_Content.data(), l_Size, 1, l_PendingRequestsFile);
            l_Content[l_Size] = 0;

            CharacterDatabase.DirectExecute(l_Content.data());
            TC_LOG_INFO("server.worldserver", PENDING_SQL_FILENAME " has been executed with success"); ///< Because if the above failed, the core would crash
        }
        else
            TC_LOG_INFO("server.worldserver", PENDING_SQL_FILENAME " is empty, ignoring.");

        fclose(l_PendingRequestsFile);

        /// Clear file
        if ((l_PendingRequestsFile = fopen(PENDING_SQL_FILENAME, "w"))) ///< Using the result of an assignment as a condition without parentheses
            fclose(l_PendingRequestsFile);
    }
    else
        TC_LOG_INFO("server.worldserver", "Unable to open " PENDING_SQL_FILENAME ", ignoring.");
}
