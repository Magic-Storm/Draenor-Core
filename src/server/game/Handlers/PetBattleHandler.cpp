////////////////////////////////////////////////////////////////////////////////
//
//  MILLENIUM-STUDIO
//  Copyright 2016 Millenium-studio SARL
//  All Rights Reserved.
//
////////////////////////////////////////////////////////////////////////////////

#include "Common.h"
#include "ObjectAccessor.h"
#include "ObjectMgr.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include "Player.h"
#include "PetBattle.h"
#include "WildBattlePet.h"
#include "AchievementMgr.h"
#include "BattlePetPackets.h"
#include "CellImpl.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"

namespace
{
    void FillLocations(WorldPackets::BattlePet::Locations& loc, PetBattleRequest const* request)
    {
        loc.LocationResult = int32(request->LocationResult);
        loc.BattleOrigin.Relocate(request->PetBattleCenterPosition[0], request->PetBattleCenterPosition[1], request->PetBattleCenterPosition[2]);
        loc.BattleFacing = request->BattleFacing;
        for (uint8 i = 0; i < MAX_PETBATTLE_TEAM; ++i)
            loc.PlayerPositions[i].Relocate(request->TeamPosition[i][0], request->TeamPosition[i][1], request->TeamPosition[i][2]);
    }

    void FillRoundResult(WorldPackets::BattlePet::RoundResult& result, PetBattle* battle)
    {
        bool const isPvP = battle->BattleType != PETBATTLE_TYPE_PVE;
        uint16 const pvpMaxRoundTime = isPvP ? 30 : 0;

        result.CurRound = battle->Turn;
        result.NextPetBattleState = uint8(battle->RoundResult);

        for (uint8 teamId = 0; teamId < MAX_PETBATTLE_TEAM; ++teamId)
        {
            result.NextInputFlags[teamId] = uint8(battle->Teams[teamId]->GetTeamInputFlags());
            result.NextTrapStatus[teamId] = uint8(battle->Teams[teamId]->GetTeamTrapFlags());
            result.RoundTimeSecs[teamId] = pvpMaxRoundTime;
        }

        for (uint8 petSlot = 0; petSlot < (MAX_PETBATTLE_TEAM * MAX_PETBATTLE_SLOTS); ++petSlot)
        {
            if (!battle->Pets[petSlot])
                continue;

            for (uint8 abilitySlot = 0; abilitySlot < MAX_PETBATTLE_ABILITIES; ++abilitySlot)
            {
                if (battle->Pets[petSlot]->Cooldowns[abilitySlot] != -1 || battle->Pets[petSlot]->Lockdowns[abilitySlot] != 0)
                {
                    WorldPackets::BattlePet::BattlePetAbility ability;
                    ability.AbilityID = int32(battle->Pets[petSlot]->Abilities[abilitySlot]);
                    ability.CooldownRemaining = int16(battle->Pets[petSlot]->Cooldowns[abilitySlot]);
                    ability.LockdownRemaining = int16(battle->Pets[petSlot]->Lockdowns[abilitySlot]);
                    ability.AbilityIndex = abilitySlot;
                    ability.Pboid = uint8(battle->Pets[petSlot]->ID);
                    result.Ability.push_back(ability);
                }
            }
        }

        for (PetBattleEvent const& event : battle->RoundEvents)
        {
            WorldPackets::BattlePet::Effect effect;
            effect.AbilityEffectID = event.AbilityEffectID;
            effect.Flags = uint16(event.Flags);
            effect.SourceAuraInstanceID = uint16(event.BuffTurn);
            effect.TurnInstanceID = uint16(event.RoundTurn);
            effect.EffectType = uint8(event.EventType);
            effect.CasterPBOID = uint8(event.SourcePetID);
            effect.StackDepth = uint8(event.StackDepth);

            for (PetBattleEventUpdate const& update : event.Updates)
            {
                WorldPackets::BattlePet::EffectTarget target;
                target.Type = uint16(update.UpdateType);
                target.Petx = uint8(update.TargetPetID);
                target.AuraInstanceID = update.Buff.ID;
                target.AuraAbilityID = update.Buff.AbilityID;
                target.RoundsRemaining = update.Buff.Duration;
                target.CurrentRound = update.Buff.Turn;
                target.StateID = update.State.ID;
                target.StateValue = update.State.Value;
                target.Health = update.Health;
                target.NewStatValue = update.Speed;
                target.TriggerAbilityID = update.TriggerAbilityId;
                target.BroadcastTextID = int32(update.NpcEmote.BroadcastTextID);
                effect.EffectTargetData.push_back(target);
            }

            result.EffectData.push_back(effect);
        }

        for (uint32 deadPet : battle->RoundDeadPets)
            result.PetXDied.push_back(int8(deadPet));
    }
}

void WorldSession::SendPetBattleSlotUpdates(bool p_NewSlotUnlocked)
{
    if (!m_Player || !m_Player->IsInWorld())
        return;

    uint32 unlockedSlotCount = m_Player->GetUnlockedPetBattleSlot();
    BattlePet::Ptr* petSlots = m_Player->GetBattlePetCombatTeam();

    if (unlockedSlotCount > 0)
        m_Player->SetFlag(PLAYER_FIELD_PLAYER_FLAGS, PLAYER_FLAGS_HAS_BATTLE_PET_TRAINING);

    WorldPackets::BattlePet::PetBattleSlotUpdates packet;
    packet.NewSlot = p_NewSlotUnlocked;
    packet.AutoSlotted = true;

    for (uint32 i = 0; i < MAX_PETBATTLE_SLOTS; ++i)
    {
        WorldPackets::BattlePet::BattlePetSlot slot;
        slot.Index = uint8(i);
        slot.Locked = !((i + 1) <= unlockedSlotCount);
        if (petSlots[i])
            slot.Pet.Guid = petSlots[i]->JournalID;
        packet.Slots.push_back(slot);
    }

    SendPacket(packet.Write());
}

void WorldSession::SendPetBattleRequestFailed(uint8 p_Reason)
{
    WorldPackets::BattlePet::RequestFailed packet;
    packet.Reason = p_Reason;
    SendPacket(packet.Write());
}

void WorldSession::SendPetBattlePvPChallenge(PetBattleRequest* p_Request)
{
    WorldPackets::BattlePet::PVPChallenge packet;
    packet.ChallengerGUID = p_Request->RequesterGuid;
    FillLocations(packet.Location, p_Request);
    SendPacket(packet.Write());
}

void WorldSession::SendPetBattleFinalizeLocation(PetBattleRequest* p_Request)
{
    WorldPackets::BattlePet::FinalizeLocation packet;
    FillLocations(packet.Location, p_Request);
    SendPacket(packet.Write());
}

void WorldSession::SendPetBattleInitialUpdate(PetBattle* p_Battle)
{
    WorldPackets::BattlePet::PetBattleInitialUpdate packet;
    WorldPackets::BattlePet::PetBattleFullUpdate& update = packet.MsgData;

    uint64 wildGuid = 0;
    if (p_Battle->BattleType == PETBATTLE_TYPE_PVE)
        wildGuid = p_Battle->Teams[PETBATTLE_PVE_TEAM_ID]->OwnerGuid;

    uint32 npcDisplayID = 0;
    uint32 npcCreatureID = 0;
    bool const isPvP = p_Battle->BattleType != PETBATTLE_TYPE_PVE;
    uint16 const pvpMaxRoundTime = isPvP ? 30 : 0;

    if (p_Battle->BattleType == PETBATTLE_TYPE_PVE && p_Battle->PveBattleType == PVE_PETBATTLE_TRAINER)
    {
        if (Creature* trainer = ObjectAccessor::GetObjectInOrOutOfWorld(wildGuid, (Creature*)nullptr))
        {
            npcDisplayID = trainer->GetDisplayId();
            npcCreatureID = trainer->GetEntry();
        }
    }

    for (uint8 teamId = 0; teamId < MAX_PETBATTLE_TEAM; ++teamId)
    {
        WorldPackets::BattlePet::PetBattlePlayerUpdate& player = update.Players[teamId];
        ObjectGuid ownerGuid = p_Battle->Teams[teamId]->OwnerGuid;
        if (p_Battle->BattleType == PETBATTLE_TYPE_PVE && teamId == PETBATTLE_PVE_TEAM_ID)
            ownerGuid.Clear();

        player.CharacterID = ownerGuid;
        player.TrapAbilityID = int32(p_Battle->Teams[teamId]->GetCatchAbilityID());
        player.TrapStatus = int32(teamId == PETBATTLE_TEAM_1 ? 5 : 2);
        player.RoundTimeSecs = pvpMaxRoundTime;
        if (teamId == PETBATTLE_TEAM_1 || p_Battle->Teams[teamId]->ActivePetID == PETBATTLE_NULL_ID)
            player.FrontPet = int8(p_Battle->Teams[teamId]->ActivePetID);
        else
            player.FrontPet = int8(p_Battle->Teams[teamId]->ActivePetID - (teamId == PETBATTLE_TEAM_2 ? MAX_PETBATTLE_SLOTS : 0));
        player.InputFlags = 6;

        for (uint8 petId = 0; petId < p_Battle->Teams[teamId]->TeamPetCount; ++petId)
        {
            std::shared_ptr<BattlePetInstance> pet = p_Battle->Teams[teamId]->TeamPets[petId];
            WorldPackets::BattlePet::PetBattlePetUpdate petUpdate;
            petUpdate.BattlePetGUID = (p_Battle->BattleType == PETBATTLE_TYPE_PVE && teamId == PETBATTLE_PVE_TEAM_ID) ? ObjectGuid() : ObjectGuid(pet->JournalID);
            petUpdate.SpeciesID = pet->Species;
            petUpdate.DisplayID = pet->DisplayModelID;
            petUpdate.Level = uint16(pet->Level);
            petUpdate.Xp = uint16(pet->XP);
            petUpdate.CurHealth = int32(pet->Health);
            petUpdate.MaxHealth = int32(pet->InfoMaxHealth);
            petUpdate.Power = int32(pet->InfoPower);
            petUpdate.Speed = int32(pet->InfoSpeed);
            petUpdate.BreedQuality = uint16(pet->Quality);
            petUpdate.StatusFlags = uint16(pet->Flags & ~PETBATTLE_FLAG_CAPTURED);
            petUpdate.Slot = int8(petId);
            petUpdate.CustomName = pet->Name;

            for (uint8 abilitySlot = 0; abilitySlot < MAX_PETBATTLE_ABILITIES; ++abilitySlot)
            {
                if (!pet->Abilities[abilitySlot])
                    continue;

                WorldPackets::BattlePet::BattlePetAbility ability;
                ability.AbilityID = int32(pet->Abilities[abilitySlot]);
                ability.CooldownRemaining = 0;
                ability.LockdownRemaining = int16(pet->Lockdowns[abilitySlot]);
                ability.AbilityIndex = abilitySlot;
                ability.Pboid = uint8(pet->ID);
                petUpdate.Abilities.push_back(ability);
            }

            for (int state = 0; state < NUM_BATTLEPET_STATES; ++state)
            {
                switch (state)
                {
                    case BATTLEPET_STATE_Stat_Power:
                    case BATTLEPET_STATE_Stat_Stamina:
                    case BATTLEPET_STATE_Stat_Speed:
                    case BATTLEPET_STATE_Stat_CritChance:
                    case BATTLEPET_STATE_Stat_Accuracy:
                        petUpdate.States.emplace_back(uint32(state), int32(pet->States[state]));
                        break;
                    default:
                        break;
                }
            }

            player.Pets.push_back(petUpdate);
        }
    }

    update.WaitingForFrontPetsMaxSecs = 30;
    update.PvpMaxRoundTime = 30;
    update.CurRound = int32(p_Battle->Turn);
    update.NpcCreatureID = npcCreatureID;
    update.NpcDisplayID = npcDisplayID;
    update.CurPetBattleState = 1;
    update.ForfeitPenalty = uint8(p_Battle->GetForfeitHealthPenalityPct());
    update.InitialWildPetGUID = wildGuid;
    update.IsPVP = isPvP;
    update.CanAwardXP = p_Battle->BattleType != PETBATTLE_TYPE_PVP_DUEL;
    SendPacket(packet.Write());
}

void WorldSession::SendPetBattleFirstRound(PetBattle* p_Battle)
{
    WorldPackets::BattlePet::BattleRound packet(SMSG_PET_BATTLE_FIRST_ROUND);
    FillRoundResult(packet.MsgData, p_Battle);
    SendPacket(packet.Write());
}

void WorldSession::SendPetBattleRoundResult(PetBattle* p_Battle)
{
    WorldPackets::BattlePet::BattleRound packet(SMSG_PET_BATTLE_ROUND_RESULT);
    FillRoundResult(packet.MsgData, p_Battle);
    SendPacket(packet.Write());
}

void WorldSession::SendPetBattleReplacementMade(PetBattle* p_Battle)
{
    WorldPackets::BattlePet::BattleRound packet(SMSG_PET_BATTLE_REPLACEMENTS_MADE);
    FillRoundResult(packet.MsgData, p_Battle);
    SendPacket(packet.Write());
}

void WorldSession::SendPetBattleFinalRound(PetBattle* p_Battle)
{
    WorldPackets::BattlePet::SceneObjectFinalRound packet;
    packet.MsgData.Abandoned = p_Battle->CombatResult == PETBATTLE_RESULT_ABANDON;
    packet.MsgData.PvpBattle = p_Battle->BattleType != PETBATTLE_TYPE_PVE;
    for (uint8 teamId = 0; teamId < MAX_PETBATTLE_TEAM; ++teamId)
        packet.MsgData.Winner[teamId] = p_Battle->WinnerTeamId == teamId;

    for (uint8 petSlot = 0; petSlot < (MAX_PETBATTLE_TEAM * MAX_PETBATTLE_SLOTS); ++petSlot)
    {
        if (!p_Battle->Pets[petSlot])
            continue;

        WorldPackets::BattlePet::FinalPet pet;
        pet.Level = uint16(p_Battle->Pets[petSlot]->Level);
        pet.Xp = uint16(p_Battle->Pets[petSlot]->XP);
        pet.Health = int32(p_Battle->Pets[petSlot]->Health);
        pet.MaxHealth = int32(p_Battle->Pets[petSlot]->InfoMaxHealth);
        pet.InitialLevel = uint16(p_Battle->Pets[petSlot]->OldLevel);
        pet.Pboid = uint8(p_Battle->Pets[petSlot]->ID);
        pet.Captured = p_Battle->CatchedPetId == int8(petSlot);
        pet.AwardedXP = p_Battle->Pets[petSlot]->OldXP != p_Battle->Pets[petSlot]->XP;
        pet.SeenAction = p_Battle->FightedPets.find(uint8(petSlot)) != p_Battle->FightedPets.end();
        packet.MsgData.Pets.push_back(pet);
    }

    SendPacket(packet.Write());
}

void WorldSession::SendPetBattleFinished(PetBattle* /*battle*/)
{
    WorldPacket l_Packet(SMSG_PET_BATTLE_FINISHED, 0);
    SendPacket(&l_Packet);
}

void WorldSession::SendPetBattleChatRestricted()
{
    WorldPacket l_Packet(SMSG_PET_BATTLE_CHAT_RESTRICTED, 0);
    SendPacket(&l_Packet);
}

void WorldSession::SendPetBattleMaxGameLenghtWarning()
{
    /// @TODO
}

void WorldSession::SendPetBattleQueueProposeMatch()
{
    WorldPacket l_Packet(SMSG_PET_BATTLE_QUEUE_PROPOSE_MATCH, 0);
    m_Player->GetSession()->SendPacket(&l_Packet);
}

void WorldSession::SendPetBattleQueueStatus(uint32 p_TicketTime, uint32 p_TicketID, uint32 p_Status, uint32 p_AvgWaitTime)
{
    if (!m_Player || !m_Player->GetSession())
        return;

    uint32 queuedTime = uint32(time(nullptr) - p_TicketTime);

    WorldPackets::BattlePet::PetBattleQueueStatus packet;
    packet.Msg.Status = p_Status;
    packet.Msg.Ticket.RequesterGuid = m_Player->GetGUID();
    packet.Msg.Ticket.Id = int32(p_TicketID);
    packet.Msg.Ticket.Type = 6;
    packet.Msg.Ticket.Time = p_TicketTime;

    if (p_AvgWaitTime && p_Status != LFBUpdateStatus::LFB_LEAVE_QUEUE)
        packet.Msg.AverageWaitTime = int32(p_AvgWaitTime);
    if (queuedTime && p_Status != LFBUpdateStatus::LFB_LEAVE_QUEUE)
        packet.Msg.ClientWaitTime = int32(queuedTime);

    SendPacket(packet.Write());
}

void WorldSession::SendPetBattleDebugQueueDumpResponse()
{
    /// @TODO
}

//////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void WorldSession::HandlePetBattleJoinQueue(WorldPacket& /*p_RecvData*/)
{
    if (!m_Player || !m_Player->IsInWorld())
        return;

    // Player can't be already in battle
    if (m_Player->_petBattleId)
    {
        SendPetBattleRequestFailed(PETBATTLE_REQUEST_IN_BATTLE);
        return;
    }

    // Player can't be in combat
    if (m_Player->isInCombat())
    {
        SendPetBattleRequestFailed(PETBATTLE_REQUEST_NOT_WHILE_IN_COMBAT);
        return;
    }

    std::recursive_mutex& l_Lock = sPetBattleSystem->GetLock();
    std::lock_guard<std::recursive_mutex> l_Guard(l_Lock);

    BattlePetInstance::Ptr  l_PlayerPets[MAX_PETBATTLE_SLOTS];
    size_t                  l_PlayerPetCount = 0;
    uint32                  l_ErrorCode = PETBATTLE_REQUEST_CREATE_FAILED;

    // Temporary pet buffer
    for (size_t l_CurrentPetSlot = 0; l_CurrentPetSlot < MAX_PETBATTLE_SLOTS; ++l_CurrentPetSlot)
        l_PlayerPets[l_CurrentPetSlot] = 0;

    // Load player pets
    BattlePet::Ptr* l_PetSlots = m_Player->GetBattlePetCombatTeam();

    uint32 l_DeadPetCount = 0;

    for (size_t l_CurrentPetSlot = 0; l_CurrentPetSlot < MAX_PETBATTLE_SLOTS; ++l_CurrentPetSlot)
    {
        if (!l_PetSlots[l_CurrentPetSlot])
            continue;

        if (l_PlayerPetCount >= MAX_PETBATTLE_SLOTS || l_PlayerPetCount >= m_Player->GetUnlockedPetBattleSlot())
            break;

        if (l_PetSlots[l_CurrentPetSlot]->Health == 0)
            l_DeadPetCount++;

        l_PlayerPets[l_PlayerPetCount] = BattlePetInstance::Ptr(new BattlePetInstance());
        l_PlayerPets[l_PlayerPetCount]->CloneFrom(l_PetSlots[l_CurrentPetSlot]);
        l_PlayerPets[l_PlayerPetCount]->Slot = l_PlayerPetCount;
        l_PlayerPets[l_PlayerPetCount]->OriginalBattlePet = l_PetSlots[l_CurrentPetSlot];

        ++l_PlayerPetCount;
    }

    if (l_DeadPetCount && l_DeadPetCount == l_PlayerPetCount)
    {
        SendPetBattleRequestFailed(PETBATTLE_REQUEST_ALL_PETS_DEAD);
        return;
    }

    // Check player team
    if (!l_PlayerPetCount)
    {
        SendPetBattleRequestFailed(PETBATTLE_REQUEST_NO_PETS_IN_SLOT);
        return;
    }

    sPetBattleSystem->JoinQueue(m_Player);
}

void WorldSession::HandlePetBattleLeaveQueue(WorldPacket& /*p_RecvData*/)
{
    if (!m_Player || !m_Player->IsInWorld())
        return;

    std::recursive_mutex& l_Lock = sPetBattleSystem->GetLock();
    std::lock_guard<std::recursive_mutex> l_Guard(l_Lock);

    sPetBattleSystem->LeaveQueue(m_Player);
}

void WorldSession::HandlePetBattleRequestWild(WorldPacket& p_RecvData)
{
    if (!m_Player || !m_Player->IsInWorld())
        return;

    if (m_IsPetBattleJournalLocked)
    {
        SendPetBattleRequestFailed(PETBATTLE_REQUEST_NO_ACCOUNT_LOCK);
        return;
    }

    std::recursive_mutex& l_Lock = sPetBattleSystem->GetLock();
    std::lock_guard<std::recursive_mutex> l_Guard(l_Lock);

    PetBattleRequest* l_BattleRequest = sPetBattleSystem->CreateRequest(m_Player->GetGUID());
    WorldPackets::BattlePet::RequestWild wild{ WorldPacket(p_RecvData) };
    wild.Read();

    l_BattleRequest->OpponentGuid = wild.Battle.TargetGUID;
    l_BattleRequest->LocationResult = uint32(wild.Battle.Location.LocationResult);
    l_BattleRequest->PetBattleCenterPosition[0] = wild.Battle.Location.BattleOrigin.GetPositionX();
    l_BattleRequest->PetBattleCenterPosition[1] = wild.Battle.Location.BattleOrigin.GetPositionY();
    l_BattleRequest->PetBattleCenterPosition[2] = wild.Battle.Location.BattleOrigin.GetPositionZ();
    l_BattleRequest->BattleFacing = wild.Battle.Location.BattleFacing;
    for (int teamId = 0; teamId < MAX_PETBATTLE_TEAM; ++teamId)
    {
        l_BattleRequest->TeamPosition[teamId][0] = wild.Battle.Location.PlayerPositions[teamId].GetPositionX();
        l_BattleRequest->TeamPosition[teamId][1] = wild.Battle.Location.PlayerPositions[teamId].GetPositionY();
        l_BattleRequest->TeamPosition[teamId][2] = wild.Battle.Location.PlayerPositions[teamId].GetPositionZ();
    }

    l_BattleRequest->RequestType = PETBATTLE_TYPE_PVE;

    eBattlePetRequests l_CanEnterResult = sPetBattleSystem->CanPlayerEnterInPetBattle(m_Player, l_BattleRequest);
    if (l_CanEnterResult != PETBATTLE_REQUEST_OK)
    {
        SendPetBattleRequestFailed(l_CanEnterResult);
        sPetBattleSystem->RemoveRequest(l_BattleRequest->RequesterGuid);
        return;
    }

    /// Wild should be for PetBattle and Player able to interact with it
    Creature* l_WildPet = m_Player->GetNPCIfCanInteractWith(l_BattleRequest->OpponentGuid, UNIT_NPC_FLAG_PETBATTLE);
    if (!l_WildPet)
    {
        SendPetBattleRequestFailed(PETBATTLE_REQUEST_TARGET_NOT_CAPTURABLE);
        sPetBattleSystem->RemoveRequest(l_BattleRequest->RequesterGuid);
        return;
    }

    ////////////////////////////////
    ////////////////////////////////

    Creature* l_Wild = sObjectAccessor->GetCreature(*m_Player, l_BattleRequest->OpponentGuid);

    if (!l_Wild)
    {
        SendPetBattleRequestFailed(PETBATTLE_REQUEST_TARGET_NOT_CAPTURABLE);
        sPetBattleSystem->RemoveRequest(l_BattleRequest->RequesterGuid);
        return;
    }

    ////////////////////////////////
    ////////////////////////////////

    /// Wild should be wild
    if (!l_Wild->GetMap()->GetWildBattlePetPools()->IsWildPet(l_Wild))
    {
        SendPetBattleRequestFailed(PETBATTLE_REQUEST_TARGET_NOT_CAPTURABLE);
        sPetBattleSystem->RemoveRequest(l_BattleRequest->RequesterGuid);
        return;
    }

    ////////////////////////////////
    ////////////////////////////////

    BattlePetInstance::Ptr  l_PlayerPets[MAX_PETBATTLE_SLOTS];
    BattlePetInstance::Ptr  l_WildBattlePets[MAX_PETBATTLE_SLOTS];
    size_t                  l_PlayerPetCount = 0;
    PetBattle* l_Battle;

    /// Temporary pet buffer
    for (size_t l_CurrentPetSlot = 0; l_CurrentPetSlot < MAX_PETBATTLE_SLOTS; ++l_CurrentPetSlot)
    {
        l_PlayerPets[l_CurrentPetSlot] = nullptr;
        l_WildBattlePets[l_CurrentPetSlot] = nullptr;
    }

    ////////////////////////////////
    ////////////////////////////////

    WildBattlePetZonePools* l_WildBattlePetPools = l_Wild->GetMap()->GetWildBattlePetPools();

    BattlePetInstance::Ptr l_WildBattlePet = l_WildBattlePetPools->GetWildBattlePet(l_Wild);

    if (!l_WildBattlePet)
    {
        l_WildBattlePet = nullptr;

        SendPetBattleRequestFailed(PETBATTLE_REQUEST_TARGET_NOT_CAPTURABLE);
        sPetBattleSystem->RemoveRequest(l_BattleRequest->RequesterGuid);
        return;
    }

    l_WildBattlePets[0] = l_WildBattlePet;

    std::list<Unit*> l_NearCreature;
    Trinity::AnyFriendlyUnitInObjectRangeCheck l_UCheck(l_Wild, l_Wild, 50.0f);
    Trinity::UnitListSearcher<Trinity::AnyFriendlyUnitInObjectRangeCheck> l_Searcher(l_Wild, l_NearCreature, l_UCheck);
    l_Wild->VisitNearbyObject(40.0f, l_Searcher);

    uint32 l_WildsPetCount = 1;
    for (Unit* l_Current : l_NearCreature)
    {
        if (l_WildsPetCount >= MAX_PETBATTLE_SLOTS)
            break;

        if (!l_Current->ToCreature() || !l_Current->isAlive() || l_Current->GetGUID() == l_Wild->GetGUID() || !l_WildBattlePetPools->IsWildPet(l_Current->ToCreature()))
            continue;

        if (l_WildBattlePetPools->GetWildBattlePet(l_Current->ToCreature()) != nullptr && roll_chance_i(80))
        {
            l_WildBattlePets[l_WildsPetCount] = l_WildBattlePetPools->GetWildBattlePet(l_Current->ToCreature());
            l_WildBattlePets[l_WildsPetCount]->OriginalCreature = l_Current->GetGUID();
            l_WildsPetCount++;
        }
    }

    ////////////////////////////////
    ////////////////////////////////

    /// Load player pets
    BattlePet::Ptr* l_PetSlots = m_Player->GetBattlePetCombatTeam();

    for (size_t l_CurrentPetSlot = 0; l_CurrentPetSlot < MAX_PETBATTLE_SLOTS; ++l_CurrentPetSlot)
    {
        if (!l_PetSlots[l_CurrentPetSlot])
            continue;

        if (l_PlayerPetCount >= MAX_PETBATTLE_SLOTS || l_PlayerPetCount >= m_Player->GetUnlockedPetBattleSlot())
            break;

        l_PlayerPets[l_PlayerPetCount] = BattlePetInstance::Ptr(new BattlePetInstance());
        l_PlayerPets[l_PlayerPetCount]->CloneFrom(l_PetSlots[l_CurrentPetSlot]);
        l_PlayerPets[l_PlayerPetCount]->Slot = l_PlayerPetCount;
        l_PlayerPets[l_PlayerPetCount]->OriginalBattlePet = l_PetSlots[l_CurrentPetSlot];

        ++l_PlayerPetCount;
    }

    ////////////////////////////////
    ////////////////////////////////

    m_Player->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PACIFIED | UNIT_FLAG_IMMUNE_TO_NPC); ///< Immuned only to NPC
    m_Player->SetTarget(l_WildPet->GetGUID());

    SendPetBattleFinalizeLocation(l_BattleRequest);

    m_Player->SetFacingTo(m_Player->GetAngle(l_BattleRequest->TeamPosition[PETBATTLE_TEAM_2][0], l_BattleRequest->TeamPosition[PETBATTLE_TEAM_2][1]));
    m_Player->SetRooted(true);

    /// Ok, Create battle
    l_Battle = sPetBattleSystem->CreateBattle();

    /// Add player pets
    l_Battle->Teams[PETBATTLE_TEAM_1]->OwnerGuid = m_Player->GetGUID();
    l_Battle->Teams[PETBATTLE_TEAM_1]->PlayerGuid = m_Player->GetGUID();

    l_Battle->Teams[PETBATTLE_TEAM_2]->OwnerGuid = l_Wild->GetGUID();

    for (size_t l_CurrentPetSlot = 0; l_CurrentPetSlot < MAX_PETBATTLE_SLOTS; ++l_CurrentPetSlot)
    {
        if (l_PlayerPets[l_CurrentPetSlot])
            l_Battle->AddPet(PETBATTLE_TEAM_1, l_PlayerPets[l_CurrentPetSlot]);

        if (l_WildBattlePets[l_CurrentPetSlot])
        {
            l_Battle->AddPet(PETBATTLE_TEAM_2, l_WildBattlePets[l_CurrentPetSlot]);

            if (Creature* l_CurrrentCreature = sObjectAccessor->GetCreature(*m_Player, l_WildBattlePets[l_CurrentPetSlot]->OriginalCreature))
            {
                /// Ok, Despawn wild
                l_CurrrentCreature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PACIFIED | UNIT_FLAG_IMMUNE_TO_PC); // Immuned to all
                l_CurrrentCreature->SetTarget(m_Player->GetGUID());
                l_CurrrentCreature->SetControlled(true, UNIT_STATE_ROOT);
                l_CurrrentCreature->_petBattleId = l_Battle->ID;

                l_WildBattlePetPools->EnterInBattle(l_CurrrentCreature);
            }
        }
    }

    l_Battle->BattleType = l_BattleRequest->RequestType;
    l_Battle->PveBattleType = PVE_PETBATTLE_WILD;

    /// Launch battle
    m_Player->_petBattleId = l_Battle->ID;
    l_Battle->Begin();

    sPetBattleSystem->RemoveRequest(l_BattleRequest->RequesterGuid);

    for (size_t l_CurrentPetID = 0; l_CurrentPetID < MAX_PETBATTLE_SLOTS; ++l_CurrentPetID)
    {
        if (l_PlayerPets[l_CurrentPetID])
            l_PlayerPets[l_CurrentPetID] = nullptr;

        if (l_WildBattlePets[l_CurrentPetID])
            l_WildBattlePets[l_CurrentPetID] = nullptr;
    }

    l_WildBattlePet = nullptr;
}

void WorldSession::HandlePetBattleWildLocationFail(WorldPacket& /*p_RecvData*/)
{

}

void WorldSession::HandlePetBattleRequestPvP(WorldPacket& p_RecvData)
{
    if (!m_Player || !m_Player->IsInWorld())
        return;

    std::recursive_mutex& l_Lock = sPetBattleSystem->GetLock();
    std::lock_guard<std::recursive_mutex> l_Guard(l_Lock);

    PetBattleRequest* l_BattleRequest = sPetBattleSystem->CreateRequest(m_Player->GetGUID());
    WorldPackets::BattlePet::RequestPVP pvp{ WorldPacket(p_RecvData) };
    pvp.Read();

    uint64 l_OpponentGuid = pvp.Battle.TargetGUID;
    l_BattleRequest->OpponentGuid = l_OpponentGuid;
    l_BattleRequest->LocationResult = uint32(pvp.Battle.Location.LocationResult);
    l_BattleRequest->PetBattleCenterPosition[0] = pvp.Battle.Location.BattleOrigin.GetPositionX();
    l_BattleRequest->PetBattleCenterPosition[1] = pvp.Battle.Location.BattleOrigin.GetPositionY();
    l_BattleRequest->PetBattleCenterPosition[2] = pvp.Battle.Location.BattleOrigin.GetPositionZ();
    l_BattleRequest->BattleFacing = pvp.Battle.Location.BattleFacing;
    for (int teamId = 0; teamId < MAX_PETBATTLE_TEAM; ++teamId)
    {
        l_BattleRequest->TeamPosition[teamId][0] = pvp.Battle.Location.PlayerPositions[teamId].GetPositionX();
        l_BattleRequest->TeamPosition[teamId][1] = pvp.Battle.Location.PlayerPositions[teamId].GetPositionY();
        l_BattleRequest->TeamPosition[teamId][2] = pvp.Battle.Location.PlayerPositions[teamId].GetPositionZ();
    }

    l_BattleRequest->RequestType = PETBATTLE_TYPE_PVP_DUEL;

    // Player can't be already in battle
    if (m_Player->_petBattleId)
    {
        SendPetBattleRequestFailed(PETBATTLE_REQUEST_IN_BATTLE);
        sPetBattleSystem->RemoveRequest(l_BattleRequest->RequesterGuid);
        return;
    }

    // Player can't be in combat
    if (m_Player->isInCombat())
    {
        SendPetBattleRequestFailed(PETBATTLE_REQUEST_NOT_WHILE_IN_COMBAT);
        sPetBattleSystem->RemoveRequest(l_BattleRequest->RequesterGuid);
        return;
    }

    // Check positions
    for (size_t l_CurrentTeamID = 0; l_CurrentTeamID < MAX_PETBATTLE_TEAM; ++l_CurrentTeamID)
    {
        if (m_Player->GetMap()->getObjectHitPos(m_Player->GetPhaseMask(), l_BattleRequest->PetBattleCenterPosition[0], l_BattleRequest->PetBattleCenterPosition[1], l_BattleRequest->PetBattleCenterPosition[2],
            l_BattleRequest->TeamPosition[l_CurrentTeamID][0], l_BattleRequest->TeamPosition[l_CurrentTeamID][1], l_BattleRequest->TeamPosition[l_CurrentTeamID][2],
            l_BattleRequest->TeamPosition[l_CurrentTeamID][0], l_BattleRequest->TeamPosition[l_CurrentTeamID][1], l_BattleRequest->TeamPosition[l_CurrentTeamID][2], 0.0f))
        {
            SendPetBattleRequestFailed(PETBATTLE_REQUEST_NOT_HERE_UNEVEN_GROUND);
            sPetBattleSystem->RemoveRequest(l_BattleRequest->RequesterGuid);
            return;
        }
    }

    Player* l_Opposant = HashMapHolder<Player>::Find(l_OpponentGuid);
    if (!l_Opposant)
    {
        SendPetBattleRequestFailed(PETBATTLE_REQUEST_TARGET_INVALID);
        sPetBattleSystem->RemoveRequest(l_BattleRequest->RequesterGuid);
        return;
    }

    // Player can't be already in battle
    if (l_Opposant->_petBattleId)
    {
        SendPetBattleRequestFailed(PETBATTLE_REQUEST_IN_BATTLE);
        sPetBattleSystem->RemoveRequest(l_BattleRequest->RequesterGuid);
        return;
    }

    // Player can't be in combat
    if (l_Opposant->isInCombat())
    {
        SendPetBattleRequestFailed(PETBATTLE_REQUEST_NOT_WHILE_IN_COMBAT);
        sPetBattleSystem->RemoveRequest(l_BattleRequest->RequesterGuid);
        return;
    }

    l_BattleRequest->IsPvPReady[PETBATTLE_TEAM_1] = true;
    l_Opposant->GetSession()->SendPetBattlePvPChallenge(l_BattleRequest);
}

void WorldSession::HandlePetBattleRequestUpdate(WorldPacket& p_RecvData)
{
    WorldPackets::BattlePet::RequestUpdate update{ WorldPacket(p_RecvData) };
    update.Read();
    uint64 l_TargetGUID = update.TargetGUID;
    bool l_Canceled = update.Canceled;

    std::recursive_mutex& l_Lock = sPetBattleSystem->GetLock();
    std::lock_guard<std::recursive_mutex> l_Guard(l_Lock);

    PetBattleRequest* l_BattleRequest = sPetBattleSystem->GetRequest(l_TargetGUID);
    Player* l_Opposant = HashMapHolder<Player>::Find(l_TargetGUID);

    if (!l_Canceled && l_BattleRequest && l_Opposant)
    {
        BattlePetInstance::Ptr  l_PlayerPets[MAX_PETBATTLE_SLOTS];
        BattlePetInstance::Ptr  l_PlayerOpposantPets[MAX_PETBATTLE_SLOTS];
        size_t                  l_PlayerPetCount = 0;
        size_t                  l_PlayerOpposantPetCount = 0;
        PetBattle* l_Battle;
        uint32                  l_ErrorCode = PETBATTLE_REQUEST_CREATE_FAILED;

        // Temporary pet buffer
        for (size_t l_CurrentPetSlot = 0; l_CurrentPetSlot < MAX_PETBATTLE_SLOTS; ++l_CurrentPetSlot)
        {
            l_PlayerPets[l_CurrentPetSlot] = 0;
            l_PlayerOpposantPets[l_CurrentPetSlot] = 0;
        }

        // Load player pets
        BattlePet::Ptr* l_PetSlots = m_Player->GetBattlePetCombatTeam();

        for (size_t l_CurrentPetSlot = 0; l_CurrentPetSlot < MAX_PETBATTLE_SLOTS; ++l_CurrentPetSlot)
        {
            if (!l_PetSlots[l_CurrentPetSlot])
                continue;

            if (l_PlayerPetCount >= MAX_PETBATTLE_SLOTS || l_PlayerPetCount >= m_Player->GetUnlockedPetBattleSlot())
                break;

            l_PlayerPets[l_PlayerPetCount] = BattlePetInstance::Ptr(new BattlePetInstance());
            l_PlayerPets[l_PlayerPetCount]->CloneFrom(l_PetSlots[l_CurrentPetSlot]);
            l_PlayerPets[l_PlayerPetCount]->Slot = l_PlayerPetCount;
            l_PlayerPets[l_PlayerPetCount]->OriginalBattlePet = l_PetSlots[l_CurrentPetSlot];

            ++l_PlayerPetCount;
        }

        BattlePet::Ptr* l_PetOpposantSlots = l_Opposant->GetBattlePetCombatTeam();

        for (size_t l_CurrentOpposantPetSlot = 0; l_CurrentOpposantPetSlot < MAX_PETBATTLE_SLOTS; ++l_CurrentOpposantPetSlot)
        {
            if (!l_PetOpposantSlots[l_CurrentOpposantPetSlot])
                continue;

            if (l_PlayerOpposantPetCount >= MAX_PETBATTLE_SLOTS || l_PlayerOpposantPetCount >= m_Player->GetUnlockedPetBattleSlot())
                break;

            l_PlayerOpposantPets[l_PlayerOpposantPetCount] = BattlePetInstance::Ptr(new BattlePetInstance());
            l_PlayerOpposantPets[l_PlayerOpposantPetCount]->CloneFrom(l_PetOpposantSlots[l_CurrentOpposantPetSlot]);
            l_PlayerOpposantPets[l_PlayerOpposantPetCount]->Slot = l_PlayerOpposantPetCount;
            l_PlayerOpposantPets[l_PlayerOpposantPetCount]->OriginalBattlePet = l_PetOpposantSlots[l_CurrentOpposantPetSlot];

            ++l_PlayerOpposantPetCount;
        }

        if (!l_PlayerOpposantPetCount || !l_PlayerPetCount)
        {
            m_Player->GetSession()->SendPetBattleRequestFailed(PETBATTLE_REQUEST_NO_PETS_IN_SLOT);
            l_Opposant->GetSession()->SendPetBattleRequestFailed(PETBATTLE_REQUEST_NO_PETS_IN_SLOT);
            sPetBattleSystem->RemoveRequest(l_TargetGUID);
            return;
        }

        m_Player->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PACIFIED | UNIT_FLAG_IMMUNE_TO_NPC); // Immuned only to NPC
        l_Opposant->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PACIFIED | UNIT_FLAG_IMMUNE_TO_NPC); // Immuned only to NPC

        m_Player->GetSession()->SendPetBattleFinalizeLocation(l_BattleRequest);
        l_Opposant->GetSession()->SendPetBattleFinalizeLocation(l_BattleRequest);

        m_Player->SetFacingTo(m_Player->GetAngle(l_BattleRequest->TeamPosition[PETBATTLE_TEAM_1][0], l_BattleRequest->TeamPosition[PETBATTLE_TEAM_1][1]));
        l_Opposant->SetFacingTo(m_Player->GetAngle(l_BattleRequest->TeamPosition[PETBATTLE_TEAM_2][0], l_BattleRequest->TeamPosition[PETBATTLE_TEAM_2][1]));
        m_Player->SetRooted(true);
        l_Opposant->SetRooted(true);

        // Ok, Create battle
        l_Battle = sPetBattleSystem->CreateBattle();

        // Add player pets
        l_Battle->Teams[PETBATTLE_TEAM_1]->OwnerGuid = l_Opposant->GetGUID();
        l_Battle->Teams[PETBATTLE_TEAM_1]->PlayerGuid = l_Opposant->GetGUID();
        l_Battle->Teams[PETBATTLE_TEAM_2]->OwnerGuid = m_Player->GetGUID();
        l_Battle->Teams[PETBATTLE_TEAM_2]->PlayerGuid = m_Player->GetGUID();

        for (size_t l_CurrentPetSlot = 0; l_CurrentPetSlot < MAX_PETBATTLE_SLOTS; ++l_CurrentPetSlot)
        {
            if (l_PlayerOpposantPets[l_CurrentPetSlot])
                l_Battle->AddPet(PETBATTLE_TEAM_1, l_PlayerOpposantPets[l_CurrentPetSlot]);

            if (l_PlayerPets[l_CurrentPetSlot])
                l_Battle->AddPet(PETBATTLE_TEAM_2, l_PlayerPets[l_CurrentPetSlot]);
        }

        l_Battle->BattleType = l_BattleRequest->RequestType;

        // Launch battle
        m_Player->_petBattleId = l_Battle->ID;
        l_Opposant->_petBattleId = l_Battle->ID;
        l_Battle->Begin();

        sPetBattleSystem->RemoveRequest(l_BattleRequest->RequesterGuid);

        for (size_t l_CurrentPetID = 0; l_CurrentPetID < MAX_PETBATTLE_SLOTS; ++l_CurrentPetID)
        {
            if (l_PlayerPets[l_CurrentPetID])
                l_PlayerPets[l_CurrentPetID] = BattlePetInstance::Ptr();

            if (l_PlayerOpposantPets[l_CurrentPetID])
                l_PlayerOpposantPets[l_CurrentPetID] = BattlePetInstance::Ptr();
        }
    }
    else
    {
        if (l_Opposant)
            l_Opposant->GetSession()->SendPetBattleRequestFailed(PETBATTLE_REQUEST_DECLINED);
        sPetBattleSystem->RemoveRequest(l_TargetGUID);
    }
}

void WorldSession::HandlePetBattleQuitNotify(WorldPacket& /*p_RecvData*/)
{
    Player* player = GetPlayer();
    if (!player)
        return;

    // Get current pet battle using the battle system
    PetBattle* petBattle = sPetBattleSystem->GetBattle(player->_petBattleId);
    if (!petBattle)
        return;

    // Mark battle as abandoned and set opponent as winner
    petBattle->Abandoned = true;
    petBattle->WinnerTeamId = (petBattle->Teams[PETBATTLE_TEAM_1]->PlayerGuid == player->GetGUID()) ? PETBATTLE_TEAM_2 : PETBATTLE_TEAM_1;

    // Finish the battle with abandonment flag
    petBattle->Finish(petBattle->WinnerTeamId, true);
}

void WorldSession::HandlePetBattleFinalNotify(WorldPacket& /*p_RecvData*/)
{
    Player* player = GetPlayer();
    if (!player)
        return;

    // Get current pet battle using the battle system
    PetBattle* petBattle = sPetBattleSystem->GetBattle(player->_petBattleId);
    if (!petBattle)
        return;

    // Finish the battle normally (not abandoned)
    petBattle->Finish(petBattle->WinnerTeamId, false);
}

void WorldSession::HandlePetBattleQueueProposeMatchResult(WorldPacket& p_RecvData)
{
    if (!m_Player || !m_Player->IsInWorld())
        return;

    std::recursive_mutex& l_Lock = sPetBattleSystem->GetLock();
    std::lock_guard<std::recursive_mutex> l_Guard(l_Lock);

    sPetBattleSystem->ProposalResponse(m_Player, p_RecvData.ReadBit());
}

enum ePetBattleActions
{
    PETBATTLE_ACTION_REQUEST_LEAVE = 0,
    PETBATTLE_ACTION_CAST = 1,
    PETBATTLE_ACTION_SWAP_OR_PASS = 2,
    PETBATTLE_ACTION_CATCH = 3,
    PETBATTLE_ACTION_LEAVE_PETBATTLE = 4
};

void WorldSession::HandlePetBattleInput(WorldPacket& p_RecvData)
{
    WorldPackets::BattlePet::PetBattleInput input{ WorldPacket(p_RecvData) };
    input.Read();

    uint8 l_Action = input.MoveType;
    uint8 l_NewFrontPetID = uint8(input.NewFrontPet);
    uint32 l_Ability = uint32(input.AbilityID);
    uint32 l_Turn = uint32(input.Round);

    // Validate input parameters
    if (l_Action > PETBATTLE_ACTION_LEAVE_PETBATTLE)
    {
        TC_LOG_ERROR("network", "Player %s sent invalid pet battle action %u", m_Player->GetName(), l_Action);
        return;
    }

    if (l_NewFrontPetID >= MAX_PETBATTLE_SLOTS)
    {
        TC_LOG_ERROR("network", "Player %s sent invalid front pet ID %u", m_Player->GetName(), l_NewFrontPetID);
        return;
    }

    std::recursive_mutex& l_Lock = sPetBattleSystem->GetLock();
    std::lock_guard<std::recursive_mutex> l_Guard(l_Lock);

    if (l_Action == PETBATTLE_ACTION_LEAVE_PETBATTLE)
    {
        SendPetBattleFinished(0);
        return;
    }

    if (!m_Player->_petBattleId)
    {
        TC_LOG_DEBUG("network", "Player %s tried to use pet battle action without being in a battle", m_Player->GetName());
        SendPetBattleFinished(0);
        return;
    }

    PetBattle* l_PetBattle = sPetBattleSystem->GetBattle(m_Player->_petBattleId);

    if (!l_PetBattle)
    {
        TC_LOG_ERROR("network", "Player %s tried to use pet battle action but battle %u not found", m_Player->GetName(), m_Player->_petBattleId);
        SendPetBattleFinished(0);
        return;
    }

    if (l_PetBattle->BattleStatus == PETBATTLE_STATUS_FINISHED)
    {
        TC_LOG_DEBUG("network", "Player %s tried to use pet battle action but battle is finished", m_Player->GetName());
        SendPetBattleFinished(0);
        return;
    }

    ///// Check sync - prevent desync attacks
    if ((l_Turn + 1) != l_PetBattle->Turn)
    {
        TC_LOG_WARN("network", "Player %s sent desynced pet battle turn %u (expected %u)", m_Player->GetName(), l_Turn, l_PetBattle->Turn - 1);
        sPetBattleSystem->ForfeitBattle(l_PetBattle->ID, m_Player->GetGUID());
        return;
    }

    if (l_Action)
    {
        uint32 l_PlayerTeamID = 0;

        if (l_PetBattle->Teams[PETBATTLE_TEAM_2]->PlayerGuid == m_Player->GetGUID())
            l_PlayerTeamID = PETBATTLE_TEAM_2;

        // Skip if input already died
        if (l_PetBattle->Teams[l_PlayerTeamID]->Ready)
        {
            TC_LOG_DEBUG("network", "Player %s tried to use pet battle action but team is already ready", m_Player->GetName());
            return;
        }

        // Validate team exists and is valid
        if (!l_PetBattle->Teams[l_PlayerTeamID])
        {
            TC_LOG_ERROR("network", "Player %s has invalid team ID %u in pet battle", m_Player->GetName(), l_PlayerTeamID);
            return;
        }

        if (l_Action == PETBATTLE_ACTION_REQUEST_LEAVE)
        {
            TC_LOG_DEBUG("network", "Player %s requested to leave pet battle %u", m_Player->GetName(), l_PetBattle->ID);
            sPetBattleSystem->ForfeitBattle(l_PetBattle->ID, m_Player->GetGUID());
        }
        else if (l_Action == PETBATTLE_ACTION_CAST)
        {
            // Validate ability ID
            if (l_Ability == 0)
            {
                TC_LOG_WARN("network", "Player %s tried to cast ability with ID 0", m_Player->GetName());
                return;
            }

            if (l_PetBattle->CanCast(l_PlayerTeamID, l_Ability))
            {
                TC_LOG_DEBUG("network", "Player %s casting ability %u in pet battle", m_Player->GetName(), l_Ability);
                l_PetBattle->PrepareCast(l_PlayerTeamID, l_Ability);
            }
            else
            {
                TC_LOG_DEBUG("network", "Player %s tried to cast invalid ability %u", m_Player->GetName(), l_Ability);
            }
        }
        else if (l_Action == PETBATTLE_ACTION_CATCH)
        {
            uint32 l_CatchAbilityID = l_PetBattle->Teams[l_PlayerTeamID]->GetCatchAbilityID();

            if (l_PetBattle->Teams[l_PlayerTeamID]->CanCatchOpponentTeamFrontPet() == PETBATTLE_TEAM_CATCH_FLAG_ENABLE_TRAP)
            {
                TC_LOG_DEBUG("network", "Player %s attempting to catch pet in battle", m_Player->GetName());
                l_PetBattle->PrepareCast(l_PlayerTeamID, l_CatchAbilityID);
            }
            else
            {
                TC_LOG_DEBUG("network", "Player %s tried to catch but conditions not met", m_Player->GetName());
            }
        }
        else if (l_Action == PETBATTLE_ACTION_SWAP_OR_PASS)
        {
            l_NewFrontPetID = (l_PlayerTeamID == PETBATTLE_TEAM_2 ? MAX_PETBATTLE_SLOTS : 0) + l_NewFrontPetID;

            // Validate pet ID range
            if (l_NewFrontPetID >= (l_PlayerTeamID == PETBATTLE_TEAM_2 ? MAX_PETBATTLE_SLOTS * 2 : MAX_PETBATTLE_SLOTS))
            {
                TC_LOG_WARN("network", "Player %s tried to swap to invalid pet ID %u", m_Player->GetName(), l_NewFrontPetID);
                return;
            }

            if (!l_PetBattle->Teams[l_PlayerTeamID]->CanSwap(l_NewFrontPetID))
            {
                TC_LOG_DEBUG("network", "Player %s tried to swap to pet %u but swap not allowed", m_Player->GetName(), l_NewFrontPetID);
                return;
            }

            TC_LOG_DEBUG("network", "Player %s swapping to pet %u", m_Player->GetName(), l_NewFrontPetID);
            l_PetBattle->SwapPet(l_PlayerTeamID, l_NewFrontPetID);
        }
    }
}

void WorldSession::HandlePetBattleReplaceFrontPet(WorldPacket& p_RecvData)
{
    std::recursive_mutex& l_Lock = sPetBattleSystem->GetLock();
    std::lock_guard<std::recursive_mutex> l_Guard(l_Lock);

    if (!m_Player->_petBattleId)
    {
        SendPetBattleFinished(0);
        return;
    }

    PetBattle* l_PetBattle = sPetBattleSystem->GetBattle(m_Player->_petBattleId);

    if (!l_PetBattle || l_PetBattle->BattleStatus == PETBATTLE_STATUS_FINISHED)
    {
        SendPetBattleFinished(0);
        return;
    }

    uint8 l_NewFrontPetID = 0;
    uint32 l_PlayerTeamID = 0;

    p_RecvData >> l_NewFrontPetID;

    if (l_PetBattle->Teams[PETBATTLE_TEAM_2]->PlayerGuid == m_Player->GetGUID())
        l_PlayerTeamID = PETBATTLE_TEAM_2;

    // Skip if input already died
    if (l_PetBattle->Teams[l_PlayerTeamID]->Ready)
        return;

    l_NewFrontPetID = (l_PlayerTeamID == PETBATTLE_TEAM_2 ? MAX_PETBATTLE_SLOTS : 0) + l_NewFrontPetID;

    if (!l_PetBattle->Teams[l_PlayerTeamID]->CanSwap(l_NewFrontPetID))
        return;

    l_PetBattle->SwapPet(l_PlayerTeamID, l_NewFrontPetID);
    l_PetBattle->SwapPet(!l_PlayerTeamID, l_PetBattle->Teams[!l_PlayerTeamID]->ActivePetID);
}