////////////////////////////////////////////////////////////////////////////////
//
// Project-Hellscream https://hellscream.org
// Copyright (C) 2018-2020 Project-Hellscream-6.2
// Discord https://discord.gg/CWCF3C9
//
////////////////////////////////////////////////////////////////////////////////

/// \addtogroup Trinityd Trinity Daemon
/// @{
/// \file

#include <openssl/opensslv.h>
#include <openssl/crypto.h>
#if defined(OPENSSL_VERSION_MAJOR) && (OPENSSL_VERSION_MAJOR >= 3)
#include <openssl/provider.h>
#endif

#include <boost/dll/runtime_symbol_info.hpp>
#include <ace/Version.h>

#include "Common.h"
#include "Database/DatabaseEnv.h"
#include "Configuration/Config.h"


#include "Log.h"
#include "Master.h"

#ifndef _TRINITY_CORE_CONFIG
# define _TRINITY_CORE_CONFIG  "worldserver.conf"
#endif

#ifdef _WIN32
#include "ServiceWin32.h"
char serviceName[] = "worldserver";
char serviceLongName[] = "Draenor-Core world service";
char serviceDescription[] = "Draenor-Coree World of Warcraft emulator world service";
/*
 * -1 - not in service mode
 *  0 - stopped
 *  1 - running
 *  2 - paused
 */
int m_ServiceStatus = -1;
#endif

/// libcurl need to be rebuild for VS2015 when we abandon multi support.

#if defined (_MSC_VER) && _MSC_VER >= 1900
#pragma comment (lib, "legacy_stdio_definitions.lib")

extern "C" { FILE __iob_func[3]{ *stdin, *stdout,  *stderr };  }
#endif

WorldDatabaseWorkerPool WorldDatabase;                      ///< Accessor to the world database
CharacterDatabaseWorkerPool CharacterDatabase;              ///< Accessor to the character database
LoginDatabaseWorkerPool LoginDatabase;                      ///< Accessor to the realm/login database
HotfixDatabaseWorkerPool HotfixDatabase;                    ///< Accessor to the hotfix database
LoginMopDatabaseWorkerPool LoginMopDatabase;                ///< Accessor to the mop login database
WebDatabaseWorkerPool WebDatabase;                          ///< Accessor to the web database


uint32 g_RealmID;                                             ///< Id of the realm

/// Print out the usage string for this program on the console.
void usage(const char* prog)
{
    printf("Usage:\n");
    printf(" %s [<options>]\n", prog);
    printf("    -c config_file           use config_file as configuration file\n");
#ifdef _WIN32
    printf("    Running as service functions:\n");
    printf("    --service                run as service\n");
    printf("    -s install               install service\n");
    printf("    -s uninstall             uninstall service\n");
#endif
}

/// Launch the Trinity server
extern int main(int argc, char** argv)
{
    ///- Command line parsing to get the configuration file name
    char const* cfg_file = _TRINITY_CORE_CONFIG;
    int c = 1;
    while (c < argc)
    {
        if (!strcmp(argv[c], "-c"))
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

        #ifdef _WIN32
        if (strcmp(argv[c], "-s") == 0) // Services
        {
            if (++c >= argc)
            {
                printf("Runtime-Error: -s option requires an input argument");
                usage(argv[0]);
                return 1;
            }

            if (strcmp(argv[c], "install") == 0)
            {
                if (WinServiceInstall())
                    printf("Installing service");
                return 1;
            }

            else if (strcmp(argv[c], "uninstall") == 0)
            {
                if (WinServiceUninstall())
                    printf("Uninstalling service");
                return 1;
            }
            else
            {
                printf("Runtime-Error: unsupported option %s", argv[c]);
                usage(argv[0]);
                return 1;
            }
        }
        if (strcmp(argv[c], "--service") == 0)
            WinServiceRun();
        #endif
        ++c;
    }

    if (!sConfigMgr->LoadInitial(cfg_file))
    {
        printf("Invalid or missing configuration file : %s", cfg_file);
        printf("Verify that the file exists and has \'[worldserver]' written in the top of the file!");
        return 1;
    }
    TC_LOG_INFO("server.worldserver", "Using configuration file %s.", cfg_file);

    TC_LOG_INFO("server.worldserver", "Using SSL version: %s (library: %s)", OPENSSL_VERSION_TEXT, OpenSSL_version(OPENSSL_VERSION));
    TC_LOG_INFO("server.worldserver", "Using ACE version: %s", ACE_VERSION);

    ///- and run the 'Master'
    /// \todo Why do we need this 'Master'? Can't all of this be in the Main as for Realmd?
    int ret = sMaster->Run();

    // at sMaster return function exist with codes
    // 0 - normal shutdown
    // 1 - shutdown at error
    // 2 - restart command used, this code can be used by restarter for restart Trinityd

    return ret;
}

/// @}
