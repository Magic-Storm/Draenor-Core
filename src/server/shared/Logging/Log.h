/*
* Copyright (C) 2008-2020 TrinityCore <http://www.trinitycore.org/>
* Copyright (C) 2005-2009 MaNGOS <http://getmangos.com/>
* Copyright (C) 2023 MagicStorm
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
#define _SILENCE_STDEXT_HASH_DEPRECATION_WARNINGS 1
#ifndef TRINITYCORE_LOG_H
#define TRINITYCORE_LOG_H

#include "Common.h"
#include "Define.h"
#include "Appender.h"
#include "Logger.h"
#include <stdarg.h>
#include "LogWorker.h"
#include "LogOperation.h"

#include <unordered_map>
#include <string>
#define LOGGER_ROOT "root"

#include <cstdarg>
#include <cstdio>

class Log
{
    typedef std::unordered_map<std::string, Logger> LoggerMap;

    private:
        Log();
        ~Log();

    public:
        static Log* instance()
        {
            static Log* instance = new Log();
            return instance;
        }

        void LoadFromConfig();
        void Close();
        bool ShouldLog(std::string const& type, LogLevel level) const;
        bool SetLogLevel(std::string const& name, char const* level, bool isLogger = true);

        void outMessage(std::string const& f, LogLevel level, char const* str, ...) ATTR_PRINTF(4, 5);
/*
        void outTrace(std::string const& f, char const* str, ...) ATTR_PRINTF(3, 4);
        void outDebug(std::string const& f, char const* str, ...) ATTR_PRINTF(3, 4);
        void outInfo(std::string const& f, char const* str, ...) ATTR_PRINTF(3, 4);
        void outWarn(std::string const& f, char const* str, ...) ATTR_PRINTF(3, 4);
        void outError(std::string const& f, char const* str, ...) ATTR_PRINTF(3, 4);
        void outFatal(std::string const& f, char const* str, ...) ATTR_PRINTF(3, 4);
*/
        void outCharDump(char const* str, uint32 account_id, uint32 guid, char const* name);

        /// No filters
        void outArena(const char * str, ...) ATTR_PRINTF(2, 3);
        void outCommand(uint32 gm_account_id, std::string gm_account_name,
            uint32 gm_character_id, std::string gm_character_name,
            uint32 sc_account_id, std::string sc_account_name,
            uint32 sc_character_id, std::string sc_character_name,
            const char* str, ...) ATTR_PRINTF(10, 11);
        void outGmChat(uint32 message_type,
                       uint32 from_account_id, std::string from_account_name,
                       uint32 from_character_id, std::string from_character_name,
                       uint32 to_account_id, std::string to_account_name,
                       uint32 to_character_id, std::string to_character_name,
                       const char * str);
        void outAshran(const char* str, ...) ATTR_PRINTF(2, 3);
        
        void SetRealmId(uint32 id);
        uint32 GetRealmID() const { return 1; } //hackfix, set realm as ID 1, need more work on howto identifi player's realm id

    private:
        static std::string GetTimestampStr();
        void vlog(std::string const& f, LogLevel level, char const* str, va_list argptr);
        void write(LogMessage* msg) const;

        Logger const* GetLoggerByType(std::string const& type) const;
        Appender* GetAppenderByName(std::string const& name);
        uint8 NextAppenderId();
        void CreateAppenderFromConfig(std::string const& name);
        void CreateLoggerFromConfig(std::string const& name);
        void ReadAppendersFromConfig();
        void ReadLoggersFromConfig();

        AppenderMap appenders;
        LoggerMap loggers;

        uint8 AppenderId;
        LogLevel lowestLogLevel;

        std::string m_logsDir;
        std::string m_logsTimestamp;

        LogWorker* worker;

        FILE* ashranLog;

        /// Slack
        bool        m_SlackEnable;
        std::string m_SlackApiUrl;
        std::string m_SlackAppName;
};

inline Logger const* Log::GetLoggerByType(std::string const& type) const
{
    LoggerMap::const_iterator it = loggers.find(type);
    if (it != loggers.end())
        return &(it->second);

    if (type == LOGGER_ROOT)
        return NULL;

    std::string parentLogger = LOGGER_ROOT;
    size_t found = type.find_last_of(".");
    if (found != std::string::npos)
        parentLogger = type.substr(0, found);

    return GetLoggerByType(parentLogger);
}

inline bool Log::ShouldLog(std::string const& type, LogLevel level) const
{
    // TODO: Use cache to store "Type.sub1.sub2": "Type" equivalence, should
    // Speed up in cases where requesting "Type.sub1.sub2" but only configured
    // Logger "Type"
    
    Logger const* logger = GetLoggerByType(type);
    if (!logger)
        return false;

    LogLevel logLevel = logger->getLogLevel();
    return logLevel != LOG_LEVEL_DISABLED && logLevel <= level;
}

inline void Log::outMessage(std::string const& filter, LogLevel level, const char* str, ...)
{
    va_list ap;
    va_start(ap, str);

    vlog(filter, level, str, ap);

    va_end(ap);
}

#define sLog Log::instance()

#if PLATFORM != PLATFORM_WINDOWS
#define TC_LOG_MESSAGE_BODY(filterType__, level__, ...)                 \
        do {                                                            \
            if (sLog->ShouldLog(filterType__, level__))                 \
                sLog->outMessage(filterType__, level__, __VA_ARGS__);   \
        } while (0)
#else
#define TC_LOG_MESSAGE_BODY(filterType__, level__, ...)                 \
        __pragma(warning(push))                                         \
        __pragma(warning(disable:4127))                                 \
        do {                                                            \
            if (sLog->ShouldLog(filterType__, level__))                 \
                sLog->outMessage(filterType__, level__, __VA_ARGS__);   \
        } while (0)                                                     \
        __pragma(warning(pop))
#endif

#define TC_LOG_TRACE(filterType__, ...) \
    TC_LOG_MESSAGE_BODY(filterType__, LOG_LEVEL_TRACE, __VA_ARGS__)

#define TC_LOG_DEBUG(filterType__, ...) \
    TC_LOG_MESSAGE_BODY(filterType__, LOG_LEVEL_DEBUG, __VA_ARGS__)

#define TC_LOG_INFO(filterType__, ...)  \
    TC_LOG_MESSAGE_BODY(filterType__, LOG_LEVEL_INFO, __VA_ARGS__)

#define TC_LOG_WARN(filterType__, ...)  \
    TC_LOG_MESSAGE_BODY(filterType__, LOG_LEVEL_WARN, __VA_ARGS__)

#define TC_LOG_ERROR(filterType__, ...) \
    TC_LOG_MESSAGE_BODY(filterType__, LOG_LEVEL_ERROR, __VA_ARGS__)

#define TC_LOG_FATAL(filterType__, ...) \
    TC_LOG_MESSAGE_BODY(filterType__, LOG_LEVEL_FATAL, __VA_ARGS__)

/*
// Returns default logger if the requested logger is not found
inline Logger* Log::GetLoggerByType(std::string const& filter)
{
    return m_LogTypePresentCache[filter] ? &loggers[filter] : &(loggers[0]);
}

inline bool Log::ShouldLog(std::string const& type, LogLevel level)
{
    // Don't even look for a logger if the LogLevel is lower than lowest log levels across all loggers
    if (level < lowestLogLevel)
        return false;

    if (m_LogTypePresentCache[type])
    {
        LogLevel loggerLevel = (LogLevel)m_LogLevelTypeByFilterCache[type];
        return  loggerLevel != LOG_LEVEL_DISABLED && loggerLevel <= level;
    }

    if (type != "misc")
        return ShouldLog("misc", level);

    return false;
}
*/

#endif
