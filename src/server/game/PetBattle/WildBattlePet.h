////////////////////////////////////////////////////////////////////////////////
//
//  MILLENIUM-STUDIO
//  Copyright 2016 Millenium-studio SARL
//  All Rights Reserved.
//
////////////////////////////////////////////////////////////////////////////////

#pragma once
#ifndef _WILDBATTLEPET_HEADER_
#define _WILDBATTLEPET_HEADER_

#include "Common.h"
#include "Timer.h"
#include "PetBattle.h"

#define WILDBATTLEPETMGR_UPDATE_INTERVAL 6000
#define WILDBATTLEPET_RESPAWN_WHEN_NOT_DEFEATED 10

class Creature;

struct WildBattlePetPoolTemplate
{
    uint32 Species;
    uint32 Entry;
    uint32 Replace;
    uint32 Max;
    uint32 RespawnTime;
    uint32 MinLevel;
    uint32 MaxLevel;
    uint32 Breeds[10];

    std::list<uint64>            ToBeReplaced;
    std::list<uint64>            Replaced;

    /// Temporary mapping for pet replacement mechanics - may need refactoring
    std::map<uint64, uint64>                                ReplacedRelation;
    std::map<uint64, std::shared_ptr<BattlePetInstance>>    ReplacedBattlePetInstances;
};

class WildBattlePetZonePools
{
    public:
        void LoadPoolTemplate(Field* fields);

        void Populate();
        void Depopulate();

        void OnAddToMap(Creature* p_Creature);
        void OnRemoveToMap(Creature* p_Creature);

        void EnterInBattle(Creature* p_Creature);
        void LeaveBattle(Creature* p_Creature, bool p_Defeated);

        void ReplaceCreature(Creature* p_Creature, WildBattlePetPoolTemplate* p_Template);
        void UnreplaceCreature(Creature* p_Creature, WildBattlePetPoolTemplate* p_Template);

        bool IsWildPet(Creature* p_Creature);
        std::shared_ptr<BattlePetInstance> GetWildBattlePet(Creature* p_Creature);

    public:
        uint32 MapID;

        std::vector<WildBattlePetPoolTemplate> m_Templates;
};

class WildBattlePetMgr
{
    public:
        WildBattlePetMgr();
        
        static WildBattlePetMgr* instance()
        {
            static WildBattlePetMgr* instance = new WildBattlePetMgr();
            return instance;
        }

        void Load();

        WildBattlePetZonePools GetZonePoolsForNewMap(uint32 p_MapId);

    private:
        ///        map       pools
        std::map<uint32, WildBattlePetZonePools>  m_PoolsByMap;
};

#define sWildBattlePetMgr WildBattlePetMgr::instance()

#endif
