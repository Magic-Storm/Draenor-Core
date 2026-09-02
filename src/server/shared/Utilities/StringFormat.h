#ifndef TRINITYCORE_STRING_FORMAT_H
#define TRINITYCORE_STRING_FORMAT_H

#include <cstdio>
#include <string>
#include <utility>

namespace Trinity
{
    template<typename... Args>
    inline std::string StringFormat(char const* fmt, Args&&... args)
    {
        char buf[2048];
        std::snprintf(buf, sizeof(buf), fmt, std::forward<Args>(args)...);
        return std::string(buf);
    }

    inline bool IsFormatEmptyOrNull(const char* fmt)
    {
        return fmt == nullptr;
    }

    inline bool IsFormatEmptyOrNull(std::string const& fmt)
    {
        return fmt.empty();
    }
}

#endif
