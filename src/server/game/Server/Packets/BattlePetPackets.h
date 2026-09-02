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
 * with this program. If not, see <http://www.trinitycore.org/>.
 */

#ifndef BattlePetPackets_h__
#define BattlePetPackets_h__

#include "Packet.h"
#include "ObjectGuid.h"
#include "Object.h"
#include "LFGPackets.h"
#include "Unit.h"
#include <list>
#include <unordered_map>
#include <vector>

enum { PET_BATTLE_TEAM_COUNT = 2 };

namespace WorldPackets
{
    namespace BattlePet
    {
        struct BattlePet
        {
            ObjectGuid Guid;
            uint32 Species = 0;
            uint32 CreatureID = 0;
            uint32 DisplayID = 0;
            uint16 Breed = 0;
            uint16 Level = 0;
            uint16 Exp = 0;
            uint16 Flags = 0;
            uint32 Power = 0;
            uint32 Health = 0;
            uint32 MaxHealth = 0;
            uint32 Speed = 0;
            uint8 Quality = 0;
            ObjectGuid Owner;
            std::string Name;
        };

        struct BattlePetSlot
        {
            BattlePet Pet;
            uint32 CollarID = 0;
            uint8 Index = 0;
            bool Locked = true;
        };

        class BattlePetJournal final : public ServerPacket
        {
        public:
            BattlePetJournal() : ServerPacket(SMSG_BATTLE_PET_JOURNAL) { }

            WorldPacket const* Write() override;

            uint16 Trap = 0;
            std::vector<BattlePetSlot> Slots;
            std::vector<BattlePet> Pets;
            bool HasJournalLock = true;
        };

        class BattlePetJournalLockAcquired final : public ServerPacket
        {
        public:
            BattlePetJournalLockAcquired() : ServerPacket(SMSG_BATTLE_PET_JOURNAL_LOCK_ACQUIRED, 0) { }

            WorldPacket const* Write() override { return &_worldPacket; }
        };

        class BattlePetJournalLockDenied final : public ServerPacket
        {
        public:
            BattlePetJournalLockDenied() : ServerPacket(SMSG_BATTLE_PET_JOURNAL_LOCK_DENIED, 0) { }

            WorldPacket const* Write() override { return &_worldPacket; }
        };

        class BattlePetRequestJournal final : public ClientPacket
        {
        public:
            BattlePetRequestJournal(WorldPacket&& packet) : ClientPacket(CMSG_BATTLE_PET_REQUEST_JOURNAL, std::move(packet)) { }

            void Read() override { }
        };

        class BattlePetUpdates final : public ServerPacket
        {
        public:
            BattlePetUpdates() : ServerPacket(SMSG_BATTLE_PET_UPDATES) { }

            WorldPacket const* Write() override;

            std::vector<BattlePet> Pets;
            bool PetAdded = false;
        };

        class BattlePetTrapLevel final : public ServerPacket
        {
        public:
            BattlePetTrapLevel() : ServerPacket(SMSG_BATTLE_PET_TRAP_LEVEL, 2) { }

            WorldPacket const* Write() override;

            int16 TrapLevel = 0;
        };

        class PetBattleSlotUpdates final : public ServerPacket
        {
        public:
            PetBattleSlotUpdates() : ServerPacket(SMSG_PET_BATTLE_SLOT_UPDATES) { }

            WorldPacket const* Write() override;

            std::vector<BattlePetSlot> Slots;
            bool AutoSlotted = false;
            bool NewSlot = false;
        };

        class BattlePetSetBattleSlot final : public ClientPacket
        {
        public:
            BattlePetSetBattleSlot(WorldPacket&& packet) : ClientPacket(CMSG_BATTLE_PET_SET_BATTLE_SLOT, std::move(packet)) { }

            void Read() override;

            ObjectGuid PetGuid;
            uint8 Slot = 0;
        };

        class BattlePetModifyName final : public ClientPacket
        {
        public:
            BattlePetModifyName(WorldPacket&& packet) : ClientPacket(CMSG_BATTLE_PET_MODIFY_NAME, std::move(packet)) { }

            void Read() override;

            ObjectGuid PetGuid;
            std::string Name;
            DeclinedName Declined;
        };

        class BattlePetDeletePet final : public ClientPacket
        {
        public:
            BattlePetDeletePet(WorldPacket&& packet) : ClientPacket(CMSG_BATTLE_PET_DELETE_PET, std::move(packet)) { }

            void Read() override;

            ObjectGuid PetGuid;
        };

        class BattlePetSetFlags final : public ClientPacket
        {
        public:
            BattlePetSetFlags(WorldPacket&& packet) : ClientPacket(CMSG_BATTLE_PET_SET_FLAGS, std::move(packet)) { }

            void Read() override;

            ObjectGuid PetGuid;
            uint32 Flags = 0;
            uint8 ControlType = 0;
        };

        class CageBattlePet final : public ClientPacket
        {
        public:
            CageBattlePet(WorldPacket&& packet) : ClientPacket(CMSG_CAGE_BATTLE_PET, std::move(packet)) { }

            void Read() override;

            ObjectGuid PetGuid;
        };

        class BattlePetDeleted final : public ServerPacket
        {
        public:
            BattlePetDeleted() : ServerPacket(SMSG_BATTLE_PET_DELETED, 16) { }

            WorldPacket const* Write() override;

            ObjectGuid PetGuid;
        };

        class BattlePetError final : public ServerPacket
        {
        public:
            BattlePetError() : ServerPacket(SMSG_BATTLE_PET_ERROR, 5) { }

            WorldPacket const* Write() override;

            uint8 Result = 0;
            uint32 CreatureID = 0;
        };

        class BattlePetSummon final : public ClientPacket
        {
        public:
            BattlePetSummon(WorldPacket&& packet) : ClientPacket(CMSG_BATTLE_PET_SUMMON, std::move(packet)) { }

            void Read() override;

            ObjectGuid PetGuid;
        };

        class Query final : public ClientPacket
        {
        public:
            Query(WorldPacket&& packet) : ClientPacket(CMSG_QUERY_BATTLE_PET_NAME, std::move(packet)) { }

            void Read() override;

            ObjectGuid BattlePetID;
            ObjectGuid UnitGUID;
        };

        class QueryResponse final : public ServerPacket
        {
        public:
            QueryResponse() : ServerPacket(SMSG_QUERY_BATTLE_PET_NAME_RESPONSE, 16 + 4 + 4 + 1) { }

            WorldPacket const* Write() override;

            ObjectGuid BattlePetID;
            uint32 CreatureID = 0;
            uint32 Timestamp = 0;
            bool Allow = false;
            std::string Name;
            std::string DeclinedNames[MAX_DECLINED_NAME_CASES] = { };
            bool HasDeclined = false;
        };

        class GuidData final : public ServerPacket
        {
        public:
            GuidData(OpcodeServer opcode) : ServerPacket(opcode, 16) { }

            WorldPacket const* Write() override;

            ObjectGuid BattlePetGUID;
        };

        struct BattlePetAbility
        {
            int32 AbilityID = 0;
            int16 CooldownRemaining = 0;
            int16 LockdownRemaining = 0;
            uint8 AbilityIndex = 0;
            uint8 Pboid = 0;
        };

        struct BattlePetAura
        {
            int32 AbilityID = 0;
            uint32 InstanceID = 0;
            int32 RoundsRemaining = 0;
            int32 CurrentRound = 0;
            uint8 CasterPBOID = 0;
        };

        struct EffectTarget
        {
            uint16 Type = 0;
            uint8 Petx = 0;
            uint32 TriggerAbilityID = 0;
            int32 NewStatValue = 0;
            int32 BroadcastTextID = 0;
            int32 Health = 0;
            uint32 AuraInstanceID = 0;
            uint32 AuraAbilityID = 0;
            int32 RoundsRemaining = 0;
            uint32 CurrentRound = 0;
            uint32 StateID = 0;
            int32 StateValue = 0;
            int32 ChangedAbilityID = 0;
            int32 CooldownRemaining = 0;
            int32 LockdownRemaining = 0;
        };

        struct Effect
        {
            uint32 AbilityEffectID = 0;
            uint16 Flags = 0;
            uint16 SourceAuraInstanceID = 0;
            uint16 TurnInstanceID = 0;
            uint8 EffectType = 0;
            uint8 CasterPBOID = 0;
            uint8 StackDepth = 0;
            std::vector<EffectTarget> EffectTargetData;
        };

        struct RoundResult
        {
            uint32 CurRound = 0;
            uint8 NextPetBattleState = 0;
            uint8 NextInputFlags[PET_BATTLE_TEAM_COUNT] = { };
            uint8 NextTrapStatus[PET_BATTLE_TEAM_COUNT] = { };
            uint16 RoundTimeSecs[PET_BATTLE_TEAM_COUNT] = { };
            std::vector<Effect> EffectData;
            std::vector<BattlePetAbility> Ability;
            std::vector<int8> PetXDied;
        };

        class BattleRound final : public ServerPacket
        {
        public:
            BattleRound(OpcodeServer opcode) : ServerPacket(opcode, 4 + 1 + 1 + 1 + 2 + 4 + 4 + 4) { }

            WorldPacket const* Write() override;

            RoundResult MsgData;
        };

        struct FinalPet
        {
            ObjectGuid Guid;
            uint16 Level = 0;
            uint16 Xp = 0;
            int32 Health = 0;
            int32 MaxHealth = 0;
            uint16 InitialLevel = 0;
            uint8 Pboid = 0;
            bool Captured = false;
            bool Caged = false;
            bool AwardedXP = false;
            bool SeenAction = false;
        };

        struct FinalRound
        {
            bool Abandoned = false;
            bool PvpBattle = false;
            bool Winner[PET_BATTLE_TEAM_COUNT] = { };
            uint32 NpcCreatureID[PET_BATTLE_TEAM_COUNT] = { };
            std::vector<FinalPet> Pets;
        };

        class SceneObjectFinalRound final : public ServerPacket
        {
        public:
            SceneObjectFinalRound() : ServerPacket(SMSG_PET_BATTLE_FINAL_ROUND, 1 + 1 + 4) { }

            WorldPacket const* Write() override;

            FinalRound MsgData;
        };

        struct Locations
        {
            int32 LocationResult = 0;
            Position BattleOrigin;
            float BattleFacing = 0.0f;
            Position PlayerPositions[PET_BATTLE_TEAM_COUNT];
        };

        class FinalizeLocation final : public ServerPacket
        {
        public:
            FinalizeLocation() : ServerPacket(SMSG_PET_BATTLE_FINALIZE_LOCATION, 4 + 12 + 4 + 12 * 2) { }

            WorldPacket const* Write() override;

            Locations Location;
        };

        class PVPChallenge final : public ServerPacket
        {
        public:
            PVPChallenge() : ServerPacket(SMSG_PET_BATTLE_PVP_CHALLENGE, 16 + 4 + 12 + 4 + 12 * 2) { }

            WorldPacket const* Write() override;

            ObjectGuid ChallengerGUID;
            Locations Location;
        };

        struct BattleRequest
        {
            ObjectGuid TargetGUID;
            Locations Location;
        };

        class RequestWild final : public ClientPacket
        {
        public:
            RequestWild(WorldPacket&& packet) : ClientPacket(CMSG_PET_BATTLE_REQUEST_WILD, std::move(packet)) { }

            void Read() override;

            BattleRequest Battle;
        };

        class RequestPVP final : public ClientPacket
        {
        public:
            RequestPVP(WorldPacket&& packet) : ClientPacket(CMSG_PET_BATTLE_REQUEST_PVP, std::move(packet)) { }

            void Read() override;

            BattleRequest Battle;
        };

        class RequestFailed final : public ServerPacket
        {
        public:
            RequestFailed() : ServerPacket(SMSG_PET_BATTLE_REQUEST_FAILED, 1) { }

            WorldPacket const* Write() override;

            uint8 Reason = 0;
        };

        class ReplaceFrontPet final : public ClientPacket
        {
        public:
            ReplaceFrontPet(WorldPacket&& packet) : ClientPacket(CMSG_PET_BATTLE_REPLACE_FRONT_PET, std::move(packet)) { }

            void Read() override;

            uint8 FrontPet = 0;
        };

        struct QueueStatus
        {
            uint32 Status = 0;
            std::vector<int32> SlotResult;
            Optional<int32> ClientWaitTime;
            Optional<int32> AverageWaitTime;
            WorldPackets::LFG::RideTicket Ticket;
        };

        class PetBattleQueueStatus final : public ServerPacket
        {
        public:
            PetBattleQueueStatus() : ServerPacket(SMSG_PET_BATTLE_QUEUE_STATUS, 4 + 4 + 1 + 1 + 28) { }

            WorldPacket const* Write() override;

            QueueStatus Msg;
        };

        class QueueProposeMatchResult final : public ClientPacket
        {
        public:
            QueueProposeMatchResult(WorldPacket&& packet) : ClientPacket(CMSG_PET_BATTLE_QUEUE_PROPOSE_MATCH_RESULT, std::move(packet)) { }

            void Read() override;

            bool Accepted = false;
        };

        class LeaveQueue final : public ClientPacket
        {
        public:
            LeaveQueue(WorldPacket&& packet) : ClientPacket(CMSG_LEAVE_PET_BATTLE_QUEUE, std::move(packet)) { }

            void Read() override;

            WorldPackets::LFG::RideTicket Ticket;
        };

        class RequestUpdate final : public ClientPacket
        {
        public:
            RequestUpdate(WorldPacket&& packet) : ClientPacket(CMSG_PET_BATTLE_REQUEST_UPDATE, std::move(packet)) { }

            void Read() override;

            ObjectGuid TargetGUID;
            bool Canceled = false;
        };

        struct PetBattlePetUpdate
        {
            ObjectGuid BattlePetGUID;
            uint32 SpeciesID = 0;
            uint32 DisplayID = 0;
            uint32 CollarID = 0;
            uint16 Level = 0;
            uint16 Xp = 0;
            int32 CurHealth = 0;
            int32 MaxHealth = 0;
            int32 Power = 0;
            int32 Speed = 0;
            uint32 NpcTeamMemberID = 0;
            uint16 BreedQuality = 0;
            uint16 StatusFlags = 0;
            int8 Slot = 0;
            std::string CustomName;
            std::vector<BattlePetAbility> Abilities;
            std::vector<BattlePetAura> Auras;
            std::vector<std::pair<uint32, int32>> States;
        };

        struct PetBattlePlayerUpdate
        {
            ObjectGuid CharacterID;
            int32 TrapAbilityID = 0;
            int32 TrapStatus = 0;
            uint16 RoundTimeSecs = 0;
            std::vector<PetBattlePetUpdate> Pets;
            int8 FrontPet = 0;
            uint8 InputFlags = 0;
        };

        struct PetBattleEnviroUpdate
        {
            std::vector<BattlePetAura> Auras;
            std::vector<std::pair<uint32, int32>> States;
        };

        struct PetBattleFullUpdate
        {
            PetBattlePlayerUpdate Players[PET_BATTLE_TEAM_COUNT];
            PetBattleEnviroUpdate Enviros[3];
            ObjectGuid InitialWildPetGUID;
            uint32 NpcCreatureID = 0;
            uint32 NpcDisplayID = 0;
            int32 CurRound = 0;
            uint16 WaitingForFrontPetsMaxSecs = 0;
            uint16 PvpMaxRoundTime = 0;
            uint8 ForfeitPenalty = 0;
            int8 CurPetBattleState = 0;
            bool IsPVP = false;
            bool CanAwardXP = false;
        };

        class PetBattleInitialUpdate final : public ServerPacket
        {
        public:
            PetBattleInitialUpdate() : ServerPacket(SMSG_PET_BATTLE_INITIAL_UPDATE, 124) { }

            WorldPacket const* Write() override;

            PetBattleFullUpdate MsgData;
        };

        class PetBattleInput final : public ClientPacket
        {
        public:
            PetBattleInput(WorldPacket&& packet) : ClientPacket(CMSG_PET_BATTLE_INPUT, std::move(packet)) { }

            void Read() override;

            int32 AbilityID = 0;
            int32 Round = 0;
            uint8 MoveType = 0;
            int8 NewFrontPet = 0;
            uint8 DebugFlags = 0;
            uint8 BattleInterrupted = 0;
            bool IgnoreAbandonPenalty = false;
        };
    }
}

ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::BattlePet::BattlePetSlot const& slot);
ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::BattlePet::BattlePet const& pet);
ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::BattlePet::BattlePetAbility const& ability);
ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::BattlePet::BattlePetAura const& aura);
ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::BattlePet::EffectTarget const& effectTarget);
ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::BattlePet::Effect const& effect);
ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::BattlePet::RoundResult const& roundResult);
ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::BattlePet::FinalPet const& finalPet);
ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::BattlePet::FinalRound const& finalRound);
ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::BattlePet::Locations& locations);
ByteBuffer& operator>>(ByteBuffer& data, WorldPackets::BattlePet::Locations& locations);
ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::BattlePet::PetBattlePetUpdate const& update);
ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::BattlePet::PetBattlePlayerUpdate const& update);
ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::BattlePet::PetBattleEnviroUpdate const& update);
ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::BattlePet::PetBattleFullUpdate const& update);

#endif // BattlePetPackets_h__
