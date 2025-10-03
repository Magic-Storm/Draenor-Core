////////////////////////////////////////////////////////////////////////////////
//
// Project-Hellscream https://hellscream.org
// Copyright (C) 2018-2020 Project-Hellscream-6.2
// Discord https://discord.gg/CWCF3C9
//
////////////////////////////////////////////////////////////////////////////////

#ifndef __ADDONHANDLER_H
#define __ADDONHANDLER_H

#include "Common.h"
#include "Config.h"
#include "WorldPacket.h"

class AddonHandler
{
    /* Construction */
    private:
        AddonHandler();

    public:
        ~AddonHandler();
        
        static AddonHandler* instance()
        {
            static AddonHandler* instance = new AddonHandler();
            return instance;
        }
                                                            //build addon packet
        bool BuildAddonPacket(WorldPacket* Source, WorldPacket* Target);
};
#define sAddOnHandler AddonHandler::instance()
#endif

