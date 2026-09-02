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

#include "EquipmentSetPackets.h"

WorldPacket const* WorldPackets::EquipmentSet::EquipmentSetID::Write()
{
    _worldPacket << uint64(GUID);
    _worldPacket << uint32(SetID);

    return &_worldPacket;
}

WorldPacket const* WorldPackets::EquipmentSet::LoadEquipmentSet::Write()
{
    _worldPacket << uint32(SetData.size());

    for (EquipmentSetInfo::EquipmentSetData const* equipSet : SetData)
    {
        _worldPacket << uint64(equipSet->Guid);
        _worldPacket << uint32(0);
        _worldPacket << uint32(equipSet->IgnoreMask);

        for (uint8 i = 0; i < EQUIPMENT_SLOT_END; ++i)
            _worldPacket << ObjectGuid(uint64(equipSet->Items[i]));

        _worldPacket.WriteBits(equipSet->Name.length(), 8);
        _worldPacket.WriteBits(equipSet->IconName.length(), 9);
        _worldPacket.FlushBits();

        _worldPacket.WriteString(equipSet->Name);
        _worldPacket.WriteString(equipSet->IconName);
    }

    return &_worldPacket;
}

void WorldPackets::EquipmentSet::SaveEquipmentSet::Read()
{
    _worldPacket >> Set.Guid;
    uint32 setId = 0;
    _worldPacket >> setId;
    _worldPacket >> Set.IgnoreMask;

    for (uint8 i = 0; i < EQUIPMENT_SLOT_END; ++i)
    {
        ObjectGuid itemGuid;
        _worldPacket >> itemGuid;
        Set.Items[i] = itemGuid.GetCounter();
    }

    uint32 setNameLength = _worldPacket.ReadBits(8);
    uint32 setIconLength = _worldPacket.ReadBits(9);

    Set.Name = _worldPacket.ReadString(setNameLength);
    Set.IconName = _worldPacket.ReadString(setIconLength);
}

void WorldPackets::EquipmentSet::DeleteEquipmentSet::Read()
{
    _worldPacket >> ID;
}

void WorldPackets::EquipmentSet::UseEquipmentSet::Read()
{
    _worldPacket >> Inv;

    for (uint8 i = 0; i < EQUIPMENT_SLOT_END; ++i)
    {
        _worldPacket >> Items[i].Item;
        _worldPacket >> Items[i].ContainerSlot;
        _worldPacket >> Items[i].Slot;
    }
}

WorldPacket const* WorldPackets::EquipmentSet::UseEquipmentSetResult::Write()
{
    _worldPacket << uint8(Reason);

    return &_worldPacket;
}
