/*
 * Copyright (C) 2008-2016 TrinityCore <http://www.trinitycore.org/>
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

#include "BattlePetPackets.h"
#include "World.h"
#include "Realm.h"

ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::BattlePet::BattlePetSlot const& slot)
{
    data << (slot.Pet.Guid.IsEmpty() ? ObjectGuid::Create<HIGHGUID_BATTLE_PET>(0) : slot.Pet.Guid);
    data << uint32(slot.CollarID);
    data << uint8(slot.Index);
    data.WriteBit(slot.Locked);
    data.FlushBits();
    return data;
}

ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::BattlePet::BattlePet const& pet)
{
    data << pet.Guid;
    data << uint32(pet.Species);
    data << uint32(pet.CreatureID);
    data << uint32(pet.DisplayID);
    data << uint16(pet.Breed);
    data << uint16(pet.Level);
    data << uint16(pet.Exp);
    data << uint16(pet.Flags);
    data << uint32(pet.Power);
    data << uint32(pet.Health);
    data << uint32(pet.MaxHealth);
    data << uint32(pet.Speed);
    data << uint8(pet.Quality);
    data.WriteBits(pet.Name.size(), 7);
    data.WriteBit(!pet.Owner.IsEmpty());
    data.WriteBit(pet.Name.empty());
    data.FlushBits();

    data.WriteString(pet.Name);

    if (!pet.Owner.IsEmpty())
    {
        data << pet.Owner;
        data << uint32(GetVirtualRealmAddress());
        data << uint32(GetVirtualRealmAddress());
    }

    return data;
}

WorldPacket const* WorldPackets::BattlePet::BattlePetJournal::Write()
{
    _worldPacket << uint16(Trap);
    _worldPacket << uint32(Slots.size());
    _worldPacket << uint32(Pets.size());

    for (BattlePetSlot const& slot : Slots)
        _worldPacket << slot;

    for (BattlePet const& pet : Pets)
        _worldPacket << pet;

    _worldPacket.WriteBit(HasJournalLock);
    _worldPacket.FlushBits();
    return &_worldPacket;
}

WorldPacket const* WorldPackets::BattlePet::BattlePetUpdates::Write()
{
    _worldPacket << uint32(Pets.size());
    for (BattlePet const& pet : Pets)
        _worldPacket << pet;

    _worldPacket.WriteBit(PetAdded);
    _worldPacket.FlushBits();
    return &_worldPacket;
}

WorldPacket const* WorldPackets::BattlePet::BattlePetTrapLevel::Write()
{
    _worldPacket << int16(TrapLevel);
    return &_worldPacket;
}

WorldPacket const* WorldPackets::BattlePet::PetBattleSlotUpdates::Write()
{
    _worldPacket << uint32(Slots.size());
    for (BattlePetSlot const& slot : Slots)
        _worldPacket << slot;

    _worldPacket.WriteBit(NewSlot);
    _worldPacket.WriteBit(AutoSlotted);
    _worldPacket.FlushBits();
    return &_worldPacket;
}

void WorldPackets::BattlePet::BattlePetSetBattleSlot::Read()
{
    _worldPacket >> PetGuid;
    _worldPacket >> Slot;
}

void WorldPackets::BattlePet::BattlePetModifyName::Read()
{
    _worldPacket >> PetGuid;
    uint32 nameLength = _worldPacket.ReadBits(7);
    bool hasDeclinedNames = _worldPacket.ReadBit();
    Name = _worldPacket.ReadString(nameLength);

    if (hasDeclinedNames)
    {
        uint8 declinedNameLengths[MAX_DECLINED_NAME_CASES];
        for (uint8 i = 0; i < MAX_DECLINED_NAME_CASES; ++i)
            declinedNameLengths[i] = _worldPacket.ReadBits(7);

        for (uint8 i = 0; i < MAX_DECLINED_NAME_CASES; ++i)
            Declined.name[i] = _worldPacket.ReadString(declinedNameLengths[i]);
    }
}

void WorldPackets::BattlePet::BattlePetDeletePet::Read()
{
    _worldPacket >> PetGuid;
}

void WorldPackets::BattlePet::BattlePetSetFlags::Read()
{
    _worldPacket >> PetGuid;
    _worldPacket >> Flags;
    ControlType = _worldPacket.ReadBits(2);
}

void WorldPackets::BattlePet::CageBattlePet::Read()
{
    _worldPacket >> PetGuid;
}

WorldPacket const* WorldPackets::BattlePet::BattlePetDeleted::Write()
{
    _worldPacket << PetGuid;
    return &_worldPacket;
}

WorldPacket const* WorldPackets::BattlePet::BattlePetError::Write()
{
    _worldPacket.WriteBits(Result, 4);
    _worldPacket.FlushBits();
    _worldPacket << uint32(CreatureID);
    return &_worldPacket;
}

void WorldPackets::BattlePet::BattlePetSummon::Read()
{
    _worldPacket >> PetGuid;
}

void WorldPackets::BattlePet::Query::Read()
{
    _worldPacket >> BattlePetID;
    _worldPacket >> UnitGUID;
}

WorldPacket const* WorldPackets::BattlePet::QueryResponse::Write()
{
    _worldPacket << BattlePetID;
    _worldPacket << uint32(CreatureID);
    _worldPacket << uint32(Timestamp);
    _worldPacket.WriteBit(Allow);
    _worldPacket.FlushBits();

    if (!Allow)
        return &_worldPacket;

    _worldPacket.WriteBits(Name.size(), 8);
    _worldPacket.WriteBit(HasDeclined);

    for (uint8 i = 0; i < MAX_DECLINED_NAME_CASES; ++i)
        _worldPacket.WriteBits(DeclinedNames[i].size(), 7);

    _worldPacket.FlushBits();
    _worldPacket.WriteString(Name);

    for (uint8 i = 0; i < MAX_DECLINED_NAME_CASES; ++i)
        _worldPacket.WriteString(DeclinedNames[i]);

    return &_worldPacket;
}

WorldPacket const* WorldPackets::BattlePet::GuidData::Write()
{
    _worldPacket << BattlePetGUID;
    return &_worldPacket;
}

ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::BattlePet::BattlePetAbility const& ability)
{
    data << int32(ability.AbilityID);
    data << int16(ability.CooldownRemaining);
    data << int16(ability.LockdownRemaining);
    data << uint8(ability.AbilityIndex);
    data << uint8(ability.Pboid);
    return data;
}

ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::BattlePet::BattlePetAura const& aura)
{
    data << int32(aura.AbilityID);
    data << uint32(aura.InstanceID);
    data << int32(aura.RoundsRemaining);
    data << int32(aura.CurrentRound);
    data << uint8(aura.CasterPBOID);
    return data;
}

ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::BattlePet::EffectTarget const& effectTarget)
{
    data.WriteBits(effectTarget.Type, 3);
    data.FlushBits();
    data << uint8(effectTarget.Petx);

    // WoD 6.2.4 target types (not Legion 7.x remapping)
    switch (effectTarget.Type)
    {
        case 1: // PETBATTLE_EVENT_UPDATE_BUFF
            data << uint32(effectTarget.AuraInstanceID);
            data << uint32(effectTarget.AuraAbilityID);
            data << int32(effectTarget.RoundsRemaining);
            data << uint32(effectTarget.CurrentRound);
            break;
        case 2: // PETBATTLE_EVENT_UPDATE_STATE
            data << uint32(effectTarget.StateID);
            data << int32(effectTarget.StateValue);
            break;
        case 3: // PETBATTLE_EVENT_UPDATE_HEALTH
            data << int32(effectTarget.Health);
            break;
        case 4: // PETBATTLE_EVENT_UPDATE_SPEED
            data << int32(effectTarget.NewStatValue);
            break;
        case 5: // PETBATTLE_EVENT_UPDATE_TRIGGER
            data << uint32(effectTarget.TriggerAbilityID);
            break;
        case 6: // PETBATTLE_EVENT_UPDATE_ABILITY_CHANGE
            data << int32(effectTarget.ChangedAbilityID);
            data << int32(effectTarget.CooldownRemaining);
            data << int32(effectTarget.LockdownRemaining);
            break;
        case 7: // PETBATTLE_EVENT_UPDATE_NPC_EMOTE
            data << int32(effectTarget.BroadcastTextID);
            break;
        default:
            break;
    }

    return data;
}

ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::BattlePet::Effect const& effect)
{
    data << uint32(effect.AbilityEffectID);
    data << uint16(effect.Flags);
    data << uint16(effect.SourceAuraInstanceID);
    data << uint16(effect.TurnInstanceID);
    data << uint8(effect.EffectType);
    data << uint8(effect.CasterPBOID);
    data << uint8(effect.StackDepth);
    data << uint32(effect.EffectTargetData.size());
    for (WorldPackets::BattlePet::EffectTarget const& target : effect.EffectTargetData)
        data << target;
    return data;
}

ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::BattlePet::RoundResult const& roundResult)
{
    data << uint32(roundResult.CurRound);
    data << uint8(roundResult.NextPetBattleState);
    data << uint32(roundResult.EffectData.size());

    for (uint8 i = 0; i < PET_BATTLE_TEAM_COUNT; ++i)
    {
        data << uint8(roundResult.NextInputFlags[i]);
        data << uint8(roundResult.NextTrapStatus[i]);
        data << uint16(roundResult.RoundTimeSecs[i]);
    }

    data << uint32(roundResult.Ability.size());

    for (WorldPackets::BattlePet::Effect const& effect : roundResult.EffectData)
        data << effect;

    for (WorldPackets::BattlePet::BattlePetAbility const& ability : roundResult.Ability)
        data << ability;

    data.WriteBits(roundResult.PetXDied.size(), 3);
    data.FlushBits();

    for (int8 pet : roundResult.PetXDied)
        data << uint8(pet);

    return data;
}

ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::BattlePet::FinalPet const& finalPet)
{
    data << finalPet.Guid;
    data << uint16(finalPet.Level);
    data << uint16(finalPet.Xp);
    data << int32(finalPet.Health);
    data << int32(finalPet.MaxHealth);
    data << uint16(finalPet.InitialLevel);
    data << uint8(finalPet.Pboid);
    data.WriteBit(finalPet.Captured);
    data.WriteBit(finalPet.Caged);
    data.WriteBit(finalPet.AwardedXP);
    data.WriteBit(finalPet.SeenAction);
    data.FlushBits();
    return data;
}

ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::BattlePet::FinalRound const& finalRound)
{
    data.WriteBit(finalRound.Abandoned);
    data.WriteBit(finalRound.PvpBattle);
    for (uint8 i = 0; i < PET_BATTLE_TEAM_COUNT; ++i)
        data.WriteBit(finalRound.Winner[i]);
    data.FlushBits();

    for (uint8 i = 0; i < PET_BATTLE_TEAM_COUNT; ++i)
        data << uint32(finalRound.NpcCreatureID[i]);

    data << uint32(finalRound.Pets.size());
    for (WorldPackets::BattlePet::FinalPet const& pet : finalRound.Pets)
        data << pet;

    return data;
}

WorldPacket const* WorldPackets::BattlePet::BattleRound::Write()
{
    _worldPacket << MsgData;
    return &_worldPacket;
}

WorldPacket const* WorldPackets::BattlePet::SceneObjectFinalRound::Write()
{
    _worldPacket << MsgData;
    return &_worldPacket;
}

ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::BattlePet::Locations& locations)
{
    data << int32(locations.LocationResult);
    data << locations.BattleOrigin.PositionXYZStream();
    data << float(locations.BattleFacing);
    for (uint8 i = 0; i < PET_BATTLE_TEAM_COUNT; ++i)
        data << locations.PlayerPositions[i].PositionXYZStream();
    return data;
}

ByteBuffer& operator>>(ByteBuffer& data, WorldPackets::BattlePet::Locations& locations)
{
    data >> locations.LocationResult;
    data >> locations.BattleOrigin.PositionXYZStream();
    data >> locations.BattleFacing;
    for (uint8 i = 0; i < PET_BATTLE_TEAM_COUNT; ++i)
        data >> locations.PlayerPositions[i].PositionXYZStream();
    return data;
}

WorldPacket const* WorldPackets::BattlePet::FinalizeLocation::Write()
{
    _worldPacket << Location;
    return &_worldPacket;
}

WorldPacket const* WorldPackets::BattlePet::PVPChallenge::Write()
{
    _worldPacket << ChallengerGUID;
    _worldPacket << Location;
    return &_worldPacket;
}

void WorldPackets::BattlePet::RequestWild::Read()
{
    _worldPacket >> Battle.TargetGUID;
    _worldPacket >> Battle.Location;
}

void WorldPackets::BattlePet::RequestPVP::Read()
{
    _worldPacket >> Battle.TargetGUID;
    _worldPacket >> Battle.Location;
}

WorldPacket const* WorldPackets::BattlePet::RequestFailed::Write()
{
    _worldPacket << uint8(Reason);
    return &_worldPacket;
}

void WorldPackets::BattlePet::ReplaceFrontPet::Read()
{
    _worldPacket >> FrontPet;
}

WorldPacket const* WorldPackets::BattlePet::PetBattleQueueStatus::Write()
{
    _worldPacket << uint32(Msg.Status);
    _worldPacket << uint32(Msg.SlotResult.size());
    _worldPacket << Msg.Ticket;
    for (int32 result : Msg.SlotResult)
        _worldPacket << int32(result);

    _worldPacket.WriteBit(Msg.AverageWaitTime.is_initialized());
    _worldPacket.WriteBit(Msg.ClientWaitTime.is_initialized());
    _worldPacket.FlushBits();

    if (Msg.AverageWaitTime)
        _worldPacket << int32(*Msg.AverageWaitTime);

    if (Msg.ClientWaitTime)
        _worldPacket << int32(*Msg.ClientWaitTime);

    return &_worldPacket;
}

void WorldPackets::BattlePet::QueueProposeMatchResult::Read()
{
    Accepted = _worldPacket.ReadBit();
}

void WorldPackets::BattlePet::LeaveQueue::Read()
{
    _worldPacket >> Ticket;
}

void WorldPackets::BattlePet::RequestUpdate::Read()
{
    _worldPacket >> TargetGUID;
    Canceled = _worldPacket.ReadBit();
}

ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::BattlePet::PetBattlePetUpdate const& update)
{
    data << update.BattlePetGUID;
    data << uint32(update.SpeciesID);
    data << uint32(update.DisplayID);
    data << uint32(update.CollarID);
    data << uint16(update.Level);
    data << uint16(update.Xp);
    data << int32(update.CurHealth);
    data << int32(update.MaxHealth);
    data << int32(update.Power);
    data << int32(update.Speed);
    data << uint32(update.NpcTeamMemberID);
    data << uint16(update.BreedQuality);
    data << uint16(update.StatusFlags);
    data << int8(update.Slot);
    data << uint32(update.Abilities.size());
    data << uint32(update.Auras.size());
    data << uint32(update.States.size());

    for (WorldPackets::BattlePet::BattlePetAbility const& ability : update.Abilities)
        data << ability;

    for (WorldPackets::BattlePet::BattlePetAura const& aura : update.Auras)
        data << aura;

    for (std::pair<uint32, int32> const& state : update.States)
    {
        data << uint32(state.first);
        data << int32(state.second);
    }

    data.WriteBits(update.CustomName.size(), 7);
    data.FlushBits();
    data.WriteString(update.CustomName);
    return data;
}

ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::BattlePet::PetBattlePlayerUpdate const& update)
{
    data << update.CharacterID;
    data << int32(update.TrapAbilityID);
    data << int32(update.TrapStatus);
    data << uint16(update.RoundTimeSecs);
    data << int8(update.FrontPet);
    data << uint8(update.InputFlags);
    data.WriteBits(update.Pets.size(), 2);
    data.FlushBits();
    for (WorldPackets::BattlePet::PetBattlePetUpdate const& pet : update.Pets)
        data << pet;
    return data;
}

ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::BattlePet::PetBattleEnviroUpdate const& update)
{
    data << uint32(update.Auras.size());
    data << uint32(update.States.size());
    for (WorldPackets::BattlePet::BattlePetAura const& aura : update.Auras)
        data << aura;
    for (std::pair<uint32, int32> const& state : update.States)
    {
        data << uint32(state.first);
        data << int32(state.second);
    }
    return data;
}

ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::BattlePet::PetBattleFullUpdate const& update)
{
    for (uint8 i = 0; i < PET_BATTLE_TEAM_COUNT; ++i)
        data << update.Players[i];

    for (uint8 i = 0; i < 3; ++i)
        data << update.Enviros[i];

    data << uint16(update.WaitingForFrontPetsMaxSecs);
    data << uint16(update.PvpMaxRoundTime);
    data << int32(update.CurRound);
    data << uint32(update.NpcCreatureID);
    data << uint32(update.NpcDisplayID);
    data << int8(update.CurPetBattleState);
    data << uint8(update.ForfeitPenalty);
    data << update.InitialWildPetGUID;
    data.WriteBit(update.IsPVP);
    data.WriteBit(update.CanAwardXP);
    data.FlushBits();
    return data;
}

WorldPacket const* WorldPackets::BattlePet::PetBattleInitialUpdate::Write()
{
    _worldPacket << MsgData;
    return &_worldPacket;
}

void WorldPackets::BattlePet::PetBattleInput::Read()
{
    _worldPacket >> MoveType;
    _worldPacket >> NewFrontPet;
    _worldPacket >> DebugFlags;
    _worldPacket >> BattleInterrupted;
    _worldPacket >> AbilityID;
    _worldPacket >> Round;
    IgnoreAbandonPenalty = _worldPacket.ReadBit();
}
