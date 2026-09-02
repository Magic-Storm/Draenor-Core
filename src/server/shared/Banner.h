#ifndef TrinityCore_Banner_h__
#define TrinityCore_Banner_h__

#include "Define.h"
#include "GitRevision.h"
#include <cstdio>
#include <csignal>

namespace Trinity
{
    namespace Banner
    {
        inline void Show(char const* applicationName, void(*log)(char const* text), void(*logExtraInfo)())
        {
            char line[256];
            std::snprintf(line, sizeof(line), "%s (%s)", applicationName, GitRevision::GetFullVersion());
            if (log)
                log(line);
            else
                std::printf("%s\n", line);
            if (logExtraInfo)
                logExtraInfo();
        }
    }

    inline void AbortHandler(int) { }
}

#endif
