////////////////////////////////////////////////////////////////////////////////
//
// Project-Hellscream https://hellscream.org
// Copyright (C) 2018-2020 Project-Hellscream-6.2
// Discord https://discord.gg/CWCF3C9
//
////////////////////////////////////////////////////////////////////////////////

#include "ObjectMgr.h"
#include "Opcodes.h"
#include "WorldSession.h"
#include "WorldPacket.h"
#include "AuthenticationPackets.h"
#include "BattlenetRpcErrorCodes.h"
#include "Realm.h"
#include "World.h"

uint8 gExpansionPerRace[] =
{
    MAX_EXPANSION,                      ///< RACE_NONE
    EXPANSION_VANILLA,                  ///< RACE_HUMAN
    EXPANSION_VANILLA,                  ///< RACE_ORC
    EXPANSION_VANILLA,                  ///< RACE_DWARF
    EXPANSION_VANILLA,                  ///< RACE_NIGHTELF
    EXPANSION_VANILLA,                  ///< RACE_UNDEAD_PLAYER
    EXPANSION_VANILLA,                  ///< RACE_TAUREN
    EXPANSION_VANILLA,                  ///< RACE_GNOME
    EXPANSION_VANILLA,                  ///< RACE_TROLL
    EXPANSION_CATACLYSM,                ///< RACE_GOBLIN
	EXPANSION_VANILLA,                  ///< RACE_BLOODELF
	EXPANSION_VANILLA,                  ///< RACE_DRAENEI
    MAX_EXPANSION,                      ///< RACE_FEL_ORC
    MAX_EXPANSION,                      ///< RACE_NAGA
    MAX_EXPANSION,                      ///< RACE_BROKEN
    MAX_EXPANSION,                      ///< RACE_SKELETON
    MAX_EXPANSION,                      ///< RACE_VRYKUL
    MAX_EXPANSION,                      ///< RACE_TUSKARR
    MAX_EXPANSION,                      ///< RACE_FOREST_TROLL
    MAX_EXPANSION,                      ///< RACE_TAUNKA
    MAX_EXPANSION,                      ///< RACE_NORTHREND_SKELETON
    MAX_EXPANSION,                      ///< RACE_ICE_TROLL
    EXPANSION_CATACLYSM,                ///< RACE_WORGEN
    MAX_EXPANSION,                      ///< RACE_GILNEAN
    EXPANSION_MISTS_OF_PANDARIA,        ///< RACE_PANDAREN_NEUTRAL
    EXPANSION_MISTS_OF_PANDARIA,        ///< RACE_PANDAREN_ALLI
    EXPANSION_MISTS_OF_PANDARIA,        ///< RACE_PANDAREN_HORDE
};

uint8 gExpansionPerClass[] =
{
    MAX_EXPANSION,                      ///< CLASS_NONE
    EXPANSION_VANILLA,                  ///< CLASS_WARRIOR
    EXPANSION_VANILLA,                  ///< CLASS_PALADIN
    EXPANSION_VANILLA,                  ///< CLASS_HUNTER
    EXPANSION_VANILLA,                  ///< CLASS_ROGUE
    EXPANSION_VANILLA,                  ///< CLASS_PRIEST
	EXPANSION_VANILLA,   ///< CLASS_DEATH_KNIGHT
    EXPANSION_VANILLA,                  ///< CLASS_SHAMAN
    EXPANSION_VANILLA,                  ///< CLASS_MAGE
    EXPANSION_VANILLA,                  ///< CLASS_WARLOCK
	EXPANSION_VANILLA,                  ///< CLASS_MONK
    EXPANSION_VANILLA,                  ///< CLASS_DRUID
};

void WorldSession::SendAuthResponse(uint32 code, bool queued, uint32 queuePos)
{
    static ExpansionRequirementContainer availableRaces;
    static ExpansionRequirementContainer availableClasses;
    if (availableRaces.empty())
    {
        for (uint32 i = 0; i < sizeof(gExpansionPerRace); ++i)
            if (gExpansionPerRace[i] != MAX_EXPANSION)
                availableRaces[i] = gExpansionPerRace[i];

        for (uint32 i = 0; i < sizeof(gExpansionPerClass); ++i)
            if (gExpansionPerClass[i] != MAX_EXPANSION)
                availableClasses[i] = gExpansionPerClass[i];
    }

    WorldPackets::Auth::AuthResponse response;
    response.Result = code;

    if (queued)
    {
        response.WaitInfo.emplace();
        response.WaitInfo->WaitCount = queuePos;
    }
    else if (code == ERROR_OK)
    {
        response.SuccessInfo.emplace();

        // Same as TCWoD AuthHandler: both levels from account expansion.
        // IsExpansionTrial must stay false — if true, client enters Starter Edition
        // glue and shows CHAR_CREATE_EXPANSION_CLASS ("cannot create death knights").
        // See AuthenticationPackets.h AuthSuccessInfo::IsExpansionTrial.
        uint8 expansion = Expansion();
        uint8 worldExpansion = uint8(sWorld->getIntConfig(CONFIG_EXPANSION));
        if (expansion < worldExpansion)
            expansion = worldExpansion;
        if (expansion < EXPANSION_WARLORDS_OF_DRAENOR)
            expansion = EXPANSION_WARLORDS_OF_DRAENOR;

        response.SuccessInfo->AccountExpansionLevel = expansion;
        response.SuccessInfo->ActiveExpansionLevel = expansion;
        response.SuccessInfo->IsExpansionTrial = false;
        response.SuccessInfo->ForceCharacterTemplate = false;
        response.SuccessInfo->Billing.BillingPlan = SESSION_NONE;
        response.SuccessInfo->Billing.TimeRemain = 0;
        response.SuccessInfo->Billing.InGameRoom = false;
        response.SuccessInfo->VirtualRealmAddress = GetVirtualRealmAddress();
        response.SuccessInfo->VirtualRealms.emplace_back(GetVirtualRealmAddress(), true, false,
            sWorld->GetRealmName(), sWorld->GetNormalizedRealmName());
        response.SuccessInfo->AvailableRaces = &availableRaces;
        response.SuccessInfo->AvailableClasses = &availableClasses;

        if (sWorld->getBoolConfig(CONFIG_TEMPLATES_ENABLED))
            for (auto const& templ : sObjectMgr->GetCharacterTemplates())
                response.SuccessInfo->Templates.push_back(*templ.second);
    }

    SendPacket(response.Write());
}

void WorldSession::SendClientCacheVersion(uint32 version)
{
    WorldPacket data(SMSG_CACHE_VERSION, 4);
    data << uint32(version);
    SendPacket(&data);
}
