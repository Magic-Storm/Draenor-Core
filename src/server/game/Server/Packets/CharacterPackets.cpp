#include "CharacterPackets.h"
#include "ObjectMgr.h"
#include "PacketUtilities.h"
#include "World.h"
#include "DBCStores.h"
#include "Item.h"

enum CharacterFlags
{
    CHARACTER_FLAG_NONE                 = 0x00000000,
    CHARACTER_LOCKED_FOR_TRANSFER       = 0x00000004,
    CHARACTER_FLAG_HIDE_HELM            = 0x00000400,
    CHARACTER_FLAG_HIDE_CLOAK           = 0x00000800,
    CHARACTER_FLAG_GHOST                = 0x00002000,
    CHARACTER_FLAG_RENAME               = 0x00004000,
    CHARACTER_FLAG_LOCKED_BY_BILLING    = 0x01000000,
    CHARACTER_FLAG_DECLINED             = 0x02000000
};

enum CharacterCustomizeFlags
{
    CHAR_CUSTOMIZE_FLAG_NONE            = 0x00000000,
    CHAR_CUSTOMIZE_FLAG_CUSTOMIZE       = 0x00000001,
    CHAR_CUSTOMIZE_FLAG_FACTION         = 0x00010000,
    CHAR_CUSTOMIZE_FLAG_RACE            = 0x00100000
};

WorldPackets::Character::EnumCharactersResult::CharacterInfo::CharacterInfo(Field* fields)
{
    // Draenor CHAR_SEL_ENUM / CHAR_SEL_ENUM_DECLINED_NAME:
    //  0 guid, 1 name, 2 race, 3 class, 4 gender, 5 playerBytes, 6 playerBytes2, 7 level,
    //  8 zone, 9 map, 10 x, 11 y, 12 z, 13 guildid, 14 playerFlags, 15 at_login,
    //  16 pet.entry, 17 pet.modelid, 18 pet.level, 19 equipmentCache, 20 banned.guid, 21 slot
    //  22 declined genitive (optional)

    Guid              = ObjectGuid::Create<HIGHGUID_PLAYER>(fields[0].GetUInt32());
    Name              = fields[1].GetString();
    Race              = fields[2].GetUInt8();
    Class             = fields[3].GetUInt8();
    Sex               = fields[4].GetUInt8();

    uint32 playerBytes = fields[5].GetUInt32();
    Skin              = uint8(playerBytes & 0xFF);
    Face              = uint8((playerBytes >> 8) & 0xFF);
    HairStyle         = uint8((playerBytes >> 16) & 0xFF);
    HairColor         = uint8((playerBytes >> 24) & 0xFF);
    FacialHair        = uint8(fields[6].GetUInt32() & 0xFF);

    Level             = fields[7].GetUInt8();
    ZoneId            = int32(fields[8].GetUInt16());
    MapId             = int32(fields[9].GetUInt16());
    PreLoadPosition.x = fields[10].GetFloat();
    PreLoadPosition.y = fields[11].GetFloat();
    PreLoadPosition.z = fields[12].GetFloat();

    if (uint32 guildId = fields[13].GetUInt32())
        GuildGuid = ObjectGuid::Create<HIGHGUID_GUILD>(guildId);

    uint32 playerFlags  = fields[14].GetUInt32();
    uint32 atLoginFlags = fields[15].GetUInt16();

    if (playerFlags & PLAYER_FLAGS_HIDE_HELM)
        Flags |= CHARACTER_FLAG_HIDE_HELM;

    if (playerFlags & PLAYER_FLAGS_HIDE_CLOAK)
        Flags |= CHARACTER_FLAG_HIDE_CLOAK;

    if (playerFlags & PLAYER_FLAGS_GHOST)
        Flags |= CHARACTER_FLAG_GHOST;

    if (atLoginFlags & AT_LOGIN_RENAME)
        Flags |= CHARACTER_FLAG_RENAME;

    if (fields[20].GetUInt32())
        Flags |= CHARACTER_FLAG_LOCKED_BY_BILLING;

    if (sWorld->getBoolConfig(CONFIG_DECLINED_NAMES_USED))
    {
        if (!fields[22].GetString().empty())
            Flags |= CHARACTER_FLAG_DECLINED;
    }
    else
        Flags |= CHARACTER_FLAG_DECLINED;

    if (atLoginFlags & AT_LOGIN_LOCKED_FOR_TRANSFER)
        Flags |= CHARACTER_LOCKED_FOR_TRANSFER;

    if (atLoginFlags & AT_LOGIN_CUSTOMIZE)
        CustomizationFlag = CHAR_CUSTOMIZE_FLAG_CUSTOMIZE;
    else if (atLoginFlags & AT_LOGIN_CHANGE_FACTION)
        CustomizationFlag = CHAR_CUSTOMIZE_FLAG_FACTION;
    else if (atLoginFlags & AT_LOGIN_CHANGE_RACE)
        CustomizationFlag = CHAR_CUSTOMIZE_FLAG_RACE;

    Flags3 = 0;
    FirstLogin = (atLoginFlags & AT_LOGIN_FIRST) != 0;

    if (!(playerFlags & PLAYER_FLAGS_GHOST) && (Class == CLASS_WARLOCK || Class == CLASS_HUNTER || Class == CLASS_DEATH_KNIGHT))
    {
        if (CreatureTemplate const* creatureInfo = sObjectMgr->GetCreatureTemplate(fields[16].GetUInt32()))
        {
            Pet.CreatureDisplayId = fields[17].GetUInt32();
            Pet.Level = fields[18].GetUInt16();
            Pet.CreatureFamily = creatureInfo->family;
        }
    }

    BoostInProgress  = false;
    ProfessionIds[0] = 0;
    ProfessionIds[1] = 0;

    Tokenizer equipment(fields[19].GetString(), ' ');
    ListPosition = fields[21].GetUInt8();
    LastPlayedTime = 0;

    bool const isOld = equipment.size() != (INVENTORY_SLOT_BAG_END * 3);
    for (uint8 slot = 0; slot < INVENTORY_SLOT_BAG_END; ++slot)
    {
        uint32 visualBase = slot * 3;
        if (isOld)
        {
            uint64 itemDatas = Player::GetUInt64ValueFromArray(equipment, visualBase);
            uint32 itemId = ((uint32*)(&itemDatas))[0];
            uint32 displayId = ((uint32*)(&itemDatas))[1];
            ItemTemplate const* proto = sObjectMgr->GetItemTemplate(itemId);
            if (!proto)
                continue;

            SpellItemEnchantmentEntry const* enchantment = nullptr;
            uint32 enchantmentData = Player::GetUInt32ValueFromArray(equipment, visualBase + 1);
            for (uint8 enchantSlot = PERM_ENCHANTMENT_SLOT; enchantSlot <= TEMP_ENCHANTMENT_SLOT; ++enchantSlot)
            {
                uint32 enchantId = 0x0000FFFF & (enchantmentData >> enchantSlot * 16);
                if (!enchantId)
                    continue;
                enchantment = sSpellItemEnchantmentStore.LookupEntry(enchantId);
                if (enchantment)
                    break;
            }

            VisualItems[slot].DisplayId = displayId ? displayId : proto->DisplayInfoID;
            VisualItems[slot].DisplayEnchantId = enchantment ? enchantment->itemVisualID : 0;
            VisualItems[slot].InventoryType = proto->InventoryType;
        }
        else
        {
            ItemTemplate const* proto = sObjectMgr->GetItemTemplate(Player::GetUInt32ValueFromArray(equipment, visualBase));
            if (!proto)
                continue;
            VisualItems[slot].DisplayId = Player::GetUInt32ValueFromArray(equipment, visualBase + 1);
            VisualItems[slot].DisplayEnchantId = Player::GetUInt32ValueFromArray(equipment, visualBase + 2);
            VisualItems[slot].InventoryType = proto->InventoryType;
        }
    }
}

WorldPacket const* WorldPackets::Character::EnumCharactersResult::Write()
{
    _worldPacket.reserve(9 + Characters.size() * sizeof(CharacterInfo) + FactionChangeRestrictions.size() * sizeof(RestrictedFactionChangeRuleInfo));

    _worldPacket.WriteBit(Success);
    _worldPacket.WriteBit(IsDeletedCharacters);
    _worldPacket << uint32(Characters.size());
    _worldPacket << uint32(FactionChangeRestrictions.size());

    for (CharacterInfo const& charInfo : Characters)
    {
        _worldPacket << charInfo.Guid;
        _worldPacket << uint8(charInfo.ListPosition);
        _worldPacket << uint8(charInfo.Race);
        _worldPacket << uint8(charInfo.Class);
        _worldPacket << uint8(charInfo.Sex);
        _worldPacket << uint8(charInfo.Skin);
        _worldPacket << uint8(charInfo.Face);
        _worldPacket << uint8(charInfo.HairStyle);
        _worldPacket << uint8(charInfo.HairColor);
        _worldPacket << uint8(charInfo.FacialHair);
        _worldPacket << uint8(charInfo.Level);
        _worldPacket << int32(charInfo.ZoneId);
        _worldPacket << int32(charInfo.MapId);
        _worldPacket << charInfo.PreLoadPosition;
        _worldPacket << charInfo.GuildGuid;
        _worldPacket << uint32(charInfo.Flags);
        _worldPacket << uint32(charInfo.CustomizationFlag);
        _worldPacket << uint32(charInfo.Flags3);
        _worldPacket << uint32(charInfo.Pet.CreatureDisplayId);
        _worldPacket << uint32(charInfo.Pet.Level);
        _worldPacket << uint32(charInfo.Pet.CreatureFamily);

        _worldPacket << uint32(charInfo.ProfessionIds[0]);
        _worldPacket << uint32(charInfo.ProfessionIds[1]);

        for (uint8 slot = 0; slot < INVENTORY_SLOT_BAG_END; ++slot)
        {
            _worldPacket << uint32(charInfo.VisualItems[slot].DisplayId);
            _worldPacket << uint32(charInfo.VisualItems[slot].DisplayEnchantId);
            _worldPacket << uint8(charInfo.VisualItems[slot].InventoryType);
        }

        _worldPacket << uint32(charInfo.LastPlayedTime);
        _worldPacket.WriteBits(charInfo.Name.length(), 6);
        _worldPacket.WriteBit(charInfo.FirstLogin);
        _worldPacket.WriteBit(charInfo.BoostInProgress);
        _worldPacket.WriteBits(charInfo.unkWod61x, 5);
        _worldPacket.FlushBits();

        _worldPacket.WriteString(charInfo.Name);
    }

    for (RestrictedFactionChangeRuleInfo const& rule : FactionChangeRestrictions)
    {
        _worldPacket << int32(rule.Mask);
        _worldPacket << uint8(rule.Race);
    }

    return &_worldPacket;
}

void WorldPackets::Character::CreateCharacter::Read()
{
    CreateInfo.reset(new CharacterCreateInfo());
    uint32 nameLength = _worldPacket.ReadBits(6);
    bool const hasTemplateSet = _worldPacket.ReadBit();

    _worldPacket >> CreateInfo->Race;
    _worldPacket >> CreateInfo->Class;
    _worldPacket >> CreateInfo->Sex;
    _worldPacket >> CreateInfo->Skin;
    _worldPacket >> CreateInfo->Face;
    _worldPacket >> CreateInfo->HairStyle;
    _worldPacket >> CreateInfo->HairColor;
    _worldPacket >> CreateInfo->FacialHairStyle;
    _worldPacket >> CreateInfo->OutfitId;
    CreateInfo->Name = _worldPacket.ReadString(nameLength);
    if (hasTemplateSet)
        CreateInfo->TemplateSet = _worldPacket.read<int32>();
}

WorldPacket const* WorldPackets::Character::CreateChar::Write()
{
    _worldPacket << uint8(Code);
    return &_worldPacket;
}

void WorldPackets::Character::CharDelete::Read()
{
    _worldPacket >> Guid;
}

WorldPacket const* WorldPackets::Character::DeleteChar::Write()
{
    _worldPacket << uint8(Code);
    return &_worldPacket;
}

void WorldPackets::Character::CharacterRenameRequest::Read()
{
    RenameInfo.reset(new CharacterRenameInfo());
    _worldPacket >> RenameInfo->Guid;
    RenameInfo->NewName = _worldPacket.ReadString(_worldPacket.ReadBits(6));
}

WorldPacket const* WorldPackets::Character::CharacterRenameResult::Write()
{
    _worldPacket << uint8(Result);
    _worldPacket.WriteBit(Guid.is_initialized());
    _worldPacket.WriteBits(Name.length(), 6);
    _worldPacket.FlushBits();

    if (Guid)
        _worldPacket << *Guid;

    _worldPacket.WriteString(Name);

    return &_worldPacket;
}

void WorldPackets::Character::CharCustomize::Read()
{
    CustomizeInfo.reset(new CharCustomizeInfo());
    _worldPacket >> CustomizeInfo->CharGUID;
    _worldPacket >> CustomizeInfo->SexID;
    _worldPacket >> CustomizeInfo->SkinID;
    _worldPacket >> CustomizeInfo->HairColorID;
    _worldPacket >> CustomizeInfo->HairStyleID;
    _worldPacket >> CustomizeInfo->FacialHairStyleID;
    _worldPacket >> CustomizeInfo->FaceID;
    CustomizeInfo->CharName = _worldPacket.ReadString(_worldPacket.ReadBits(6));
}

void WorldPackets::Character::CharRaceOrFactionChange::Read()
{
    RaceOrFactionChangeInfo.reset(new CharRaceOrFactionChangeInfo());

    RaceOrFactionChangeInfo->FactionChange = _worldPacket.ReadBit();

    uint32 nameLength = _worldPacket.ReadBits(6);

    bool const hasSkinID = _worldPacket.ReadBit();
    bool const hasHairColorID = _worldPacket.ReadBit();
    bool const hasHairStyleID = _worldPacket.ReadBit();
    bool const hasFacialHairStyleID = _worldPacket.ReadBit();
    bool const hasFaceID = _worldPacket.ReadBit();

    _worldPacket >> RaceOrFactionChangeInfo->Guid;
    _worldPacket >> RaceOrFactionChangeInfo->SexID;
    _worldPacket >> RaceOrFactionChangeInfo->RaceID;

    RaceOrFactionChangeInfo->Name = _worldPacket.ReadString(nameLength);

    if (hasSkinID)
        RaceOrFactionChangeInfo->SkinID = _worldPacket.read<uint8>();

    if (hasHairColorID)
        RaceOrFactionChangeInfo->HairColorID = _worldPacket.read<uint8>();

    if (hasHairStyleID)
        RaceOrFactionChangeInfo->HairStyleID = _worldPacket.read<uint8>();

    if (hasFacialHairStyleID)
        RaceOrFactionChangeInfo->FacialHairStyleID = _worldPacket.read<uint8>();

    if (hasFaceID)
        RaceOrFactionChangeInfo->FaceID = _worldPacket.read<uint8>();
}

WorldPacket const* WorldPackets::Character::CharFactionChangeResult::Write()
{
    _worldPacket << uint8(Result);
    _worldPacket << Guid;
    _worldPacket.WriteBit(Display.is_initialized());
    _worldPacket.FlushBits();

    if (Display)
    {
        _worldPacket.WriteBits(Display->Name.length(), 6);
        _worldPacket << uint8(Display->SexID);
        _worldPacket << uint8(Display->SkinID);
        _worldPacket << uint8(Display->HairColorID);
        _worldPacket << uint8(Display->HairStyleID);
        _worldPacket << uint8(Display->FacialHairStyleID);
        _worldPacket << uint8(Display->FaceID);
        _worldPacket << uint8(Display->RaceID);
        _worldPacket.WriteString(Display->Name);
    }

    return &_worldPacket;
}

void WorldPackets::Character::GenerateRandomCharacterName::Read()
{
    _worldPacket >> Race;
    _worldPacket >> Sex;
}

WorldPacket const* WorldPackets::Character::GenerateRandomCharacterNameResult::Write()
{
    _worldPacket.WriteBit(Success);
    _worldPacket.WriteBits(Name.length(), 6);
    _worldPacket.FlushBits();

    _worldPacket.WriteString(Name);

    return &_worldPacket;
}

WorldPackets::Character::ReorderCharacters::ReorderCharacters(WorldPacket&& packet) : ClientPacket(CMSG_REORDER_CHARACTERS, std::move(packet)),
    Entries(sWorld->getIntConfig(CONFIG_CHARACTERS_PER_REALM))
{

}

void WorldPackets::Character::ReorderCharacters::Read()
{
    Entries.resize(_worldPacket.ReadBits(9));
    for (ReorderInfo& reorderInfo : Entries)
    {
        _worldPacket >> reorderInfo.PlayerGUID;
        _worldPacket >> reorderInfo.NewPosition;
    }
}

void WorldPackets::Character::UndeleteCharacter::Read()
{
    UndeleteInfo.reset(new CharacterUndeleteInfo());
    _worldPacket >> UndeleteInfo->ClientToken;
    _worldPacket >> UndeleteInfo->CharacterGuid;
}

WorldPacket const* WorldPackets::Character::UndeleteCharacterResponse::Write()
{
    ASSERT(UndeleteInfo);
    _worldPacket << int32(UndeleteInfo->ClientToken);
    _worldPacket << uint32(Result);
    _worldPacket << UndeleteInfo->CharacterGuid;
    return &_worldPacket;
}

WorldPacket const* WorldPackets::Character::UndeleteCooldownStatusResponse::Write()
{
    _worldPacket.WriteBit(OnCooldown);
    _worldPacket << uint32(MaxCooldown);
    _worldPacket << uint32(CurrentCooldown);
    return &_worldPacket;
}

void WorldPackets::Character::PlayerLogin::Read()
{
    _worldPacket >> Guid;
    _worldPacket >> FarClip;
}

WorldPacket const* WorldPackets::Character::LoginVerifyWorld::Write()
{
    _worldPacket << int32(MapID);
    _worldPacket << Pos.PositionXYZOStream();
    _worldPacket << uint32(Reason);
    return &_worldPacket;
}

WorldPacket const* WorldPackets::Character::CharacterLoginFailed::Write()
{
    _worldPacket << uint8(Code);
    return &_worldPacket;
}

WorldPacket const* WorldPackets::Character::LogoutResponse::Write()
{
    _worldPacket << int32(LogoutResult);
    _worldPacket.WriteBit(Instant);
    _worldPacket.FlushBits();
    return &_worldPacket;
}

WorldPacket const* WorldPackets::Character::LogoutComplete::Write()
{
    _worldPacket << SwitchToCharacter;
    return &_worldPacket;
}

void WorldPackets::Character::LoadingScreenNotify::Read()
{
    _worldPacket >> MapID;
    Showing = _worldPacket.ReadBit();
}

WorldPacket const* WorldPackets::Character::InitialSetup::Write()
{
    _worldPacket << uint8(ServerExpansionLevel);
    _worldPacket << uint8(ServerExpansionTier);
    _worldPacket << int32(ServerRegionID);
    _worldPacket << uint32(RaidOrigin);

    return &_worldPacket;
}

void WorldPackets::Character::SetActionBarToggles::Read()
{
    _worldPacket >> Mask;
}

void WorldPackets::Character::RequestPlayedTime::Read()
{
    TriggerScriptEvent = _worldPacket.ReadBit();
}

WorldPacket const* WorldPackets::Character::PlayedTime::Write()
{
    _worldPacket << int32(TotalTime);
    _worldPacket << int32(LevelTime);
    _worldPacket.WriteBit(TriggerEvent);
    _worldPacket.FlushBits();

    return &_worldPacket;
}

void WorldPackets::Character::ShowingCloak::Read()
{
    ShowCloak = _worldPacket.ReadBit();
}

void WorldPackets::Character::ShowingHelm::Read()
{
    ShowHelm = _worldPacket.ReadBit();
}

void WorldPackets::Character::SetTitle::Read()
{
    _worldPacket >> TitleID;
}

void WorldPackets::Character::AlterApperance::Read()
{
    _worldPacket >> NewHairStyle;
    _worldPacket >> NewHairColor;
    _worldPacket >> NewFacialHair;
    _worldPacket >> NewSkinColor;
    _worldPacket >> NewFace;
}

WorldPacket const* WorldPackets::Character::BarberShopResultServer::Write()
{
    _worldPacket << int32(Result);
    return &_worldPacket;
}

WorldPacket const* WorldPackets::Character::LogXPGain::Write()
{
    _worldPacket << Victim;
    _worldPacket << int32(Original);
    _worldPacket << uint8(Reason);
    _worldPacket << int32(Amount);
    _worldPacket << float(GroupBonus);
    _worldPacket.WriteBit(ReferAFriend);

    _worldPacket.FlushBits();

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Character::TitleEarned::Write()
{
    _worldPacket << uint32(Index);

    return &_worldPacket;
}

void WorldPackets::Character::SetFactionAtWar::Read()
{
    _worldPacket >> FactionIndex;
}

void WorldPackets::Character::SetFactionNotAtWar::Read()
{
    _worldPacket >> FactionIndex;
}

void WorldPackets::Character::SetFactionInactive::Read()
{
    _worldPacket >> Index;
    State = _worldPacket.ReadBit();
}

void WorldPackets::Character::SetWatchedFaction::Read()
{
    _worldPacket >> FactionIndex;
}

WorldPacket const* WorldPackets::Character::SetFactionVisible::Write()
{
    _worldPacket << FactionIndex;
    return &_worldPacket;
}

WorldPackets::Character::CharCustomizeResponse::CharCustomizeResponse(WorldPackets::Character::CharCustomizeInfo const* info)
    : ServerPacket(SMSG_CHAR_CUSTOMIZE, 16 + 1 + 1 + 1 + 1 + 1 + 1 + 1)
{
    CharGUID = info->CharGUID;
    SexID = info->SexID;
    SkinID = info->SkinID;
    HairColorID = info->HairColorID;
    HairStyleID = info->HairStyleID;
    FacialHairStyleID = info->FacialHairStyleID;
    FaceID = info->FaceID;
    CharName = info->CharName;
}

WorldPacket const* WorldPackets::Character::CharCustomizeResponse::Write()
{
    _worldPacket << CharGUID;
    _worldPacket << uint8(SexID);
    _worldPacket << uint8(SkinID);
    _worldPacket << uint8(HairColorID);
    _worldPacket << uint8(HairStyleID);
    _worldPacket << uint8(FacialHairStyleID);
    _worldPacket << uint8(FaceID);
    _worldPacket.WriteBits(CharName.length(), 6);
    _worldPacket.FlushBits();
    _worldPacket.WriteString(CharName);

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Character::CharCustomizeFailed::Write()
{
    _worldPacket << uint8(Result);
    _worldPacket << CharGUID;

    return &_worldPacket;
}

void WorldPackets::Character::SetPlayerDeclinedNames::Read()
{
    _worldPacket >> Player;

    uint8 stringLengths[MAX_DECLINED_NAME_CASES];

    for (uint8 i = 0; i < MAX_DECLINED_NAME_CASES; ++i)
        stringLengths[i] = _worldPacket.ReadBits(7);

    for (uint8 i = 0; i < MAX_DECLINED_NAME_CASES; ++i)
        DeclinedNames.name[i] = _worldPacket.ReadString(stringLengths[i]);
}

WorldPacket const * WorldPackets::Character::SetPlayerDeclinedNamesResult::Write()
{
    _worldPacket << int32(ResultCode);
    _worldPacket << Player;

    return &_worldPacket;
}
