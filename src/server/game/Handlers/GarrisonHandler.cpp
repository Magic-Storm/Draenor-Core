#ifndef CROSS
#include "Common.h"
#include "Language.h"
#include "DatabaseEnv.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include "Opcodes.h"
#include "Log.h"
#include "World.h"
#include "ObjectMgr.h"
#include "InstanceSaveMgr.h"
#include "Player.h"
#include "UpdateMask.h"
#include "NPCHandler.h"
#include "Pet.h"
#include "Group.h"
#include "MapManager.h"
#include "GarrisonMgr.hpp"
#include "GarrisonPackets.h"
#include "DB2Stores.h"
#include "CreatureAI.h"
#include "Chat.h"
#include "ScriptMgr.h"
#include "../../../scripts/Draenor/Garrison/GarrisonScriptData.hpp"
#include "../../scripts/Draenor/Garrison/GarrisonNPC.hpp"

namespace
{
    WorldPackets::Garrison::GarrisonBuildingInfo ToPacketBuilding(MS::Garrison::GarrisonBuilding const& building)
    {
        WorldPackets::Garrison::GarrisonBuildingInfo packet;
        packet.GarrPlotInstanceID = building.PlotInstanceID;
        packet.GarrBuildingID = building.BuildingID;
        packet.TimeBuilt = time_t(building.TimeBuiltStart);
        packet.CurrentGarSpecID = building.SpecID;
        packet.TimeSpecCooldown = time_t(building.TimeBuiltEnd);
        packet.Active = building.Active;
        return packet;
    }

    WorldPackets::Garrison::GarrisonFollower ToPacketFollower(MS::Garrison::GarrisonFollower const& follower)
    {
        WorldPackets::Garrison::GarrisonFollower packet;
        packet.DbID = follower.DatabaseID;
        packet.GarrFollowerID = follower.FollowerID;
        packet.Quality = follower.Quality;
        packet.FollowerLevel = uint32(follower.Level);
        packet.ItemLevelWeapon = uint32(follower.ItemLevelWeapon);
        packet.ItemLevelArmor = uint32(follower.ItemLevelArmor);
        packet.Xp = follower.XP;
        packet.CurrentBuildingID = follower.CurrentBuildingID;
        packet.CurrentMissionID = follower.CurrentMissionID;
        packet.AbilityID = follower.Abilities;
        packet.FollowerStatus = follower.Flags;
        packet.CustomName = follower.ShipName;
        return packet;
    }

    WorldPackets::Garrison::GarrisonMission ToPacketMission(MS::Garrison::GarrisonMission const& mission, MS::Garrison::Manager* garrison)
    {
        WorldPackets::Garrison::GarrisonMission packet;
        packet.DbID = mission.DatabaseID;
        packet.MissionRecID = mission.MissionID;
        packet.OfferTime = time_t(mission.OfferTime);
        packet.OfferDuration = mission.OfferMaxDuration;
        packet.StartTime = time_t(mission.StartTime);
        if (mission.State == MS::Garrison::Mission::State::InProgress && sGarrMissionStore.LookupEntry(mission.MissionID))
        {
            packet.TravelDuration = garrison->GetMissionTravelDuration(mission.MissionID);
            packet.MissionDuration = garrison->GetMissionDuration(mission.MissionID);
        }
        packet.MissionState = mission.State;
        return packet;
    }

    WorldPackets::Garrison::Shipment ToPacketShipment(MS::Garrison::Manager* garrison, MS::Garrison::GarrisonWorkOrder const& order, bool includeAssignedFollower)
    {
        WorldPackets::Garrison::Shipment packet;
        packet.ShipmentRecID = order.ShipmentID;
        packet.ShipmentID = order.DatabaseID;
        if (includeAssignedFollower)
            packet.AssignedFollowerDBID = garrison->GetBuilding(order.PlotInstanceID).FollowerAssigned;
        packet.CreationTime = order.CreationTime;
        packet.ShipmentDuration = order.CompleteTime > order.CreationTime ? (order.CompleteTime - order.CreationTime) : 0;
        return packet;
    }
}

void WorldSession::HandleGetGarrisonInfoOpcode(WorldPacket& /*p_RecvData*/)
{
    if (!m_Player)
        return;

    MS::Garrison::Manager* l_Garrison = m_Player->GetGarrison();

    if (!l_Garrison || !l_Garrison->GetGarrisonSiteLevelEntry())
        return;

    std::vector<MS::Garrison::GarrisonPlotInstanceInfoLocation> l_Plots = l_Garrison->GetPlots();
    std::vector<MS::Garrison::GarrisonMission> l_CompletedMission = l_Garrison->GetCompletedMissions();
    std::vector<MS::Garrison::GarrisonMission> l_Missions = l_Garrison->GetMissions();
    std::vector<MS::Garrison::GarrisonBuilding> l_Buildings = l_Garrison->GetBuildings();
    std::vector<MS::Garrison::GarrisonFollower> l_Followers = l_Garrison->GetFollowers();

    if (!m_Player->IsInGarrison())
    {
        WorldPackets::Garrison::GarrisonRemoteInfo remoteInfo;
        WorldPackets::Garrison::GarrisonRemoteSiteInfo site;
        site.GarrSiteLevelID = l_Garrison->GetGarrisonSiteLevelEntry()->SiteLevelID;
        site.Buildings.reserve(l_Buildings.size());
        for (MS::Garrison::GarrisonBuilding const& building : l_Buildings)
            site.Buildings.emplace_back(building.PlotInstanceID, building.BuildingID);
        remoteInfo.Sites.push_back(std::move(site));
        SendPacket(remoteInfo.Write());
    }

    std::vector<WorldPackets::Garrison::GarrisonBuildingInfo> buildings;
    std::vector<WorldPackets::Garrison::GarrisonPlotInfo> plots;
    std::vector<WorldPackets::Garrison::GarrisonFollower> followers;
    std::vector<WorldPackets::Garrison::GarrisonMission> missions;

    buildings.reserve(l_Buildings.size());
    for (MS::Garrison::GarrisonBuilding const& building : l_Buildings)
        buildings.push_back(ToPacketBuilding(building));

    plots.reserve(l_Plots.size());
    for (MS::Garrison::GarrisonPlotInstanceInfoLocation const& plot : l_Plots)
    {
        WorldPackets::Garrison::GarrisonPlotInfo packetPlot;
        packetPlot.GarrPlotInstanceID = plot.PlotInstanceID;
        packetPlot.PlotPos.Relocate(plot.X, plot.Y, plot.Z, plot.O);
        packetPlot.PlotType = l_Garrison->GetPlotType(plot.PlotInstanceID);
        plots.push_back(packetPlot);
    }

    followers.reserve(l_Followers.size());
    for (MS::Garrison::GarrisonFollower const& follower : l_Followers)
        followers.push_back(ToPacketFollower(follower));

    missions.reserve(l_Missions.size());
    for (MS::Garrison::GarrisonMission const& mission : l_Missions)
        missions.push_back(ToPacketMission(mission, l_Garrison));

    WorldPackets::Garrison::GetGarrisonInfoResult garrisonInfo;
    garrisonInfo.GarrSiteID = l_Garrison->GetGarrisonSiteLevelEntry()->SiteID;
    garrisonInfo.GarrSiteLevelID = l_Garrison->GetGarrisonSiteLevelEntry()->SiteLevelID;
    garrisonInfo.FactionIndex = l_Garrison->GetGarrisonFactionIndex();
    garrisonInfo.NumFollowerActivationsRemaining = l_Garrison->GetNumFollowerActivationsRemaining();

    garrisonInfo.Buildings.reserve(buildings.size());
    for (WorldPackets::Garrison::GarrisonBuildingInfo const& building : buildings)
        garrisonInfo.Buildings.push_back(&building);

    garrisonInfo.Plots.reserve(plots.size());
    for (WorldPackets::Garrison::GarrisonPlotInfo& plot : plots)
        garrisonInfo.Plots.push_back(&plot);

    garrisonInfo.Followers.reserve(followers.size());
    for (WorldPackets::Garrison::GarrisonFollower const& follower : followers)
        garrisonInfo.Followers.push_back(&follower);

    garrisonInfo.Missions.reserve(missions.size());
    garrisonInfo.CanStartMission.reserve(missions.size());
    for (WorldPackets::Garrison::GarrisonMission const& mission : missions)
    {
        garrisonInfo.Missions.push_back(&mission);
        garrisonInfo.CanStartMission.push_back(mission.MissionState == MS::Garrison::Mission::State::Available);
    }

    garrisonInfo.ArchivedMissions.reserve(l_CompletedMission.size());
    for (MS::Garrison::GarrisonMission const& mission : l_CompletedMission)
        garrisonInfo.ArchivedMissions.push_back(int32(mission.MissionID));

    SendPacket(garrisonInfo.Write());

    WorldPackets::Garrison::GarrisonRequestBlueprintAndSpecializationDataResult blueprintData;
    for (int32 blueprint : l_Garrison->GetKnownBlueprints())
        blueprintData.BlueprintsKnown.push_back(uint32(blueprint));
    for (int32 specialization : l_Garrison->GetKnownSpecializations())
        blueprintData.SpecializationsKnown.push_back(uint32(specialization));
    SendPacket(blueprintData.Write());
}

void WorldSession::HandleRequestGarrisonUpgradeableOpcode(WorldPacket& /*p_RecvData*/)
{
    if (!m_Player)
        return;

    MS::Garrison::Manager* l_Garrison = m_Player->GetGarrison();

    if (!l_Garrison)
        return;

    bool l_CanUpgrade = l_Garrison->CanUpgrade();

    WorldPackets::Garrison::GarrisonIsUpgradeableResult packet;
    packet.Result = uint32(!l_CanUpgrade);
    SendPacket(packet.Write());
}

void WorldSession::HandleUpgradeGarrisonOpcode(WorldPacket& p_RecvData)
{
    if (!m_Player)
        return;

    MS::Garrison::Manager* l_Garrison = m_Player->GetGarrison();

    if (!l_Garrison)
        return;

    uint64 l_NpcGUID = 0;

    p_RecvData.readPackGUID(l_NpcGUID);

    Creature* l_Unit = GetPlayer()->GetNPCIfCanInteractWithFlag2(l_NpcGUID, UNIT_NPC_FLAG2_GARRISON_ARCHITECT);

    if (!l_Unit)
    {
        TC_LOG_DEBUG("network", "WORLD: HandleUpgradeGarrisonOpcode - Unit (GUID: %u) not found or you can not interact with him.", uint32(GUID_LOPART(l_NpcGUID)));
        return;
    }

    if (!l_Garrison->CanUpgrade())
    {
        TC_LOG_DEBUG("network", "WORLD: HandleUpgradeGarrisonOpcode - Can't upgrade");
        return;
    }

    l_Garrison->Upgrade();
}

void WorldSession::HandleRequestLandingPageShipmentInfoOpcode(WorldPacket& /*p_RecvData*/)
{
    if (!m_Player)
        return;

    MS::Garrison::Manager* l_Garrison = m_Player->GetGarrison();

    if (!l_Garrison || !l_Garrison->GetGarrisonSiteLevelEntry())
        return;

    std::vector<MS::Garrison::GarrisonWorkOrder> l_WorkOrders = l_Garrison->GetWorkOrders();

    WorldPackets::Garrison::GarrisonLandingPage packet;
    packet.Shipments.reserve(l_WorkOrders.size());
    for (MS::Garrison::GarrisonWorkOrder const& order : l_WorkOrders)
        packet.Shipments.push_back(ToPacketShipment(l_Garrison, order, true));

    SendPacket(packet.Write());
}

void WorldSession::HandleGarrisonMissionNPCHelloOpcode(WorldPacket& p_RecvData)
{
    if (!m_Player)
        return;

    MS::Garrison::Manager* l_Garrison = m_Player->GetGarrison();

    if (!l_Garrison)
        return;

    uint64 l_NpcGUID = 0;

    p_RecvData.readPackGUID(l_NpcGUID);

    Creature* l_Unit = GetPlayer()->GetNPCIfCanInteractWithFlag2(l_NpcGUID, UNIT_NPC_FLAG2_GARRISON_MISSION_NPC);

    if (!l_Unit)
        l_Unit = GetPlayer()->GetNPCIfCanInteractWithFlag2(l_NpcGUID, UNIT_NPC_FLAG2_SHIPYARD_MISSION_NPC);

    if (!l_Unit)
    {
        TC_LOG_DEBUG("network", "WORLD: HandleGarrisonMissionNPCHelloOpcode - Unit (GUID: %u) not found or you can not interact with him.", uint32(GUID_LOPART(l_NpcGUID)));
        return;
    }

    SendGarrisonOpenMissionNpc(l_NpcGUID);
}

void WorldSession::HandleGarrisonRequestSetMissionNPC(WorldPacket& p_RecvData)
{
    if (!m_Player)
        return;

    MS::Garrison::Manager* l_Garrison = m_Player->GetGarrison();

    if (!l_Garrison)
        return;

    uint64 l_NpcGUID = 0;

    p_RecvData.readPackGUID(l_NpcGUID);

    Creature* l_Unit = GetPlayer()->GetNPCIfCanInteractWithFlag2(l_NpcGUID, UNIT_NPC_FLAG2_GARRISON_MISSION_NPC);

    if (!l_Unit)
    {
        TC_LOG_DEBUG("network", "WORLD: HandleGarrisonRequestSetMissionNPC - Unit (GUID: %u) not found or you can not interact with him.", uint32(GUID_LOPART(l_NpcGUID)));
        return;
    }

    SendGarrisonSetMissionNpc(l_NpcGUID);
}

void WorldSession::HandleGarrisonRequestBuildingsOpcode(WorldPacket& /*p_RecvData*/)
{
    if (!m_Player)
        return;

    MS::Garrison::Manager* l_Garrison = m_Player->GetGarrison();

    if (!l_Garrison || !l_Garrison->GetGarrisonSiteLevelEntry())
        return;

    std::vector<MS::Garrison::GarrisonBuilding> l_Buildings = l_Garrison->GetBuildings();

    WorldPackets::Garrison::GarrisonBuildingLandmarks packet;
    packet.Landmarks.reserve(l_Buildings.size());

    for (uint32 l_I = 0; l_I < l_Buildings.size(); ++l_I)
    {
        MS::Garrison::GarrisonPlotInstanceInfoLocation l_PlotLocation = l_Garrison->GetPlot(l_Buildings[l_I].PlotInstanceID);
        uint32 l_SiteLevelPlotInstanceID = 0;
        uint32 l_BuildingPlotInstanceID  = 0;

        for (uint32 l_Y = 0; l_Y < sGarrSiteLevelPlotInstStore.GetNumRows(); ++l_Y)
        {
            const GarrSiteLevelPlotInstEntry* l_Entry = sGarrSiteLevelPlotInstStore.LookupEntry(l_Y);

            if (l_Entry && l_Entry->PlotInstanceID == l_Buildings[l_I].PlotInstanceID && l_Entry->SiteLevelID == l_Garrison->GetGarrisonSiteLevelEntry()->SiteLevelID)
            {
                l_SiteLevelPlotInstanceID = l_Entry->ID;
                break;
            }
        }

        for (uint32 l_Y = 0; l_Y < sGarrBuildingPlotInstStore.GetNumRows(); ++l_Y)
        {
            const GarrBuildingPlotInstEntry* l_Entry = sGarrBuildingPlotInstStore.LookupEntry(l_Y);

            if (l_Entry && l_Entry->SiteLevelPlotInstID == l_SiteLevelPlotInstanceID && l_Entry->BuildingID == l_Buildings[l_I].BuildingID)
            {
                l_BuildingPlotInstanceID = l_Entry->ID;
                break;
            }
        }

        Position pos;
        pos.Relocate(l_PlotLocation.X, l_PlotLocation.Y, l_PlotLocation.Z);
        packet.Landmarks.emplace_back(l_BuildingPlotInstanceID, pos);
    }

    SendPacket(packet.Write());
}

void WorldSession::HandleGarrisonPurchaseBuildingOpcode(WorldPacket& p_RecvData)
{
    if (!m_Player)
        return;

    MS::Garrison::Manager* l_Garrison = m_Player->GetGarrison();

    if (!l_Garrison)
        return;

    uint64 l_NpcGUID = 0;

    uint32 l_BuildingID     = 0;
    uint32 l_PlotInstanceID = 0;

    p_RecvData.readPackGUID(l_NpcGUID);
    p_RecvData >> l_PlotInstanceID;
    p_RecvData >> l_BuildingID;

    sGarrisonBuildingManager->LearnAllowedBuildings(m_Player, l_Garrison);

    Creature* l_Unit = GetPlayer()->GetNPCIfCanInteractWithFlag2(l_NpcGUID, UNIT_NPC_FLAG2_GARRISON_ARCHITECT);

    if (!l_Unit)
    {
        TC_LOG_DEBUG("network", "WORLD: HandleGarrisonPurchaseBuildingOpcode - Unit (GUID: %u) not found or you can not interact with him.", uint32(GUID_LOPART(l_NpcGUID)));
        return;
    }

    MS::Garrison::PurchaseBuildingResults::Type l_Result = MS::Garrison::PurchaseBuildingResults::Ok;

    if (!sGarrBuildingStore.LookupEntry(l_BuildingID))
        l_Result = MS::Garrison::PurchaseBuildingResults::InvalidBuildingID;

    if (!l_Result && !sGarrPlotInstanceStore.LookupEntry(l_PlotInstanceID))
        l_Result = MS::Garrison::PurchaseBuildingResults::InvalidPlot;

    if (!l_Result && !l_Garrison->KnownBlueprint(l_BuildingID))
        l_Result = MS::Garrison::PurchaseBuildingResults::RequireBluePrint;

    if (!l_Result && l_Garrison->GetBuildingWithBuildingID(l_BuildingID).BuildingID != 0)
        l_Result = MS::Garrison::PurchaseBuildingResults::BuildingExists;

    if (!l_Result && !l_Garrison->IsBuildingPlotInstanceValid(l_BuildingID, l_PlotInstanceID))
        l_Result = MS::Garrison::PurchaseBuildingResults::InvalidPlotBuilding;

    if (!l_Result)
        l_Result = l_Garrison->CanPurchaseBuilding(l_BuildingID);

    if (!sGarrisonBuildingManager->IsBluePrintAllowedForPurchasingBuilding(l_BuildingID, m_Player))
    {
        l_Result = MS::Garrison::PurchaseBuildingResults::InvalidBuildingID;

        std::string l_Message = "Building not available yet";
        switch (m_Player->GetSession()->GetSessionDbcLocale())
        {
            case LocaleConstant::LOCALE_frFR:
                l_Message = "Batiment non disponible";
                break;

            case LocaleConstant::LOCALE_esES:
            case LocaleConstant::LOCALE_esMX:
                l_Message = "Edificio no disponible";
                break;

            default:
                break;
        }

        WorldPacket l_Data(SMSG_PRINT_NOTIFICATION, 2 + l_Message.size());
        l_Data.WriteBits(l_Message.size(), 12);
        l_Data.FlushBits();
        l_Data.WriteString(l_Message);

        SendPacket(&l_Data);
    }

    WorldPackets::Garrison::GarrisonPlaceBuildingResult placeResult;
    placeResult.Result = l_Result;
    if (l_Result == MS::Garrison::PurchaseBuildingResults::Ok)
        placeResult.BuildingInfo = ToPacketBuilding(l_Garrison->PurchaseBuilding(l_BuildingID, l_PlotInstanceID));
    placeResult.PlayActivationCinematic = false;
    SendPacket(placeResult.Write());
}

void WorldSession::HandleGarrisonCancelConstructionOpcode(WorldPacket& p_RecvData)
{
    if (!m_Player)
        return;

    MS::Garrison::Manager* l_Garrison = m_Player->GetGarrison();

    if (!l_Garrison)
        return;

    uint64 l_NpcGUID = 0;

    uint32 l_PlotInstanceID = 0;

    p_RecvData.readPackGUID(l_NpcGUID);
    p_RecvData >> l_PlotInstanceID;

    Creature* l_Unit = GetPlayer()->GetNPCIfCanInteractWithFlag2(l_NpcGUID, UNIT_NPC_FLAG2_GARRISON_ARCHITECT);

    if (!l_Unit)
    {
        TC_LOG_DEBUG("network", "WORLD: HandleGarrisonCancelConstructionOpcode - Unit (GUID: %u) not found or you can not interact with him.", uint32(GUID_LOPART(l_NpcGUID)));
        return;
    }

    l_Garrison->CancelConstruction(l_PlotInstanceID);
}

void WorldSession::HandleGarrisonStartMissionOpcode(WorldPacket& p_RecvData)
{
    if (!m_Player)
        return;

    MS::Garrison::Manager* l_Garrison = m_Player->GetGarrison();

    if (!l_Garrison)
        return;

    uint64 l_NpcGUID        = 0;
    uint32 l_FollowerCount  = 0;
    uint32 l_MissionID      = 0;

    std::vector<uint64> l_Followers;

    p_RecvData.readPackGUID(l_NpcGUID);
    p_RecvData >> l_FollowerCount;
    p_RecvData >> l_MissionID;

    for (uint32 l_I = 0; l_I < l_FollowerCount; ++l_I)
    {
        uint64 l_FollowerDBID = 0;
        p_RecvData >> l_FollowerDBID;

        l_Followers.push_back(l_FollowerDBID);
    }

    Creature* l_Unit = GetPlayer()->GetNPCIfCanInteractWithFlag2(l_NpcGUID, UNIT_NPC_FLAG2_GARRISON_MISSION_NPC);

    if (!l_Unit)
    {
        TC_LOG_DEBUG("network", "WORLD: HandleGarrisonStartMissionOpcode - Unit (GUID: %u) not found or you can not interact with him.", uint32(GUID_LOPART(l_NpcGUID)));
        return;
    }

    l_Garrison->StartMission(l_MissionID, l_Followers);
}

void WorldSession::HandleGarrisonCompleteMissionOpcode(WorldPacket& p_RecvData)
{
    if (!m_Player)
        return;

    MS::Garrison::Manager* l_Garrison = m_Player->GetGarrison();

    if (!l_Garrison)
        return;

    uint64 l_NpcGUID   = 0;
    uint32 l_MissionID = 0;

    p_RecvData.readPackGUID(l_NpcGUID);
    p_RecvData >> l_MissionID;

    Creature* l_Unit = GetPlayer()->GetNPCIfCanInteractWithFlag2(l_NpcGUID, UNIT_NPC_FLAG2_GARRISON_MISSION_NPC);

    if (!l_Unit)
    {
        TC_LOG_DEBUG("network", "WORLD: HandleGarrisonCompleteMissionOpcode - Unit (GUID: %u) not found or you can not interact with him.", uint32(GUID_LOPART(l_NpcGUID)));
        return;
    }
    
    l_Garrison->CompleteMission(l_MissionID);
}

void WorldSession::HandleGarrisonMissionBonusRollOpcode(WorldPacket& p_RecvData)
{
    if (!m_Player)
        return;

    MS::Garrison::Manager* l_Garrison = m_Player->GetGarrison();

    if (!l_Garrison)
        return;

    uint64 l_NpcGUID    = 0;
    uint32 l_MissionID  = 0;

    p_RecvData.readPackGUID(l_NpcGUID);
    p_RecvData >> l_MissionID;

    Creature* l_Unit = GetPlayer()->GetNPCIfCanInteractWithFlag2(l_NpcGUID, UNIT_NPC_FLAG2_GARRISON_MISSION_NPC);

    if (!l_Unit)
    {
        TC_LOG_DEBUG("network", "WORLD: HandleGarrisonMissionBonusRollOpcode - Unit (GUID: %u) not found or you can not interact with him.", uint32(GUID_LOPART(l_NpcGUID)));
        return;
    }

    l_Garrison->DoMissionBonusRoll(l_NpcGUID, l_MissionID);
}

void WorldSession::HandleGarrisonGenerateRecruitsOpcode(WorldPacket& p_RecvData)
{
    if (m_Player == nullptr)
        return;

    MS::Garrison::Manager* l_Garrison = m_Player->GetGarrison();

    if (l_Garrison == nullptr)
        return;

    uint64 l_GUID      = 0;
    uint32 l_TraitID   = 0;
    uint32 l_AbilityID = 0;

    p_RecvData.readPackGUID(l_GUID);
    p_RecvData >> l_AbilityID;
    p_RecvData >> l_TraitID;

    Creature* l_Unit = GetPlayer()->GetNPCIfCanInteractWith(l_GUID, 0);

    if (l_Unit == nullptr)
    {
        TC_LOG_DEBUG("network", "WORLD: HandleGarrisonMissionBonusRollOpcode - Unit (GUID: %u) not found or you can not interact with him.", uint32(GUID_LOPART(l_GUID)));
        return;
    }

    if (l_Unit->ToGarrisonNPCAI())
        l_Unit->ToGarrisonNPCAI()->SendRecruitmentFollowersGenerated(m_Player, l_AbilityID ? l_AbilityID : l_TraitID, 0, l_TraitID ? true : false);
}

void WorldSession::HandleGarrisonSetRecruitmentPreferencesOpcode(WorldPacket& p_RecvData)
{
    if (m_Player == nullptr)
        return;

    MS::Garrison::Manager* l_Garrison = m_Player->GetGarrison();

    if (l_Garrison == nullptr)
        return;
    

    uint64 l_GUID      = 0; ///< Unused ?
    uint32 l_TraitID   = 0;
    uint32 l_AbilityID = 0;

    p_RecvData.readPackGUID(l_GUID);

    p_RecvData >> l_AbilityID;
    p_RecvData >> l_TraitID;
}

void WorldSession::HandleGarrisonRecruitFollower(WorldPacket& p_RecvData)
{
    if (m_Player == nullptr)
        return;

    MS::Garrison::Manager* l_Garrison = m_Player->GetGarrison();

    if (l_Garrison == nullptr)
        return;

    uint64 l_GUID       = 0;

    p_RecvData.readPackGUID(l_GUID);
    uint32 l_FollowerID = 0;
    p_RecvData >> l_FollowerID;

    Creature* l_Unit = m_Player->GetNPCIfCanInteractWith(l_GUID, 0);

    if (l_Unit == nullptr)
    {
        TC_LOG_DEBUG("network", "WORLD: HandleGarrisonMissionBonusRollOpcode - Unit (GUID: %u) not found or you can not interact with him.", uint32(GUID_LOPART(l_GUID)));
        return;
    }

    WorldPacket l_RecruitmentResult(SMSG_GARRISON_RECRUIT_FOLLOWER_RESULT, 64);

    if (l_Garrison->CanRecruitFollower())
        l_RecruitmentResult << uint32(MS::Garrison::PurchaseBuildingResults::Ok);
    else
    {
        l_RecruitmentResult << uint32(MS::Garrison::PurchaseBuildingResults::Ok); ///< need to find appropriate error ID
        m_Player->PlayerTalkClass->SendCloseGossip();
        return;
    }

    std::vector<MS::Garrison::GarrisonFollower> l_WeeklyFollowers = l_Garrison->GetWeeklyFollowerRecruits(m_Player);

    for (MS::Garrison::GarrisonFollower l_Follower : l_WeeklyFollowers)
    {
        if (l_Follower.FollowerID == l_FollowerID)
        {
            l_Follower.Write(l_RecruitmentResult);
            l_Garrison->AddFollower(l_Follower);
            l_Garrison->SetCanRecruitFollower(false);
            m_Player->SetCharacterWorldState(CharacterWorldStates::GarrisonTavernBoolCanRecruitFollower, 0);
            break;
        }
    }

    if (m_Player->GetTeamId() == TEAM_ALLIANCE && m_Player->GetQuestStatus(MS::Garrison::Quests::Alliance_TheHeadHunterHarverst) == QUEST_STATUS_INCOMPLETE)
        m_Player->QuestObjectiveSatisfy(39383, 1, 14);
    else if (m_Player->GetTeamId() == TEAM_HORDE && m_Player->GetQuestStatus(MS::Garrison::Quests::Horde_TheHeadHunterHarverst) == QUEST_STATUS_INCOMPLETE)
        m_Player->QuestObjectiveSatisfy(39418, 1, 14);

    m_Player->SendDirectMessage(&l_RecruitmentResult);
    m_Player->PlayerTalkClass->SendCloseGossip();
}

void WorldSession::HandleGarrisonChangeFollowerActivationStateOpcode(WorldPacket& p_RecvData)
{
    if (!m_Player)
        return;

    MS::Garrison::Manager* l_Garrison = m_Player->GetGarrison();

    if (!l_Garrison)
        return;

    uint64  l_FollowerDBID  = 0;
    bool    l_Desactivate   = false;

    p_RecvData >> l_FollowerDBID;
    l_Desactivate = p_RecvData.ReadBit();

    l_Garrison->ChangeFollowerActivationState(l_FollowerDBID, !l_Desactivate);
}

void WorldSession::HandleGarrisonAssignFollowerToBuilding(WorldPacket& p_RecvData)
{
    if (!m_Player)
        return;

    MS::Garrison::Manager* l_GarrisonMgr = m_Player->GetGarrison();

    if (!l_GarrisonMgr)
        return;

    uint64 l_NpcGUID       = 0;
    uint64 l_FollowerDBID  = 0;
    int32 l_PlotInstanceID = 0;

    p_RecvData.readPackGUID(l_NpcGUID);
    l_PlotInstanceID = p_RecvData.read<int32>();
    p_RecvData >> l_FollowerDBID;

    Creature* l_Creature = m_Player->GetNPCIfCanInteractWithFlag2(l_NpcGUID, UNIT_NPC_FLAG2_GARRISON_ARCHITECT);

    if (!l_Creature)
    {
        TC_LOG_DEBUG("network", "WORLD: HandleGarrisonAssignFollowerToBuilding - Unit (GUID: %u) not found or you can not interact with him.", uint32(GUID_LOPART(l_NpcGUID)));
        return;
    }

    if (l_PlotInstanceID)
    {
        l_GarrisonMgr->AssignFollowerToBuilding(l_FollowerDBID, (uint32)l_PlotInstanceID);

        WorldPackets::Garrison::GarrisonAssignFollowerToBuildingResult packet;
        packet.FollowerDBID = l_FollowerDBID;
        packet.Result = 0;
        packet.PlotInstanceID = l_PlotInstanceID;
        SendPacket(packet.Write());
    }
}

void WorldSession::HandleGarrisonRemoveFollowerFromBuilding(WorldPacket& p_RecvData)
{
    if (!m_Player)
        return;

    MS::Garrison::Manager* l_GarrisonMgr = m_Player->GetGarrison();

    if (!l_GarrisonMgr)
        return;

    uint64 l_NpcGUID      = 0;
    uint64 l_FollowerDBID = 0;

    p_RecvData.readPackGUID(l_NpcGUID);
    p_RecvData >> l_FollowerDBID;

    Creature* l_Creature = m_Player->GetNPCIfCanInteractWithFlag2(l_NpcGUID, UNIT_NPC_FLAG2_GARRISON_ARCHITECT);

    if (!l_Creature)
    {
        TC_LOG_DEBUG("network", "WORLD: HandleGarrisonRemoveFollowerFromBuilding - Unit (GUID: %u) not found or you can not interact with him.", uint32(GUID_LOPART(l_NpcGUID)));
        return;
    }

    l_GarrisonMgr->AssignFollowerToBuilding(l_FollowerDBID, 0);

    WorldPackets::Garrison::GarrisonRemoveFollowerFromBuildingResult packet;
    packet.FollowerDBID = l_FollowerDBID;
    packet.Result = 0;
    SendPacket(packet.Write());
}

void WorldSession::HandleGarrisonGetShipmentInfoOpcode(WorldPacket& p_RecvData)
{
    if (!m_Player)
        return;

    MS::Garrison::Manager* l_Garrison = m_Player->GetGarrison();

    if (!l_Garrison || !m_Player->IsInGarrison())
        return;

    uint64 l_NpcGUID = 0;

    p_RecvData.readPackGUID(l_NpcGUID);

    Creature* l_Unit = GetPlayer()->GetNPCIfCanInteractWithFlag2(l_NpcGUID, UNIT_NPC_FLAG2_GARRISON_SHIPMENT_CRAFTER);

    if (!l_Unit)
    {
        TC_LOG_DEBUG("network", "WORLD: HandleGarrisonMissionNPCHelloOpcode - Unit (GUID: %u) not found or you can not interact with him.", uint32(GUID_LOPART(l_NpcGUID)));
        return;
    }

    uint32 l_ShipmentID     = 0;
    uint32 l_OrderAvailable = 0;
    uint32 l_PlotInstanceID = 0;

    l_PlotInstanceID = l_Garrison->GetCreaturePlotInstanceID(l_NpcGUID);

    if (!!l_PlotInstanceID)
    {
        l_OrderAvailable = l_Garrison->GetBuildingMaxWorkOrder(l_PlotInstanceID);

        uint32 l_BuildingID = l_Garrison->GetBuilding(l_PlotInstanceID).BuildingID;

        if (l_BuildingID)
        {
            if (l_Unit->AI())
                l_ShipmentID = l_Unit->AI()->OnShipmentIDRequest(m_Player);

            if (l_ShipmentID == -1) ///< Comparison of integers of different signs: 'uint32' (aka 'unsigned int') and 'int'
                l_ShipmentID = sGarrisonShipmentManager->GetShipmentIDForBuilding(l_BuildingID, m_Player, false);
        }
    }

    bool l_Success = !!l_ShipmentID && !!l_PlotInstanceID;

    WorldPackets::Garrison::GetShipmentInfoResponse response;
    response.Success = l_Success;
    if (l_Success)
    {
        std::vector<MS::Garrison::GarrisonWorkOrder> l_WorkOrders = l_Garrison->GetWorkOrders();
        response.ShipmentID = l_ShipmentID;
        response.MaxShipments = l_OrderAvailable;
        response.PlotInstanceID = l_PlotInstanceID;
        for (MS::Garrison::GarrisonWorkOrder const& order : l_WorkOrders)
        {
            if (order.PlotInstanceID != l_PlotInstanceID)
                continue;
            response.Shipments.push_back(ToPacketShipment(l_Garrison, order, false));
        }
    }

    SendPacket(response.Write());
}

void WorldSession::HandleGarrisonCreateShipmentOpcode(WorldPacket& p_RecvData)
{
    std::function<void(const std::string &)> l_OnError = [this](const std::string & p_Message) -> void
    {
        if (m_Player->GetSession()->GetSecurity() > SEC_PLAYER)
            ChatHandler(m_Player).PSendSysMessage("HandleGarrisonCreateShipmentOpcode => %s", p_Message.c_str());

        WorldPackets::Garrison::CreateShipmentResponse ack;
        ack.ShipmentID = 0;
        ack.ShipmentRecID = 0;
        ack.Result = 1;
        SendPacket(ack.Write());
    };

    if (!m_Player)
        return;

    MS::Garrison::Manager* l_Garrison = m_Player->GetGarrison();

    if (!l_Garrison || !m_Player->IsInGarrison())
        return;

    uint64 l_NpcGUID = 0;
    uint32 l_Count = 0;

    p_RecvData.readPackGUID(l_NpcGUID);
    p_RecvData >> l_Count;

    /// Min 1 work order
    if (!l_Count)
        l_Count = 1;

    Creature* l_Unit = GetPlayer()->GetNPCIfCanInteractWithFlag2(l_NpcGUID, UNIT_NPC_FLAG2_GARRISON_SHIPMENT_CRAFTER);

    if (!l_Unit)
    {
        TC_LOG_DEBUG("network", "WORLD: HandleGarrisonMissionNPCHelloOpcode - Unit (GUID: %u) not found or you can not interact with him.", uint32(GUID_LOPART(l_NpcGUID)));
        return;
    }

    uint32 l_ShipmentID     = 0;
    uint32 l_OrderMax       = 0;
    uint32 l_PlotInstanceID = 0;

    l_PlotInstanceID = l_Garrison->GetCreaturePlotInstanceID(l_NpcGUID);

    if (!!l_PlotInstanceID)
    {
        l_OrderMax = l_Garrison->GetBuildingMaxWorkOrder(l_PlotInstanceID);

        uint32 l_BuildingID = l_Garrison->GetBuilding(l_PlotInstanceID).BuildingID;

        if (l_BuildingID)
        {
            l_ShipmentID = sGarrisonShipmentManager->GetShipmentIDForBuilding(l_BuildingID, m_Player, true);

            if (l_ShipmentID == MS::Garrison::Barn::ShipmentIDS::ShipmentFurredBeast)
            {
                if (l_Unit->AI())
                    l_ShipmentID = l_Unit->AI()->OnShipmentIDRequest(m_Player);
            }

            sScriptMgr->OnShipmentCreated(m_Player, l_Unit, l_BuildingID);
        }
    }

    if (!l_ShipmentID || !l_PlotInstanceID)
    {
        l_OnError("Invalid ShipmentID or PlotInstanceID");
        return;
    }

    for (uint32 l_OrderI = 0; l_OrderI < l_Count; ++l_OrderI)
    {
        if (((int32)l_OrderMax - (int32)l_Garrison->GetWorkOrderCount(l_PlotInstanceID)) < 1)
        {
            l_OnError("Max work order for this building reached");
            return;
        }

        const CharShipmentEntry* l_ShipmentEntry = sCharShipmentStore.LookupEntry(l_ShipmentID);

        if (!l_ShipmentEntry)
        {
            l_OnError("Shipment entry not found");
            return;
        }

        const SpellInfo* l_Spell = sSpellMgr->GetSpellInfo(l_ShipmentEntry->SpellID);

        if (!l_Spell)
        {
            l_OnError("Shipment spell not found");
            return;
        }

        bool l_HasReagents = true;
        for (uint32 l_I = 0; l_I < MAX_SPELL_REAGENTS; ++l_I)
        {
            uint32 l_ItemEntry = l_Spell->Reagent[l_I];
            uint32 l_ItemCount = l_Spell->ReagentCount[l_I];

            if (!l_ItemEntry || !l_ItemCount)
                continue;

            if (!m_Player->HasItemCount(l_ItemEntry, l_ItemCount))
                l_HasReagents = false;
        }

        if (l_Spell->CurrencyID)
        {
            if (!m_Player->HasCurrency(l_Spell->CurrencyID, l_Spell->CurrencyCount))
                l_HasReagents = false;
        }

        if (!l_HasReagents)
        {
            l_OnError("Doesn't have reagents");
            return;
        }

        for (uint32 l_I = 0; l_I < MAX_SPELL_REAGENTS; ++l_I)
        {
            uint32 l_ItemEntry = l_Spell->Reagent[l_I];
            uint32 l_ItemCount = l_Spell->ReagentCount[l_I];

            if (!l_ItemEntry || !l_ItemCount)
                continue;

            m_Player->DestroyItemCount(l_ItemEntry, l_ItemCount, true);
        }

        if (l_Spell->CurrencyID)
            m_Player->ModifyCurrency(l_Spell->CurrencyID, -int32(l_Spell->CurrencyCount), false);

        m_Player->CastSpell(m_Player, l_Spell, TRIGGERED_FULL_MASK);

        uint64 l_DatabaseID = l_Garrison->StartWorkOrder(l_PlotInstanceID, l_ShipmentID);

        WorldPackets::Garrison::CreateShipmentResponse ack;
        ack.ShipmentID = l_DatabaseID;
        ack.ShipmentRecID = l_ShipmentID;
        ack.Result = l_DatabaseID ? 0 : 1;
        SendPacket(ack.Write());
    }
}

void WorldSession::HandleGarrisonFollowerRename(WorldPacket& p_RecvData)
{
    uint64 l_DatabaseID;
    uint32 l_NameLen;
    std::string l_Name;

    p_RecvData >> l_DatabaseID;
    l_NameLen = p_RecvData.ReadBits(7);
    l_Name = p_RecvData.ReadString(l_NameLen);

    MS::Garrison::Manager* l_Garrison = m_Player->GetGarrison();

    if (!l_Garrison)
        return;

    l_Garrison->RenameFollower(l_DatabaseID, l_Name);
}

void WorldSession::HandleGarrisonDecommisionShip(WorldPacket& p_RecvData)
{
    uint64 l_Guid, l_DatabaseID;

    p_RecvData.readPackGUID(l_Guid);
    p_RecvData >> l_DatabaseID;

    Creature* l_NPC = m_Player->GetNPCIfCanInteractWithFlag2(l_Guid, UNIT_NPC_FLAG2_SHIPYARD_MISSION_NPC);

    if (!l_NPC)
        return;

    MS::Garrison::Manager* l_Garrison = m_Player->GetGarrison();

    if (!l_Garrison)
        return;

    l_Garrison->RemoveFollower(l_DatabaseID);
}

void WorldSession::HandlePlayerJoinedFriendlyGarrison(WorldPacket& p_RecvData)
{
	bool Enabled = p_RecvData.ReadBit();

	if (Enabled)
	{
		Group* group = m_Player->GetGroup();

		if (!group)
			return;

		uint32 leaderGuid = group->GetLeaderGUID();
		if (Player* leader = ObjectAccessor::FindPlayer(leaderGuid))
		{
			MS::Garrison::Manager* l_Garrison = leader->GetGarrison();
			if (leader == m_Player)
				return;

			if (!l_Garrison)
				return;

			Map* map = leader->GetMap();

			InstancePlayerBind* bind = leader->GetBoundInstance(leader->GetMapId(), leader->GetDifficultyID(map->GetEntry()));

			if (InstanceSave* save = sInstanceSaveMgr->GetInstanceSave(leader->GetInstanceId()))
				m_Player->BindToInstance(save, !save->CanReset());

			float x, y, z;
			leader->GetContactPoint(m_Player, x, y, z);
			m_Player->TeleportTo(leader->GetMapId(), x, y, z, m_Player->GetAngle(leader), TELE_TO_SEAMLESS);
			m_Player->SetPhaseMask(leader->GetPhaseMask(), true);
			m_Player->SetInPartyGarrison();
			m_Player->SetUInt32Value(PLAYER_FIELD_LOCAL_FLAGS, PLAYER_LOCAL_FLAG_USING_PARTY_GARRISON);
		}
	}
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void WorldSession::SendGarrisonOpenArchitect(uint64 p_CreatureGUID)
{
    MS::Garrison::Manager* l_Garrison = m_Player->GetGarrison();
    if (!l_Garrison)
        return;

    WorldPackets::Garrison::GarrisonOpenArchitect packet;
    packet.NpcGUID = ObjectGuid(p_CreatureGUID);
    SendPacket(packet.Write());
}
void WorldSession::SendGarrisonOpenMissionNpc(uint64 p_CreatureGUID)
{
    MS::Garrison::Manager* l_Garrison = m_Player->GetGarrison();

    if (!l_Garrison)
        return;

    WorldPackets::Garrison::GarrisonOpenMissionNpc packet;
    packet.NpcGUID = ObjectGuid(p_CreatureGUID);
    SendPacket(packet.Write());
}

void WorldSession::SendGarrisonSetMissionNpc(uint64 p_CreatureGUID)
{
    Creature* l_Creature = sObjectAccessor->FindCreature(p_CreatureGUID);

    WorldPacket l_Data(SMSG_GARRISON_SET_MISSION_NPC, 22);
    l_Data.appendPackGUID(p_CreatureGUID);
    l_Data << uint32(l_Creature && l_Creature->HasFlag(UNIT_FIELD_NPC_FLAGS + 1, UNIT_NPC_FLAG2_SHIPYARD_MISSION_NPC) ? MS::Garrison::FollowerType::Ship : MS::Garrison::FollowerType::NPC);
    SendPacket(&l_Data);
}
#endif