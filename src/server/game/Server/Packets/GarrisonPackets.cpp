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

#include "GarrisonPackets.h"

WorldPacket const* WorldPackets::Garrison::GarrisonCreateResult::Write()
{
    _worldPacket << uint32(Result);
    _worldPacket << uint32(GarrSiteLevelID);

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Garrison::GarrisonDeleteResult::Write()
{
    _worldPacket << uint32(Result);
    _worldPacket << uint32(GarrSiteID);

    return &_worldPacket;
}

ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::Garrison::GarrisonPlotInfo& plotInfo)
{
    data << uint32(plotInfo.GarrPlotInstanceID);
    data << plotInfo.PlotPos.PositionXYZOStream();
    data << uint32(plotInfo.PlotType);

    return data;
}

ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::Garrison::GarrisonBuildingInfo const& buildingInfo)
{
    data << uint32(buildingInfo.GarrPlotInstanceID);
    data << uint32(buildingInfo.GarrBuildingID);
    data << uint32(buildingInfo.TimeBuilt);
    data << uint32(buildingInfo.CurrentGarSpecID);
    data << uint32(buildingInfo.TimeSpecCooldown);
    data.WriteBit(buildingInfo.Active);
    data.FlushBits();

    return data;
}

ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::Garrison::GarrisonFollower const& follower)
{
    data << uint64(follower.DbID);
    data << uint32(follower.GarrFollowerID);
    data << uint32(follower.Quality);
    data << uint32(follower.FollowerLevel);
    data << uint32(follower.ItemLevelWeapon);
    data << uint32(follower.ItemLevelArmor);
    data << uint32(follower.Xp);
    data << uint32(follower.CurrentBuildingID);
    data << uint32(follower.CurrentMissionID);
    data << uint32(follower.AbilityID.size());
    data << uint32(follower.FollowerStatus);
    for (uint32 ability : follower.AbilityID)
        data << uint32(ability);

    data.WriteBits(follower.CustomName.length(), 7);
    data.FlushBits();
    data.WriteString(follower.CustomName);

    return data;
}

ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::Garrison::GarrisonMission const& mission)
{
    data << uint64(mission.DbID);
    data << uint32(mission.MissionRecID);
    data << uint32(mission.OfferTime);
    data << uint32(mission.OfferDuration);
    data << uint32(mission.StartTime);
    data << uint32(mission.TravelDuration);
    data << uint32(mission.MissionDuration);
    data << uint32(mission.MissionState);

    return data;
}

ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::Garrison::GarrisonMissionAreaBonus const& areaBonus)
{
    data << uint32(areaBonus.GarrMssnBonusAbilityID);
    data << uint32(areaBonus.StartTime);

    return data;
}

WorldPacket const* WorldPackets::Garrison::GetGarrisonInfoResult::Write()
{
    _worldPacket.reserve(4 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 4 +
        Buildings.size() * sizeof(GarrisonBuildingInfo) +
        Plots.size() * sizeof(GarrisonPlotInfo) +
        Followers.size() * (sizeof(GarrisonFollower) + 5 * 4) +
        Missions.size() * sizeof(GarrisonMission) +
        MissionAreaBonuses.size() * sizeof(GarrisonMissionAreaBonus) +
        ArchivedMissions.size() * 4 +
        CanStartMission.size());

    _worldPacket << int32(GarrSiteID);
    _worldPacket << int32(GarrSiteLevelID);
    _worldPacket << int32(FactionIndex);
    _worldPacket << uint32(Buildings.size());
    _worldPacket << uint32(Plots.size());
    _worldPacket << uint32(Followers.size());
    _worldPacket << uint32(Missions.size());
    _worldPacket << uint32(CanStartMission.size());
    _worldPacket << uint32(MissionAreaBonuses.size());
    _worldPacket << uint32(ArchivedMissions.size());
    _worldPacket << int32(NumFollowerActivationsRemaining);

    for (GarrisonBuildingInfo const* building : Buildings)
        _worldPacket << *building;

    for (GarrisonPlotInfo* plot : Plots)
        _worldPacket << *plot;

    for (GarrisonFollower const* follower : Followers)
        _worldPacket << *follower;

    for (GarrisonMission const* mission : Missions)
        _worldPacket << *mission;

    for (GarrisonMissionAreaBonus const* areaBonus : MissionAreaBonuses)
        _worldPacket << *areaBonus;

    if (!ArchivedMissions.empty())
        _worldPacket.append(ArchivedMissions.data(), ArchivedMissions.size());

    for (bool canStartMission : CanStartMission)
        _worldPacket.WriteBit(canStartMission);

    _worldPacket.FlushBits();

    return &_worldPacket;
}

ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::Garrison::GarrisonRemoteBuildingInfo const& building)
{
    data << uint32(building.GarrPlotInstanceID);
    data << uint32(building.GarrBuildingID);

    return data;
}

ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::Garrison::GarrisonRemoteSiteInfo const& site)
{
    data << uint32(site.GarrSiteLevelID);
    data << uint32(site.Buildings.size());
    for (WorldPackets::Garrison::GarrisonRemoteBuildingInfo const& building : site.Buildings)
        data << building;

    return data;
}

WorldPacket const* WorldPackets::Garrison::GarrisonRemoteInfo::Write()
{
    _worldPacket << uint32(Sites.size());
    for (GarrisonRemoteSiteInfo const& site : Sites)
        _worldPacket << site;

    return &_worldPacket;
}

void WorldPackets::Garrison::GarrisonPurchaseBuilding::Read()
{
    _worldPacket >> NpcGUID;
    _worldPacket >> PlotInstanceID;
    _worldPacket >> BuildingID;
}

WorldPacket const* WorldPackets::Garrison::GarrisonPlaceBuildingResult::Write()
{
    _worldPacket << uint32(Result);
    _worldPacket << BuildingInfo;
    _worldPacket.WriteBit(PlayActivationCinematic);
    _worldPacket.FlushBits();

    return &_worldPacket;
}

void WorldPackets::Garrison::GarrisonCancelConstruction::Read()
{
    _worldPacket >> NpcGUID;
    _worldPacket >> PlotInstanceID;
}

WorldPacket const* WorldPackets::Garrison::GarrisonBuildingRemoved::Write()
{
    _worldPacket << uint32(Result);
    _worldPacket << uint32(GarrPlotInstanceID);
    _worldPacket << uint32(GarrBuildingID);

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Garrison::GarrisonLearnBlueprintResult::Write()
{
    _worldPacket << uint32(Result);
    _worldPacket << uint32(BuildingID);

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Garrison::GarrisonUnlearnBlueprintResult::Write()
{
    _worldPacket << uint32(Result);
    _worldPacket << uint32(BuildingID);

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Garrison::GarrisonRequestBlueprintAndSpecializationDataResult::Write()
{
    _worldPacket << uint32(BlueprintsKnown.size());
    _worldPacket << uint32(SpecializationsKnown.size());
    for (uint32 blueprint : BlueprintsKnown)
        _worldPacket << uint32(blueprint);

    for (uint32 specialization : SpecializationsKnown)
        _worldPacket << uint32(specialization);

    return &_worldPacket;
}

ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::Garrison::GarrisonBuildingLandmark& landmark)
{
    data << uint32(landmark.GarrBuildingPlotInstID);
    data << landmark.Pos.PositionXYZStream();

    return data;
}

WorldPacket const* WorldPackets::Garrison::GarrisonBuildingLandmarks::Write()
{
    _worldPacket << uint32(Landmarks.size());
    for (GarrisonBuildingLandmark& landmark : Landmarks)
        _worldPacket << landmark;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Garrison::GarrisonPlotPlaced::Write()
{
    _worldPacket << *PlotInfo;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Garrison::GarrisonPlotRemoved::Write()
{
    _worldPacket << uint32(GarrPlotInstanceID);

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Garrison::GarrisonAddFollowerResult::Write()
{
    _worldPacket << uint32(Result);
    _worldPacket << Follower;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Garrison::GarrisonRemoveFollowerResult::Write()
{
    _worldPacket << uint64(FollowerDBID);
    _worldPacket << uint32(Result);
    _worldPacket << uint32(Destroyed);

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Garrison::GarrisonBuildingActivated::Write()
{
    _worldPacket << uint32(GarrPlotInstanceID);

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Garrison::GarrisonIsUpgradeableResult::Write()
{
    _worldPacket << uint32(Result);
    return &_worldPacket;
}

WorldPacket const* WorldPackets::Garrison::GarrisonUpgradeResult::Write()
{
    _worldPacket << uint32(Result);
    _worldPacket << uint32(GarrSiteLevelID);
    return &_worldPacket;
}

void WorldPackets::Garrison::UpgradeGarrison::Read()
{
    _worldPacket >> NpcGUID;
}

ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::Garrison::Shipment const& shipment)
{
    data << uint32(shipment.ShipmentRecID);
    data << uint64(shipment.ShipmentID);
    data << uint64(shipment.AssignedFollowerDBID);
    data << uint32(shipment.CreationTime);
    data << uint32(shipment.ShipmentDuration);
    data << uint32(shipment.RewardedXP);
    return data;
}

WorldPacket const* WorldPackets::Garrison::GarrisonLandingPage::Write()
{
    _worldPacket << uint32(Shipments.size());
    for (Shipment const& shipment : Shipments)
        _worldPacket << shipment;
    return &_worldPacket;
}

WorldPacket const* WorldPackets::Garrison::GetShipmentInfoResponse::Write()
{
    _worldPacket.WriteBit(Success);
    _worldPacket.FlushBits();

    _worldPacket << uint32(ShipmentID);
    _worldPacket << uint32(MaxShipments);
    _worldPacket << uint32(Shipments.size());
    _worldPacket << uint32(PlotInstanceID);

    for (Shipment const& shipment : Shipments)
        _worldPacket << shipment;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Garrison::CreateShipmentResponse::Write()
{
    _worldPacket << uint64(ShipmentID);
    _worldPacket << uint32(Result);
    _worldPacket << uint32(ShipmentRecID);
    return &_worldPacket;
}

WorldPacket const* WorldPackets::Garrison::GarrisonOpenArchitect::Write()
{
    _worldPacket << NpcGUID;
    return &_worldPacket;
}

WorldPacket const* WorldPackets::Garrison::GarrisonOpenMissionNpc::Write()
{
    _worldPacket << NpcGUID;
    return &_worldPacket;
}

WorldPacket const* WorldPackets::Garrison::GarrisonAssignFollowerToBuildingResult::Write()
{
    _worldPacket << uint64(FollowerDBID);
    _worldPacket << int32(Result);
    _worldPacket << int32(PlotInstanceID);
    return &_worldPacket;
}

WorldPacket const* WorldPackets::Garrison::GarrisonRemoveFollowerFromBuildingResult::Write()
{
    _worldPacket << uint64(FollowerDBID);
    _worldPacket << int32(Result);
    return &_worldPacket;
}

WorldPacket const* WorldPackets::Garrison::GarrisonAddMissionResult::Write()
{
    _worldPacket << uint32(Result);
    _worldPacket << MissionData;
    return &_worldPacket;
}

WorldPacket const* WorldPackets::Garrison::GarrisonStartMissionResult::Write()
{
    _worldPacket << uint32(Result);
    _worldPacket << MissionData;
    _worldPacket << uint32(FollowerDBIDs.size());
    for (uint64 followerDbId : FollowerDBIDs)
        _worldPacket << uint64(followerDbId);
    return &_worldPacket;
}

WorldPacket const* WorldPackets::Garrison::GarrisonCompleteMissionResult::Write()
{
    _worldPacket << uint32(Result);
    _worldPacket << MissionData;
    _worldPacket << uint32(MissionRecID);
    _worldPacket << uint32(0);
    _worldPacket.WriteBit(Succeeded);
    _worldPacket.FlushBits();
    return &_worldPacket;
}

WorldPacket const* WorldPackets::Garrison::GarrisonMissionBonusRollResult::Write()
{
    _worldPacket << MissionData;
    _worldPacket << uint32(MissionRecID);
    _worldPacket << uint32(Result);
    return &_worldPacket;
}

WorldPacket const* WorldPackets::Garrison::GarrisonFollowerChangedItemLevel::Write()
{
    _worldPacket << Follower;
    return &_worldPacket;
}

WorldPacket const* WorldPackets::Garrison::GarrisonFollowerChangedXP::Write()
{
    _worldPacket << uint32(TotalXp);
    _worldPacket << uint32(Result);
    _worldPacket << Follower;
    _worldPacket << Follower2;
    return &_worldPacket;
}

WorldPacket const* WorldPackets::Garrison::GarrisonNumFollowerActivationsRemaining::Write()
{
    _worldPacket << uint32(Amount);
    return &_worldPacket;
}

void WorldPackets::Garrison::GarrisonStartMission::Read()
{
    _worldPacket >> NpcGUID;
    uint32 followerCount = 0;
    _worldPacket >> followerCount;
    _worldPacket >> MissionRecID;
    FollowerDBIDs.resize(followerCount);
    for (uint32 i = 0; i < followerCount; ++i)
        _worldPacket >> FollowerDBIDs[i];
}

void WorldPackets::Garrison::GarrisonCompleteMission::Read()
{
    _worldPacket >> NpcGUID;
    _worldPacket >> MissionRecID;
}

void WorldPackets::Garrison::GarrisonMissionBonusRoll::Read()
{
    _worldPacket >> NpcGUID;
    _worldPacket >> MissionRecID;
}

void WorldPackets::Garrison::GarrisonAssignFollowerToBuilding::Read()
{
    _worldPacket >> NpcGUID;
    _worldPacket >> PlotInstanceID;
    _worldPacket >> FollowerDBID;
}

void WorldPackets::Garrison::GarrisonRemoveFollowerFromBuilding::Read()
{
    _worldPacket >> NpcGUID;
    _worldPacket >> FollowerDBID;
}

void WorldPackets::Garrison::GarrisonSetFollowerInactive::Read()
{
    _worldPacket >> FollowerDBID;
    Inactive = _worldPacket.ReadBit();
}

void WorldPackets::Garrison::CreateShipment::Read()
{
    _worldPacket >> NpcGUID;
    _worldPacket >> Count;
}

void WorldPackets::Garrison::GarrisonRequestShipmentInfo::Read()
{
    _worldPacket >> NpcGUID;
}
