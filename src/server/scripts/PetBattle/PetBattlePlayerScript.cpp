////////////////////////////////////////////////////////////////////////////////
//
//  MILLENIUM-STUDIO
//  Copyright 2016 Millenium-studio SARL
//  All Rights Reserved.
//
////////////////////////////////////////////////////////////////////////////////

#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "ScriptedGossip.h"
#include "PetBattle.h"
#include "Common.h"

class PlayerScriptPetBattle : public PlayerScript
{
    public:
        PlayerScriptPetBattle() : PlayerScript("PlayerScriptPetBattle") {}

        void OnMovementInform(Player* player, uint32 p_MoveType, uint32 p_ID) override
        {
            if (player && p_MoveType == POINT_MOTION_TYPE && p_ID == PETBATTLE_ENTER_MOVE_SPLINE_ID)
            {
                player->AddDelayedEvent([p = player]() -> void
                {
                    if (auto petbattle = sPetBattleSystem->GetBattle(p->_petBattleId))
                    {
                        p->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PACIFIED | UNIT_FLAG_IMMUNE_TO_NPC); // Immuned only to NPC
                        p->SetRooted(true);

                        petbattle->Begin();
                    }
                }, 1000);
            }
        }
};

#ifndef __clang_analyzer__
void AddSC_PetBattlePlayerScript()
{
    new PlayerScriptPetBattle;
}
#endif
