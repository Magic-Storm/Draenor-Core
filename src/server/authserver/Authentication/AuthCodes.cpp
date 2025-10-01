/*
 * Copyright (C) 2008-2012 TrinityCore <http://www.trinitycore.org/>
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

#include "AuthCodes.h"

namespace AuthHelper
{
    bool IsAcceptedClientBuild(int build)
    {
        static int accepted_versions[] = JADECORE_ACCEPTED_CLIENT_BUILD;

        for (int i = 0; accepted_versions[i]; ++i)
            if (build == accepted_versions[i])
                return true;

        return false;
    }

    bool IsPreBCAcceptedClientBuild(int build)
    {
        // Pre-BC builds (1.12.x)
        return build >= 5875 && build <= 6180;
    }

    RealmBuildInfo const* GetBuildInfo(int build)
    {
        static RealmBuildInfo buildInfo = {0, 0, 0, 0};
        
        // Simple build info mapping
        if (build >= 17399) // MoP
        {
            buildInfo.MajorVersion = 5;
            buildInfo.MinorVersion = 4;
            buildInfo.BugfixVersion = 7;
            buildInfo.Build = build;
            return &buildInfo;
        }
        else if (build >= 16135) // Cata
        {
            buildInfo.MajorVersion = 4;
            buildInfo.MinorVersion = 3;
            buildInfo.BugfixVersion = 4;
            buildInfo.Build = build;
            return &buildInfo;
        }
        else if (build >= 12340) // WotLK
        {
            buildInfo.MajorVersion = 3;
            buildInfo.MinorVersion = 3;
            buildInfo.BugfixVersion = 5;
            buildInfo.Build = build;
            return &buildInfo;
        }
        
        return nullptr;
    }
};
