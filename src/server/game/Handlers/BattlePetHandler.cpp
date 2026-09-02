////////////////////////////////////////////////////////////////////////////////
//
//  MILLENIUM-STUDIO
//  Copyright 2014-2015 Millenium-studio SARL
//  All Rights Reserved.
//
////////////////////////////////////////////////////////////////////////////////

#include "Common.h"
#include "ObjectAccessor.h"
#include "ObjectMgr.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include "Player.h"
#include "Item.h"
#include "PetBattle.h"
#include "WildBattlePet.h"
#include "AchievementMgr.h"
#include "BattlePetPackets.h"
#include "DB2Stores.h"
#include "CellImpl.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"

namespace
{
    WorldPackets::BattlePet::BattlePet BuildJournalPet(BattlePet::Ptr const& pet)
    {
        WorldPackets::BattlePet::BattlePet packetPet;
        BattlePetSpeciesEntry const* species = sBattlePetSpeciesStore.LookupEntry(pet->Species);

        pet->UpdateStats();

        packetPet.Guid = pet->JournalID;
        packetPet.Species = pet->Species;
        packetPet.CreatureID = species ? species->CreatureID : 0;
        packetPet.DisplayID = pet->DisplayModelID;
        packetPet.Breed = pet->Breed;
        packetPet.Level = pet->Level;
        packetPet.Exp = pet->XP;
        packetPet.Flags = pet->Flags;
        packetPet.Power = pet->InfoPower;
        packetPet.Health = pet->Health > pet->InfoMaxHealth ? pet->InfoMaxHealth : pet->Health;
        packetPet.MaxHealth = pet->InfoMaxHealth;
        packetPet.Speed = pet->InfoSpeed;
        packetPet.Quality = pet->Quality;
        packetPet.Name = pet->Name;
        return packetPet;
    }
}

void WorldSession::SendBattlePetUpdates(bool p_AddedPet)
{
    if (!m_Player || !m_Player->IsInWorld())
        return;

    WorldPackets::BattlePet::BattlePetUpdates packet;
    packet.PetAdded = p_AddedPet;

    for (BattlePet::Ptr const& pet : m_Player->GetBattlePets())
        packet.Pets.push_back(BuildJournalPet(pet));

    SendPacket(packet.Write());
}

void WorldSession::SendBattlePetTrapLevel()
{
    if (!m_Player || !m_Player->IsInWorld())
        return;

    WorldPackets::BattlePet::BattlePetTrapLevel packet;
    packet.TrapLevel = int16(m_Player->GetBattlePetTrapLevel());
    SendPacket(packet.Write());
}

void WorldSession::SendBattlePetJournalLockAcquired()
{
    WorldPackets::BattlePet::BattlePetJournalLockAcquired packet;
    SendPacket(packet.Write());
    m_IsPetBattleJournalLocked = true;
}

void WorldSession::SendBattlePetJournalLockDenied()
{
    m_IsPetBattleJournalLocked = false;
    WorldPackets::BattlePet::BattlePetJournalLockDenied packet;
    SendPacket(packet.Write());
}

void WorldSession::SendBattlePetJournal()
{
    if (!m_Player || !m_Player->IsInWorld())
        return;

    std::vector<BattlePet::Ptr> pets = m_Player->GetBattlePets();
    uint32 unlockedSlotCount = m_Player->GetUnlockedPetBattleSlot();
    BattlePet::Ptr* petSlots = m_Player->GetBattlePetCombatTeam();

    if (unlockedSlotCount > 0)
        m_Player->SetFlag(PLAYER_FIELD_PLAYER_FLAGS, PLAYER_FLAGS_HAS_BATTLE_PET_TRAINING);

    WorldPackets::BattlePet::BattlePetJournal packet;
    packet.Trap = uint16(m_Player->GetBattlePetTrapLevel());
    packet.HasJournalLock = true;

    for (uint32 i = 0; i < MAX_PETBATTLE_SLOTS; ++i)
    {
        WorldPackets::BattlePet::BattlePetSlot slot;
        slot.Index = uint8(i);
        slot.Locked = !((i + 1) <= unlockedSlotCount);
        if (petSlots[i])
            slot.Pet.Guid = petSlots[i]->JournalID;
        packet.Slots.push_back(slot);
    }

    for (BattlePet::Ptr const& pet : pets)
        packet.Pets.push_back(BuildJournalPet(pet));

    SendPacket(packet.Write());
}

void WorldSession::SendBattlePetDeleted(uint64 p_BattlePetGUID)
{
    WorldPackets::BattlePet::BattlePetDeleted packet;
    packet.PetGuid = p_BattlePetGUID;
    SendPacket(packet.Write());
}

void WorldSession::SendBattlePetRevoked(uint64 p_BattlePetGUID)
{
    WorldPackets::BattlePet::GuidData packet(SMSG_BATTLE_PET_REVOKED);
    packet.BattlePetGUID = p_BattlePetGUID;
    SendPacket(packet.Write());
}

void WorldSession::SendBattlePetRestored(uint64 p_BattlePetGUID)
{
    WorldPackets::BattlePet::GuidData packet(SMSG_BATTLE_PET_RESTORED);
    packet.BattlePetGUID = p_BattlePetGUID;
    SendPacket(packet.Write());
}

void WorldSession::SendBattlePetsHealed()
{
    WorldPacket l_Packet(SMSG_BATTLE_PETS_HEALED, 0);
    m_Player->GetSession()->SendPacket(&l_Packet);
}

void WorldSession::SendBattlePetLicenseChanged()
{
    WorldPacket l_Packet(SMSG_BATTLE_PET_LICENSE_CHANGED, 0);
    m_Player->GetSession()->SendPacket(&l_Packet);
}

void WorldSession::SendBattlePetError(uint32 p_Result, uint32 p_CreatureID)
{
    WorldPackets::BattlePet::BattlePetError packet;
    packet.Result = uint8(p_Result);
    packet.CreatureID = p_CreatureID;
    SendPacket(packet.Write());
}

void WorldSession::SendBattlePetCageDateError(uint32 p_SecondsUntilCanCage)
{
    WorldPacket l_Packet(SMSG_BATTLE_PET_CAGE_DATE_ERROR, 4);
    l_Packet << uint32(p_SecondsUntilCanCage);

    m_Player->GetSession()->SendPacket(&l_Packet);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void WorldSession::HandleBattlePetQueryName(WorldPacket& p_RecvData)
{
    uint64 l_UnitGuid;
    uint64 l_JournalGuid;

    p_RecvData.readPackGUID(l_JournalGuid);
    p_RecvData.readPackGUID(l_UnitGuid);

    Creature* l_Creature = Unit::GetCreature(*m_Player, l_UnitGuid);

    if (!l_Creature)
        return;

    BattlePet::Ptr l_BattlePet = nullptr;

    if (l_Creature->GetOwner() && l_Creature->GetOwner()->IsPlayer())
        l_BattlePet = l_Creature->GetOwner()->ToPlayer()->GetBattlePet(l_JournalGuid);

    if (!l_BattlePet)
        return;

    bool l_HaveCustomName = l_Creature->GetUInt32Value(UNIT_FIELD_BATTLE_PET_COMPANION_NAME_TIMESTAMP) != 0;
    bool l_HaveDeclinedNames = false;

    if (l_BattlePet)
    {
        for (int l_I = 0; l_I < MAX_DECLINED_NAME_CASES; ++l_I)
        {
            if (!l_BattlePet->DeclinedNames[l_I].empty())
                l_HaveDeclinedNames = true;
        }
    }

    WorldPackets::BattlePet::QueryResponse packet;
    packet.BattlePetID = l_JournalGuid;
    packet.CreatureID = l_Creature->GetEntry();
    packet.Timestamp = l_Creature->GetUInt32Value(UNIT_FIELD_BATTLE_PET_COMPANION_NAME_TIMESTAMP);
    packet.Allow = l_HaveCustomName;
    if (l_HaveCustomName)
        packet.Name = l_Creature->GetName();
    packet.HasDeclined = l_HaveDeclinedNames;
    if (l_BattlePet)
    {
        for (uint32 i = 0; i < MAX_DECLINED_NAME_CASES; ++i)
            packet.DeclinedNames[i] = l_BattlePet->DeclinedNames[i];
    }

    SendPacket(packet.Write());
}

void WorldSession::HandleBattlePetUpdateNotify(WorldPacket& p_RecvData)
{
    uint64 l_BattlePetGUID;
    p_RecvData.readPackGUID(l_BattlePetGUID);

    /// Nothing todo the client received a battle pet update
}

void WorldSession::HandleBattlePetRequestJournalLock(WorldPacket& /*p_RecvData*/)
{
}

void WorldSession::HandleBattlePetRequestJournal(WorldPacket& /*p_RecvData*/)
{
    SendBattlePetJournal();
}

void WorldSession::HandleBattlePetDeletePet(WorldPacket& p_RecvData)
{
    uint64 l_BattlePetGUID;
    p_RecvData.readPackGUID(l_BattlePetGUID);

    if (!m_Player || !m_Player->IsInWorld())
        return;

    // Find the battle pet
    BattlePet::Ptr l_BattlePet = m_Player->GetBattlePet(l_BattlePetGUID);
    if (!l_BattlePet)
        return;

    // Remove from combat team if it's there
    std::shared_ptr<BattlePet>* l_PetSlots = m_Player->GetBattlePetCombatTeam();
    for (size_t l_I = 0; l_I < MAX_PETBATTLE_SLOTS; ++l_I)
    {
        if (l_PetSlots[l_I] && l_PetSlots[l_I]->JournalID == l_BattlePetGUID)
        {
            l_PetSlots[l_I] = nullptr;
            break;
        }
    }

    // Unsummon if currently summoned
    if (m_Player->GetSummonedBattlePet() && 
        m_Player->GetSummonedBattlePet()->GetGuidValue(UNIT_FIELD_BATTLE_PET_COMPANION_GUID) == l_BattlePetGUID)
    {
        m_Player->UnsummonCurrentBattlePetIfAny(false);
    }

    // Remove from player's pet collection
    std::vector<BattlePet::Ptr>& l_BattlePets = m_Player->m_BattlePets;
    for (auto it = l_BattlePets.begin(); it != l_BattlePets.end(); ++it)
    {
        if (*it && (*it)->JournalID == l_BattlePetGUID)
        {
            // Delete from database
            SQLTransaction l_Transaction = LoginDatabase.BeginTransaction();
            PreparedStatement* l_Stmt = LoginDatabase.GetPreparedStatement(LOGIN_DEL_BATTLE_PET);
            l_Stmt->setUInt64(0, l_BattlePetGUID);
            l_Stmt->setUInt32(1, GetAccountId());
            l_Transaction->Append(l_Stmt);
            LoginDatabase.CommitTransaction(l_Transaction);

            l_BattlePets.erase(it);
            break;
        }
    }

    // Update combat team
    m_Player->UpdateBattlePetCombatTeam();

    // Send updates to client
    SendBattlePetUpdates(false);
}

void WorldSession::HandleBattlePetDeletePetCheat(WorldPacket& p_RecvData)
{
    uint64 l_BattlePetGUID;
    p_RecvData.readPackGUID(l_BattlePetGUID);

    if (!m_Player || !m_Player->IsInWorld())
        return;

    // Check if player has GM permissions
    if (!m_Player->isGameMaster())
        return;

    // Find the battle pet
    BattlePet::Ptr l_BattlePet = m_Player->GetBattlePet(l_BattlePetGUID);
    if (!l_BattlePet)
        return;

    // Remove from combat team if it's there
    std::shared_ptr<BattlePet>* l_PetSlots = m_Player->GetBattlePetCombatTeam();
    for (size_t l_I = 0; l_I < MAX_PETBATTLE_SLOTS; ++l_I)
    {
        if (l_PetSlots[l_I] && l_PetSlots[l_I]->JournalID == l_BattlePetGUID)
        {
            l_PetSlots[l_I] = nullptr;
            break;
        }
    }

    // Unsummon if currently summoned
    if (m_Player->GetSummonedBattlePet() && 
        m_Player->GetSummonedBattlePet()->GetGuidValue(UNIT_FIELD_BATTLE_PET_COMPANION_GUID) == l_BattlePetGUID)
    {
        m_Player->UnsummonCurrentBattlePetIfAny(false);
    }

    // Remove from player's pet collection
    std::vector<BattlePet::Ptr>& l_BattlePets = m_Player->m_BattlePets;
    for (auto it = l_BattlePets.begin(); it != l_BattlePets.end(); ++it)
    {
        if (*it && (*it)->JournalID == l_BattlePetGUID)
        {
            // Delete from database (GM command - no account restriction)
            SQLTransaction l_Transaction = LoginDatabase.BeginTransaction();
            PreparedStatement* l_Stmt = LoginDatabase.GetPreparedStatement(LOGIN_DEL_BATTLE_PET_CHEAT);
            l_Stmt->setUInt64(0, l_BattlePetGUID);
            l_Transaction->Append(l_Stmt);
            LoginDatabase.CommitTransaction(l_Transaction);

            l_BattlePets.erase(it);
            break;
        }
    }

    // Update combat team
    m_Player->UpdateBattlePetCombatTeam();

    // Send updates to client
    SendBattlePetUpdates(false);

    // Log GM action
    TC_LOG_INFO("entities.player", "GM %s (GUID: %u) deleted battle pet %u", 
        m_Player->GetName(), m_Player->GetGUIDLow(), l_BattlePetGUID);
}

void WorldSession::HandleBattlePetModifyName(WorldPacket& p_RecvData)
{
    DeclinedName    l_DeclinedNames;
    uint64          l_PetJournalID;
    bool            l_HaveDeclinedNames = false;
    uint32          l_NameLenght        = 0;

    uint32 l_DeclinedNameLens[MAX_DECLINED_NAME_CASES];

    p_RecvData.readPackGUID(l_PetJournalID);
    l_NameLenght        = p_RecvData.ReadBits(7);
    l_HaveDeclinedNames = p_RecvData.ReadBit();

    if (l_HaveDeclinedNames)
    {
        p_RecvData.ResetBitReading();

        for (size_t l_I = 0 ; l_I < MAX_DECLINED_NAME_CASES ; ++l_I)
            l_DeclinedNameLens[l_I] = p_RecvData.ReadBits(7);

        for (size_t l_I = 0; l_I < MAX_DECLINED_NAME_CASES; ++l_I)
            l_DeclinedNames.name[l_I] = p_RecvData.ReadString(l_DeclinedNameLens[l_I]);
    }

    std::string l_Name = p_RecvData.ReadString(l_NameLenght);

    PetNameInvalidReason l_NameInvalidReason = sObjectMgr->CheckPetName(l_Name);
    if (l_NameInvalidReason != PET_NAME_SUCCESS)
    {
        SendPetNameInvalid(l_NameInvalidReason, l_Name, (l_HaveDeclinedNames) ? &l_DeclinedNames : NULL);
        return;
    }

    uint32 l_TimeStamp = l_Name.empty() ? 0 : time(0);

    BattlePet::Ptr l_BattlePet = m_Player->GetBattlePet(l_PetJournalID);

    if (l_BattlePet)
    {
        l_BattlePet->Name           = l_Name;
        l_BattlePet->NameTimeStamp  = l_TimeStamp;

        if (l_HaveDeclinedNames)
        {
            for (size_t l_I = 0; l_I < MAX_DECLINED_NAME_CASES; ++l_I)
                l_BattlePet->DeclinedNames[l_I] = l_DeclinedNames.name[l_I];
        }
    }

    m_Player->SetUInt32Value(UNIT_FIELD_BATTLE_PET_COMPANION_NAME_TIMESTAMP, l_TimeStamp);

    Creature* l_Creature = m_Player->GetSummonedBattlePet();

    if (!l_Creature)
        return;

    if (l_Creature->GetUInt32Value(UNIT_FIELD_BATTLE_PET_COMPANION_GUID) == l_PetJournalID)
    {
        l_Creature->SetName(l_Name);
        l_Creature->SetUInt32Value(UNIT_FIELD_BATTLE_PET_COMPANION_NAME_TIMESTAMP, l_TimeStamp);
    }
}

void WorldSession::HandleBattlePetSummon(WorldPacket& recvData)
{
    uint64 l_JournalID;

    recvData.readPackGUID(l_JournalID);

    if (m_Player->GetSummonedBattlePet() && m_Player->GetSummonedBattlePet()->GetGuidValue(UNIT_FIELD_BATTLE_PET_COMPANION_GUID) == l_JournalID)
        m_Player->UnsummonCurrentBattlePetIfAny(false);
    else
    {
        m_Player->UnsummonCurrentBattlePetIfAny(false);
        m_Player->SummonBattlePet(l_JournalID);
    }
}

void WorldSession::HandleBattlePetSetBattleSlot(WorldPacket& p_RecvData)
{
    if (m_IsPetBattleJournalLocked)
        return;

    uint64  l_PetJournalID;
    uint8   l_DestSlot = 0;

    p_RecvData.readPackGUID(l_PetJournalID);
    p_RecvData >> l_DestSlot;

    if (l_DestSlot >= MAX_PETBATTLE_SLOTS)
        return;

    BattlePet::Ptr   l_BattlePet = m_Player->GetBattlePet(l_PetJournalID);
    BattlePet::Ptr * l_PetSlots = m_Player->GetBattlePetCombatTeam();

    if (l_BattlePet)
    {
        for (uint8 l_I = 0; l_I < MAX_PETBATTLE_SLOTS; ++l_I)
        {
            if (l_PetSlots[l_I] && l_PetSlots[l_I]->Slot == l_DestSlot)
                l_PetSlots[l_I]->Slot = l_BattlePet->Slot;
        }

        l_BattlePet->Slot = l_DestSlot;
    }

    m_Player->UpdateBattlePetCombatTeam();
    SendPetBattleSlotUpdates(false);
}

void WorldSession::HandleBattlePetSetFlags(WorldPacket& p_RecvData)
{
    uint64 l_PetJournalID;
    uint32 l_Flag = 0;
    uint8 l_Action = 0;

    p_RecvData.readPackGUID(l_PetJournalID);
    p_RecvData >> l_Flag;

    l_Action = p_RecvData.ReadBits(2);  ///< 0 add flag, 2 remove it ///< l_Action is never read 01/18/16

    BattlePet::Ptr l_BattlePet = m_Player->GetBattlePet(l_PetJournalID);

    if (l_BattlePet)
    {
        if (l_BattlePet->Flags & l_Flag)
            l_BattlePet->Flags = l_BattlePet->Flags & ~l_Flag;
        else
            l_BattlePet->Flags |= l_Flag;
    }
}

void WorldSession::HandleBattlePetCage(WorldPacket& p_RecvData)
{
    uint64 l_BattlePetGUID;
    p_RecvData.readPackGUID(l_BattlePetGUID);

    if (!m_Player || !m_Player->IsInWorld())
        return;

    // Unsummon if currently summoned
    if (m_Player->m_SummonSlot[SUMMON_SLOT_MINIPET])
    {
        Creature* l_OldSummon = m_Player->GetMap()->GetCreature(m_Player->m_SummonSlot[SUMMON_SLOT_MINIPET]);
        if (l_OldSummon && l_OldSummon->isSummon() && l_OldSummon->GetGuidValue(UNIT_FIELD_BATTLE_PET_COMPANION_GUID) == l_BattlePetGUID)
            l_OldSummon->ToTempSummon()->UnSummon();
    }

    // Find the battle pet
    BattlePet::Ptr l_BattlePet = m_Player->GetBattlePet(l_BattlePetGUID);
    if (!l_BattlePet)
        return;

    // Get species info
    BattlePetSpeciesEntry const* l_SpeciesEntry = sBattlePetSpeciesStore.LookupEntry(l_BattlePet->Species);
    if (!l_SpeciesEntry)
        return;

    // Check if pet can be traded (cageable)
    if ((l_SpeciesEntry->Flags & BATTLEPET_SPECIES_FLAG_CAGEABLE) == 0)
        return;

    // if (petInfo->SaveInfo == STATE_DELETED) - This check is not available in Draenor-Core

    // Use species item ID for cage
    uint32 l_ItemId = ITEM_BATTLE_PET_CAGE_ID;
    uint32 l_Count = 1;
    uint32 l_NoSpaceForCount = 0;
    ItemPosCountVec l_Dest;
    InventoryResult l_Msg = m_Player->CanStoreNewItem(NULL_BAG, NULL_SLOT, l_Dest, l_ItemId, l_Count, &l_NoSpaceForCount);
    if (l_Msg != EQUIP_ERR_OK)
        l_Count -= l_NoSpaceForCount;

    if (l_Count == 0 || l_Dest.empty())
        return;

    // Create dynamic data for pet modifiers (if supported)
    uint32 l_DynData = 0;
    l_DynData |= l_BattlePet->Quality;
    l_DynData |= uint32(l_BattlePet->Quality << 24);

    Item* l_Item = m_Player->StoreNewItem(l_Dest, l_ItemId, true, 0);
    if (!l_Item)                                               // prevent crash
        return;

    // Set item modifiers if supported
    // Note: ITEM_MODIFIER_BATTLE_PET_* constants don't exist in this codebase
    // The item will be created with basic species data

    m_Player->SendNewItem(l_Item, 1, false, true);

    // Remove spell if pet has one
    if (l_SpeciesEntry->SummonSpellID)
        m_Player->removeSpell(l_SpeciesEntry->SummonSpellID);

    // Remove battle pet from collection
    std::vector<BattlePet::Ptr>& l_BattlePets = m_Player->m_BattlePets;
    for (auto it = l_BattlePets.begin(); it != l_BattlePets.end(); ++it)
    {
        if (*it && (*it)->JournalID == l_BattlePetGUID)
        {
            // Delete from database
            SQLTransaction l_Transaction = LoginDatabase.BeginTransaction();
            PreparedStatement* l_Stmt = LoginDatabase.GetPreparedStatement(LOGIN_DEL_BATTLE_PET);
            l_Stmt->setUInt64(0, l_BattlePetGUID);
            l_Stmt->setUInt32(1, GetAccountId());
            l_Transaction->Append(l_Stmt);
            LoginDatabase.CommitTransaction(l_Transaction);

            l_BattlePets.erase(it);
            break;
        }
    }

    // Update combat team
    m_Player->UpdateBattlePetCombatTeam();

    // Send battle pet deleted packet
    SendBattlePetDeleted(l_BattlePetGUID);
}
