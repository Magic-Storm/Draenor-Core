////////////////////////////////////////////////////////////////////////////////
//
// Project-Hellscream https://hellscream.org
// Copyright (C) 2018-2020 Project-Hellscream-6.2
// Discord https://discord.gg/CWCF3C9
//
////////////////////////////////////////////////////////////////////////////////

#include "Common.h"
#include "Language.h"
#include "DatabaseEnv.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include "Opcodes.h"
#include "Log.h"
#include "World.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "UpdateMask.h"
#include "NPCHandler.h"
#include "QueryPackets.h"
#include "Pet.h"
#include "MapManager.h"

enum NameQueryResponse
{
    NAME_QUERY_RESULT_OK    = 0,
    NAME_QUERY_RESULT_DENY  = 1,
    NAME_QUERY_RESULT_RETRY = 2,
};

void WorldSession::SendNameQueryOpcode(uint64 guid, bool /*atLeave*/)
{
#ifndef CROSS
    Player* player = ObjectAccessor::FindPlayer(guid);
    WorldPackets::Query::QueryPlayerNameResponse response;
    response.Player = ObjectGuid(guid);
    if (response.Data.Initialize(ObjectGuid(guid), player))
        response.Result = RESPONSE_SUCCESS;
    else
        response.Result = NAME_QUERY_RESULT_DENY;
    SendPacket(response.Write());
#else /* CROSS */
    Player* player = ObjectAccessor::FindPlayerInOrOutOfWorld(guid);
    InterRealmClient* playerClient = player ? player->GetSession()->GetInterRealmClient() : nullptr;

    WorldPacket data(SMSG_NAME_QUERY_RESPONSE);
    data << uint8(playerClient ? NAME_QUERY_RESULT_OK : NAME_QUERY_RESULT_DENY);
    data.appendPackGUID(guid);

    if (playerClient)
    {
        data.WriteBit(false);
        data.WriteBits(strlen(player->GetName()), 6);

        if (DeclinedName const* names = (player ? player->GetDeclinedNames() : NULL))
        {
            for (uint8 i = 0; i < MAX_DECLINED_NAME_CASES; ++i)
                data.WriteBits(names->name[i].size(), 7);
        }
        else
        {
            for (uint8 i = 0; i < MAX_DECLINED_NAME_CASES; ++i)
                data.WriteBits(0, 7);
        }

        data.FlushBits();

        if (DeclinedName const* names = (player ? player->GetDeclinedNames() : NULL))
        {
            for (uint8 i = 0; i < MAX_DECLINED_NAME_CASES; ++i)
                if (names->name[i].size())
                    data.append(names->name[i].c_str(), names->name[i].size());
        }

        data.appendPackGUID(player ? player->GetSession()->GetWoWAccountGUID()  : 0);
        data.appendPackGUID(player ? player->GetSession()->GetBNetAccountGUID() : 0);
        data.appendPackGUID(guid);

        data << uint32(playerClient->GetRealmId());
        data << uint8(player->getRace());
        data << uint8(player->getGender());
        data << uint8(player->getClass());
        data << uint8(player->getLevel());
        data.WriteString(player->GetName());
    }

    SendPacket(&data);
#endif /* CROSS */
}

void WorldSession::HandleNameQueryOpcode(WorldPacket& recvData)
{
    WorldPackets::Query::QueryPlayerName query(std::move(recvData));
    query.Read();
    SendNameQueryOpcode(uint64(query.Player));
}

void WorldSession::HandleQueryTimeOpcode(WorldPacket& /*recvData*/)
{
    SendQueryTimeResponse();
}

void WorldSession::SendQueryTimeResponse()
{
    WorldPackets::Query::QueryTimeResponse response;
    response.CurrentTime = time(nullptr);
    response.TimeOutRequest = int32(sWorld->GetNextDailyQuestsResetTime() - time(nullptr));
    SendPacket(response.Write());
}

/// Only _static_ data is sent in this packet !!!
void WorldSession::HandleCreatureQueryOpcode(WorldPacket& recvData)
{
    WorldPackets::Query::QueryCreature query(std::move(recvData));
    query.Read();
    uint32 entry = query.CreatureID;

    WorldPackets::Query::QueryCreatureResponse response;
    response.CreatureID = entry;

    if (CreatureTemplate const* creatureInfo = sObjectMgr->GetCreatureTemplate(entry))
    {
        std::string Name = creatureInfo->Name;
        std::string SubName = creatureInfo->SubName;
        std::string FemaleName = creatureInfo->FemaleName;

        LocaleConstant locale = GetSessionDbLocaleIndex();
        if (locale >= 0)
        {
            if (CreatureLocale const* creatureLocale = sObjectMgr->GetCreatureLocale(entry))
            {
                ObjectMgr::GetLocaleString(creatureLocale->Name, locale, Name);
                ObjectMgr::GetLocaleString(creatureLocale->SubName, locale, SubName);
                ObjectMgr::GetLocaleString(creatureLocale->FemaleName, locale, FemaleName);
            }
        }

        response.Allow = true;
        response.Stats.Title = SubName;
        response.Stats.CursorName = creatureInfo->IconName;
        response.Stats.Leader = creatureInfo->RacialLeader;
        response.Stats.Name[0] = Name;
        response.Stats.NameAlt[0] = FemaleName;
        response.Stats.Flags[0] = creatureInfo->type_flags;
        response.Stats.Flags[1] = creatureInfo->type_flags2;
        response.Stats.CreatureType = creatureInfo->type;
        response.Stats.CreatureFamily = creatureInfo->family;
        response.Stats.Classification = creatureInfo->rank;
        response.Stats.ProxyCreatureID[0] = creatureInfo->KillCredit[0];
        response.Stats.ProxyCreatureID[1] = creatureInfo->KillCredit[1];
        response.Stats.CreatureDisplayID[0] = creatureInfo->Modelid1;
        response.Stats.CreatureDisplayID[1] = creatureInfo->Modelid2;
        response.Stats.CreatureDisplayID[2] = creatureInfo->Modelid3;
        response.Stats.CreatureDisplayID[3] = creatureInfo->Modelid4;
        response.Stats.HpMulti = creatureInfo->ModHealth;
        response.Stats.EnergyMulti = creatureInfo->ModMana;
        response.Stats.CreatureMovementInfoID = creatureInfo->movementId;
        response.Stats.RequiredExpansion = creatureInfo->RequiredExpansion;
        for (uint32 i = 0; i < MAX_CREATURE_QUEST_ITEMS; ++i)
            if (creatureInfo->questItems[i])
                response.Stats.QuestItems.push_back(int32(creatureInfo->questItems[i]));
    }

    SendPacket(response.Write());
}

/// Only _static_ data is sent in this packet !!!
void WorldSession::HandleGameObjectQueryOpcode(WorldPacket& recvData)
{
    WorldPackets::Query::QueryGameObject query(std::move(recvData));
    query.Read();

    WorldPackets::Query::QueryGameObjectResponse response;
    response.GameObjectID = query.GameObjectID;

    if (GameObjectTemplate const* gobInfo = sObjectMgr->GetGameObjectTemplate(query.GameObjectID))
    {
        std::string name = gobInfo->name;
        std::string castBarCaption = gobInfo->castBarCaption;

        int localeIndex = GetSessionDbLocaleIndex();
        if (localeIndex >= 0)
        {
            if (GameObjectLocale const* gobLocale = sObjectMgr->GetGameObjectLocale(query.GameObjectID))
            {
                ObjectMgr::GetLocaleString(gobLocale->Name, localeIndex, name);
                ObjectMgr::GetLocaleString(gobLocale->CastBarCaption, localeIndex, castBarCaption);
            }
        }

        response.Allow = true;
        response.Stats.Type = gobInfo->type;
        response.Stats.DisplayID = gobInfo->displayId;
        response.Stats.Name[0] = name;
        response.Stats.IconName = gobInfo->IconName;
        response.Stats.CastBarCaption = castBarCaption;
        for (uint32 i = 0; i < MAX_GAMEOBJECT_DATA; ++i)
            response.Stats.Data[i] = gobInfo->raw.data[i];
        response.Stats.Size = gobInfo->size;
        for (uint32 i = 0; i < MAX_GAMEOBJECT_QUEST_ITEMS; ++i)
            if (gobInfo->questItems[i])
                response.Stats.QuestItems.push_back(int32(gobInfo->questItems[i]));
        response.Stats.Expansion = uint32(gobInfo->unkInt32);
    }

    SendPacket(response.Write());
}

void WorldSession::HandleCorpseLocationFromClientQueryOpcode(WorldPacket& /*recvData*/)
{
    WorldPackets::Query::CorpseLocation packet;
    Corpse* corpse = GetPlayer()->GetCorpse();

    if (!corpse)
    {
        packet.Valid = false;
        SendPacket(packet.Write());
        return;
    }

    uint32 mapId = corpse->GetMapId();
    uint32 corpseMapId = mapId;
    float x = corpse->GetTransport() ? corpse->GetTransOffsetX() : corpse->GetPositionX();
    float y = corpse->GetTransport() ? corpse->GetTransOffsetY() : corpse->GetPositionY();
    float z = corpse->GetTransport() ? corpse->GetTransOffsetZ() : corpse->GetPositionZ();

    if (mapId != m_Player->GetMapId())
    {
        if (MapEntry const* corpseMapEntry = sMapStore.LookupEntry(mapId))
        {
            if (corpseMapEntry->IsDungeon() && corpseMapEntry->CorpseMapID >= 0)
            {
                if (Map const* entranceMap = sMapMgr->CreateBaseMap(corpseMapEntry->CorpseMapID))
                {
                    mapId = corpseMapEntry->CorpseMapID;
                    x = corpseMapEntry->CorpseX;
                    y = corpseMapEntry->CorpseY;
                    z = entranceMap->GetHeight(GetPlayer()->GetPhaseMask(), x, y, MAX_HEIGHT);
                }
            }
        }
    }

    packet.Valid = true;
    packet.ActualMapID = int32(corpseMapId);
    packet.MapID = int32(mapId);
    packet.Position = G3D::Vector3(x, y, z);
    packet.Transport = ObjectGuid(corpse->GetTransGUID());
    SendPacket(packet.Write());
}
void WorldSession::HandleCemeteryListOpcode(WorldPacket& /*recvData*/)
{
    GetPlayer()->SendCemeteryList(false);
}

void WorldSession::HandleForcedReactionsOpcode(WorldPacket& /*recvData*/)
{
    GetPlayer()->GetReputationMgr().SendForceReactions();
}

void WorldSession::HandleNpcTextQueryOpcode(WorldPacket& p_Packet)
{
    uint32 l_TextID;
    uint64 l_Guid;

    p_Packet >> l_TextID;
    p_Packet.readPackGUID(l_Guid);

    GetPlayer()->SetSelection(l_Guid);

    GossipText const* pGossip = sObjectMgr->GetGossipText(l_TextID); ///< pGossip is never read 01/18/16

    bool l_Allow = true;

    WorldPacket l_Data(SMSG_NPC_TEXT_UPDATE, 100);          // guess size
    l_Data << l_TextID;
    l_Data.WriteBit(l_Allow);     // unk bit (true on retail sniff)
    l_Data.FlushBits();

    ByteBuffer l_Buffer;

    /// Text probability
    for (size_t l_I = 0; l_I < MAX_GOSSIP_TEXT_OPTIONS; l_I++)
        l_Buffer << float(0);

        /// Look like that make random bug on gossip
        //l_Buffer << float(pGossip ? pGossip->Options[l_I].Probability : 0);

    l_Buffer << uint32(l_TextID);

    /// Text ID
    for (size_t l_I = 0; l_I < MAX_GOSSIP_TEXT_OPTIONS - 1; l_I++)
        l_Buffer << uint32(0);

    l_Data << uint32(l_Buffer.size());
    l_Data.append(l_Buffer);

    SendPacket(&l_Data);
}

#define DEFAULT_GREETINGS_GOSSIP      68

void WorldSession::SendBroadcastTextDb2Reply(uint32 p_Entry)
{
    ByteBuffer l_ReplyContent;

    WorldPacket data(SMSG_DB_REPLY, 2 * 1024);
    data << uint32(DB2_REPLY_BROADCAST_TEXT);
    data << uint32(p_Entry);
    data << uint32(sObjectMgr->GetHotfixDate(p_Entry, DB2_REPLY_BROADCAST_TEXT));
    data.WriteBit(1);                                                         ///< Found ???

    //////////////////////////////////////////////////////////////////////////

    GossipText const* l_Gossip = sObjectMgr->GetGossipText(p_Entry);

    if (!l_Gossip)
        l_Gossip = sObjectMgr->GetGossipText(DEFAULT_GREETINGS_GOSSIP);

    std::string l_DefaultText = "Greetings, $N";
    std::string l_Text1 = l_Gossip ? l_Gossip->Options[0].Text_0 : l_DefaultText;
    std::string l_Text2 = l_Gossip ? l_Gossip->Options[0].Text_1 : l_DefaultText;

    int l_LocaleIndex = GetSessionDbLocaleIndex();
    if (l_LocaleIndex >= 0 && l_LocaleIndex <= MAX_LOCALES)
    {
        if (NpcTextLocale const* l_NpcTextLocale = sObjectMgr->GetNpcTextLocale(p_Entry))
        {
            ObjectMgr::GetLocaleString(l_NpcTextLocale->Text_0[0], l_LocaleIndex, l_Text1);
            ObjectMgr::GetLocaleString(l_NpcTextLocale->Text_1[0], l_LocaleIndex, l_Text2);
        }
    }

    l_ReplyContent << uint32(p_Entry);
    l_ReplyContent << uint32(l_Gossip != 0 ? l_Gossip->Options[0].Language : LANG_UNIVERSAL);
    l_ReplyContent << uint16(l_Text1.size() ? l_Text1.size() + 1 : 0);

    if (l_Text1.size())
        l_ReplyContent << std::string(l_Text1);

    l_ReplyContent << uint16(l_Text2.size() ? l_Text2.size() + 1 : 0);

    if (l_Text2.size())
        l_ReplyContent << std::string(l_Text2);

    if (l_Gossip)
    {
        for (int l_J = 0; l_J < MAX_GOSSIP_TEXT_EMOTES; ++l_J)
            l_ReplyContent << uint32(l_Gossip->Options[0].Emotes[l_J]._Emote);
        for (int l_J = 0; l_J < MAX_GOSSIP_TEXT_EMOTES; ++l_J)
            l_ReplyContent << uint32(l_Gossip->Options[0].Emotes[l_J]._Delay);
    }
    else
    {
        for (int l_J = 0; l_J < MAX_GOSSIP_TEXT_EMOTES; ++l_J)
        {
            l_ReplyContent << uint32(0);
            l_ReplyContent << uint32(0);
        }
    }

    l_ReplyContent << uint32(l_Gossip->SoundID);
    l_ReplyContent << uint32(0);
    l_ReplyContent << uint32(0x01);   /// unk

    //////////////////////////////////////////////////////////////////////////

    data << uint32(l_ReplyContent.size());
    data.append(l_ReplyContent);

    SendPacket(&data);
}

/// Only _static_ data is sent in this packet !!!
void WorldSession::HandlePageTextQueryOpcode(WorldPacket& p_Packet)
{
    uint64 l_ItemGUID   = 0;
    uint32 l_PageTextID = 0;

    p_Packet >> l_PageTextID;
    p_Packet.readPackGUID(l_ItemGUID);

    while (l_PageTextID)
    {
        PageText const* l_PageText = sObjectMgr->GetPageText(l_PageTextID);

        WorldPacket l_Data(SMSG_PAGE_TEXT_QUERY_RESPONSE, 2 * 1024);
        l_Data << uint32(l_PageTextID);                             ///< Page Text ID
        l_Data.WriteBit(l_PageText != NULL);                        ///< Allow
        l_Data.FlushBits();

        if (l_PageText)
        {
            std::string l_Text = l_PageText->Text;

            int l_Locale = GetSessionDbLocaleIndex();
            if (l_Locale >= 0)
            {
                if (PageTextLocale const* l_PageTxtLocale = sObjectMgr->GetPageTextLocale(l_PageTextID))
                    ObjectMgr::GetLocaleString(l_PageTxtLocale->Text, l_Locale, l_Text);
            }

            l_Data << uint32(l_PageTextID);                         ///< ID
            l_Data << uint32(l_PageText->NextPage);                 ///< Next Page ID

            l_Data.WriteBits(l_Text.size(), 12);                    ///< Text
            l_Data.FlushBits();

            l_Data.WriteString(l_Text);                             ///< Text
        }

        if (l_PageText)
            l_PageTextID = l_PageText->NextPage;
        else
            l_PageTextID = 0;

        SendPacket(&l_Data);
    }
}

void WorldSession::HandleCorpseTransportQueryOpcode(WorldPacket & p_Packet)
{
    uint64 l_TransportGUID = 0;

    p_Packet.readPackGUID(l_TransportGUID);

    GameObject * l_TransportGOB = HashMapHolder<GameObject>::Find(l_TransportGUID);

    if (!l_TransportGOB)
        return;

    if (!l_TransportGOB->IsTransport())
        return;

    WorldPacket data(SMSG_CORPSE_TRANSPORT_QUERY, 4+4+4+4);
    data << float(l_TransportGOB->GetPositionX());
    data << float(l_TransportGOB->GetPositionY());
    data << float(l_TransportGOB->GetPositionZ());
    data << float(l_TransportGOB->GetOrientation());

    SendPacket(&data);
}

void WorldSession::HandleQuestPOIQuery(WorldPacket& p_Packet)
{
    uint32 l_Count;

    p_Packet >> l_Count;

    if (l_Count >= MAX_QUEST_LOG_SIZE)
    {
        p_Packet.rfinish();
        return;
    }

    std::list<uint32> l_QuestList;

    for (uint32 i = 0; i < l_Count; ++i)
        l_QuestList.push_back(p_Packet.read<uint32>());

    WorldPacket l_Data(SMSG_QUEST_POIQUERY_RESPONSE, 15 * 1024);
    l_Data << uint32(l_Count); // count
    l_Data << uint32(l_Count);

    for (auto l_CurrentQuest : l_QuestList)
    {
        uint32 l_QuestID = l_CurrentQuest;
        uint16 l_QuestSlot = m_Player->FindQuestSlot(l_QuestID);
        bool l_QuestOK = false;

        if (l_QuestSlot != MAX_QUEST_LOG_SIZE)
            l_QuestOK =m_Player->GetQuestSlotQuestId(l_QuestSlot) == l_QuestID;

        if (l_QuestOK)
        {
            QuestPOIVector const* l_POIs = sObjectMgr->GetQuestPOIVector(l_QuestID);

            if (l_POIs)
            {
                l_Data << uint32(l_QuestID);
                l_Data << uint32(l_POIs->size());     ///< Num Blobs

                const Quest * l_Quest = sObjectMgr->GetQuestTemplate(l_QuestID);

                for (QuestPOIVector::const_iterator l_It = l_POIs->begin(); l_It != l_POIs->end(); ++l_It)
                {
                    const QuestObjective * l_Objective = l_Quest->GetQuestObjectiveXIndex(l_It->ObjectiveIndex);

                    uint32 l_QuestObjectiveID   = 0;
                    uint32 l_QuestObjectID      = 0;

                    if (l_Objective)
                    {
                        l_QuestObjectiveID = l_Objective->ID;
                        l_QuestObjectID = l_Objective->ObjectID;
                    }

                    l_Data << uint32(l_It->Id);                   ///< BlobIndex
                    l_Data << int32(l_It->ObjectiveIndex);        ///< Objective Index
                    l_Data << uint32(l_QuestObjectiveID);         ///< Quest Objective ID
                    l_Data << uint32(l_QuestObjectID);            ///< Quest Object ID
                    l_Data << uint32(l_It->MapID);                ///< Map ID
                    l_Data << uint32(l_It->WorldMapAreaID);       ///< World Map Area ID
                    l_Data << uint32(l_It->Floor);                ///< Floor
                    l_Data << uint32(l_It->Priority);             ///< Priority
                    l_Data << uint32(l_It->Flags);                ///< Flags
                    l_Data << uint32(l_It->WorldEffectID);        ///< World Effect ID
                    l_Data << uint32(l_It->PlayerConditionID);    ///< Player Condition ID
                    l_Data << uint32(0);                          ///< Unk Wod
                    l_Data << uint32(l_It->Points.size());

                    for (std::vector<QuestPOIPoint>::const_iterator l_PointIT = l_It->Points.begin(); l_PointIT != l_It->Points.end(); ++l_PointIT)
                    {
                        l_Data << int32(l_PointIT->x);            ///< X
                        l_Data << int32(l_PointIT->y);            ///< Y
                    }

                }
            }
            else
            {
                l_Data << uint32(l_QuestID);  ///< Quest ID
                l_Data << uint32(0);          ///< Num Blobs
            }
        }
        else
        {
            l_Data << uint32(l_QuestID);      ///< Quest ID
            l_Data << uint32(0);              ///< Num Blobs
        }
    }

    SendPacket(&l_Data);
}

void WorldSession::HandleDBQueryBulk(WorldPacket& p_RecvPacket)
{
    uint32 l_Type = 0;
    uint32 l_Count = 0;

    p_RecvPacket >> l_Type;
    l_Count = p_RecvPacket.ReadBits(13);

    DB2StorageBase* l_DB2Store = sDB2PerHash[l_Type];

    for (uint32 l_I = 0; l_I < l_Count; ++l_I)
    {
        uint64 l_GUID;
        uint32 l_Entry;

        p_RecvPacket.readPackGUID(l_GUID);
        p_RecvPacket >> l_Entry;

        /// Specific case, localized string not yet supported by the system
        if (l_Type == DB2_REPLY_BROADCAST_TEXT)
        {
            SendBroadcastTextDb2Reply(l_Entry);
        }
        else if (l_DB2Store)
        {
            ByteBuffer l_ResponseData(2 * 1024);
            if (l_DB2Store->WriteRecord(l_Entry, l_ResponseData, GetSessionDbLocaleIndex()))
            {
                WorldPacket l_Data(SMSG_DB_REPLY, 4 + 4 + 4 + 4 + l_ResponseData.size());
                l_Data << uint32(l_Type);
                l_Data << uint32(l_Entry);
                l_Data << uint32(sObjectMgr->GetHotfixDate(l_Entry, l_Type));
                l_Data.WriteBit(1);                                                         ///< Found ???
                l_Data << uint32(l_ResponseData.size());
                l_Data.append(l_ResponseData);

                SendPacket(&l_Data);
            }
            else
            {
                WorldPacket l_Data(SMSG_DB_REPLY, 4 + 4 + 4 + 4);
                l_Data << uint32(l_Type);
                l_Data << uint32(-int32(l_Entry));
                l_Data << uint32(time(NULL));
                l_Data.WriteBit(0);                                                         ///< Found ???
                l_Data << uint32(0);

                SendPacket(&l_Data);
            }
        }
    }
}
