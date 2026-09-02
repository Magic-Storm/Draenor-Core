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

#ifndef GarrisonPackets_h__
#define GarrisonPackets_h__

#include "Packet.h"
#include "ObjectGuid.h"
#include "Position.h"
#include "PacketUtilities.h"
#include <string>
#include <vector>

namespace WorldPackets
{
    namespace Garrison
    {
        class GarrisonCreateResult final : public ServerPacket
        {
        public:
            GarrisonCreateResult() : ServerPacket(SMSG_GARRISON_CREATE_RESULT, 4 + 4) { }

            WorldPacket const* Write() override;

            uint32 GarrSiteLevelID = 0;
            uint32 Result = 0;
        };

        class GarrisonDeleteResult final : public ServerPacket
        {
        public:
            GarrisonDeleteResult() : ServerPacket(SMSG_GARRISON_DELETE_RESULT, 4 + 4) { }

            WorldPacket const* Write() override;

            uint32 Result = 0;
            uint32 GarrSiteID = 0;
        };

        class GetGarrisonInfo final : public ClientPacket
        {
        public:
            GetGarrisonInfo(WorldPacket&& packet) : ClientPacket(CMSG_GET_GARRISON_INFO, std::move(packet)) { }

            void Read() override { }
        };

        struct GarrisonPlotInfo
        {
            uint32 GarrPlotInstanceID = 0;
            Position PlotPos;
            uint32 PlotType = 0;
        };

        struct GarrisonBuildingInfo
        {
            uint32 GarrPlotInstanceID = 0;
            uint32 GarrBuildingID = 0;
            time_t TimeBuilt = time_t(0);
            uint32 CurrentGarSpecID = 0;
            time_t TimeSpecCooldown = time_t(2288912640);   // 06/07/1906 18:35:44 - another in the series of magic blizz dates
            bool Active = false;
        };

        struct GarrisonFollower
        {
            uint64 DbID = 0;
            uint32 GarrFollowerID = 0;
            uint32 Quality = 0;
            uint32 FollowerLevel = 0;
            uint32 ItemLevelWeapon = 0;
            uint32 ItemLevelArmor = 0;
            uint32 Xp = 0;
            uint32 CurrentBuildingID = 0;
            uint32 CurrentMissionID = 0;
            std::vector<uint32> AbilityID;
            uint32 FollowerStatus = 0;
            std::string CustomName;
        };

        struct GarrisonMission
        {
            uint64 DbID = 0;
            uint32 MissionRecID = 0;
            time_t OfferTime = time_t(0);
            uint32 OfferDuration = 0;
            time_t StartTime = time_t(2288912640);
            uint32 TravelDuration = 0;
            uint32 MissionDuration = 0;
            uint32 MissionState = 0;
        };

        struct GarrisonMissionAreaBonus
        {
            uint32 GarrMssnBonusAbilityID = 0;
            time_t StartTime = time_t(0);
        };

        class GetGarrisonInfoResult final : public ServerPacket
        {
        public:
            GetGarrisonInfoResult() : ServerPacket(SMSG_GET_GARRISON_INFO_RESULT) { }

            WorldPacket const* Write() override;

            uint32 GarrSiteID = 0;
            uint32 GarrSiteLevelID = 0;
            uint32 FactionIndex = 0;
            uint32 NumFollowerActivationsRemaining = 0;
            std::vector<GarrisonPlotInfo*> Plots;
            std::vector<GarrisonBuildingInfo const*> Buildings;
            std::vector<GarrisonFollower const*> Followers;
            std::vector<GarrisonMission const*> Missions;
            std::vector<GarrisonMissionAreaBonus const*> MissionAreaBonuses;
            std::vector<bool> CanStartMission;
            std::vector<int32> ArchivedMissions;
        };

        struct GarrisonRemoteBuildingInfo
        {
            GarrisonRemoteBuildingInfo() : GarrPlotInstanceID(0), GarrBuildingID(0) { }
            GarrisonRemoteBuildingInfo(uint32 plotInstanceId, uint32 buildingId) : GarrPlotInstanceID(plotInstanceId), GarrBuildingID(buildingId) { }

            uint32 GarrPlotInstanceID;
            uint32 GarrBuildingID;
        };

        struct GarrisonRemoteSiteInfo
        {
            uint32 GarrSiteLevelID = 0;
            std::vector<GarrisonRemoteBuildingInfo> Buildings;
        };

        class GarrisonRemoteInfo final : public ServerPacket
        {
        public:
            GarrisonRemoteInfo() : ServerPacket(SMSG_GARRISON_REMOTE_INFO) { }

            WorldPacket const* Write() override;

            std::vector<GarrisonRemoteSiteInfo> Sites;
        };

        class GarrisonPurchaseBuilding final : public ClientPacket
        {
        public:
            GarrisonPurchaseBuilding(WorldPacket&& packet) : ClientPacket(CMSG_GARRISON_PURCHASE_BUILDING, std::move(packet)) { }

            void Read() override;

            ObjectGuid NpcGUID;
            uint32 BuildingID = 0;
            uint32 PlotInstanceID = 0;
        };

        class GarrisonPlaceBuildingResult final : public ServerPacket
        {
        public:
            GarrisonPlaceBuildingResult() : ServerPacket(SMSG_GARRISON_PLACE_BUILDING_RESULT) { }

            WorldPacket const* Write() override;

            uint32 Result = 0;
            GarrisonBuildingInfo BuildingInfo;
            bool PlayActivationCinematic = false;
        };

        class GarrisonCancelConstruction final : public ClientPacket
        {
        public:
            GarrisonCancelConstruction(WorldPacket&& packet) : ClientPacket(CMSG_GARRISON_CANCEL_CONSTRUCTION, std::move(packet)) { }

            void Read() override;

            ObjectGuid NpcGUID;
            uint32 PlotInstanceID = 0;
        };

        class GarrisonBuildingRemoved final : public ServerPacket
        {
        public:
            GarrisonBuildingRemoved() : ServerPacket(SMSG_GARRISON_BUILDING_REMOVED, 4 + 4 + 4) { }

            WorldPacket const* Write() override;

            uint32 Result = 0;
            uint32 GarrPlotInstanceID = 0;
            uint32 GarrBuildingID = 0;
        };

        class GarrisonLearnBlueprintResult final : public ServerPacket
        {
        public:
            GarrisonLearnBlueprintResult() : ServerPacket(SMSG_GARRISON_LEARN_BLUEPRINT_RESULT, 4 + 4) { }

            WorldPacket const* Write() override;

            uint32 BuildingID = 0;
            uint32 Result = 0;
        };

        class GarrisonUnlearnBlueprintResult final : public ServerPacket
        {
        public:
            GarrisonUnlearnBlueprintResult() : ServerPacket(SMSG_GARRISON_UNLEARN_BLUEPRINT_RESULT, 4 + 4) { }

            WorldPacket const* Write() override;

            uint32 BuildingID = 0;
            uint32 Result = 0;
        };

        class GarrisonRequestBlueprintAndSpecializationData final : public ClientPacket
        {
        public:
            GarrisonRequestBlueprintAndSpecializationData(WorldPacket&& packet) : ClientPacket(CMSG_GARRISON_REQUEST_BLUEPRINT_AND_SPECIALIZATION_DATA, std::move(packet)) { }

            void Read() override { }
        };

        class GarrisonRequestBlueprintAndSpecializationDataResult final : public ServerPacket
        {
        public:
            GarrisonRequestBlueprintAndSpecializationDataResult() : ServerPacket(SMSG_GARRISON_REQUEST_BLUEPRINT_AND_SPECIALIZATION_DATA_RESULT, 400) { }

            WorldPacket const* Write() override;

            std::vector<uint32> SpecializationsKnown;
            std::vector<uint32> BlueprintsKnown;
        };

        class GarrisonGetBuildingLandmarks final : public ClientPacket
        {
        public:
            GarrisonGetBuildingLandmarks(WorldPacket&& packet) : ClientPacket(CMSG_GARRISON_GET_BUILDING_LANDMARKS, std::move(packet)) { }

            void Read() override { }
        };

        struct GarrisonBuildingLandmark
        {
            GarrisonBuildingLandmark() : GarrBuildingPlotInstID(0), Pos() { }
            GarrisonBuildingLandmark(uint32 buildingPlotInstId, Position const& pos) : GarrBuildingPlotInstID(buildingPlotInstId), Pos(pos) { }

            uint32 GarrBuildingPlotInstID;
            Position Pos;
        };

        class GarrisonBuildingLandmarks final : public ServerPacket
        {
        public:
            GarrisonBuildingLandmarks() : ServerPacket(SMSG_GARRISON_BUILDING_LANDMARKS) { }

            WorldPacket const* Write() override;

            std::vector<GarrisonBuildingLandmark> Landmarks;
        };

        class GarrisonPlotPlaced final : public ServerPacket
        {
        public:
            GarrisonPlotPlaced() : ServerPacket(SMSG_GARRISON_PLOT_PLACED) { }

            WorldPacket const* Write() override;

            GarrisonPlotInfo* PlotInfo = nullptr;
        };

        class GarrisonPlotRemoved final : public ServerPacket
        {
        public:
            GarrisonPlotRemoved() : ServerPacket(SMSG_GARRISON_PLOT_REMOVED, 4) { }

            WorldPacket const* Write() override;

            uint32 GarrPlotInstanceID = 0;
        };

        class GarrisonAddFollowerResult final : public ServerPacket
        {
        public:
            GarrisonAddFollowerResult() : ServerPacket(SMSG_GARRISON_ADD_FOLLOWER_RESULT, 8 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 5 * 4 + 4) { }

            WorldPacket const* Write() override;

            GarrisonFollower Follower;
            uint32 Result = 0;
        };

        class GarrisonRemoveFollowerResult final : public ServerPacket
        {
        public:
            GarrisonRemoveFollowerResult() : ServerPacket(SMSG_GARRISON_REMOVE_FOLLOWER_RESULT, 8 + 4) { }

            WorldPacket const* Write() override;

            uint64 FollowerDBID = 0;
            uint32 Result = 0;
            uint32 Destroyed = 0;
        };

        class GarrisonBuildingActivated final : public ServerPacket
        {
        public:
            GarrisonBuildingActivated() : ServerPacket(SMSG_GARRISON_BUILDING_ACTIVATED, 4) { }

            WorldPacket const* Write() override;

            uint32 GarrPlotInstanceID = 0;
        };

        class GarrisonIsUpgradeableResult final : public ServerPacket
        {
        public:
            GarrisonIsUpgradeableResult() : ServerPacket(SMSG_GARRISON_IS_UPGRADEABLE_RESULT, 4) { }

            WorldPacket const* Write() override;

            uint32 Result = 0;
        };

        class GarrisonUpgradeResult final : public ServerPacket
        {
        public:
            GarrisonUpgradeResult() : ServerPacket(SMSG_GARRISON_UPGRADE_RESULT, 4 + 4) { }

            WorldPacket const* Write() override;

            uint32 Result = 0;
            uint32 GarrSiteLevelID = 0;
        };

        class UpgradeGarrison final : public ClientPacket
        {
        public:
            UpgradeGarrison(WorldPacket&& packet) : ClientPacket(CMSG_UPGRADE_GARRISON, std::move(packet)) { }

            void Read() override;

            ObjectGuid NpcGUID;
        };

        struct Shipment
        {
            uint32 ShipmentRecID = 0;
            uint64 ShipmentID = 0;
            uint64 AssignedFollowerDBID = 0;
            uint32 CreationTime = 0;
            uint32 ShipmentDuration = 0;
            uint32 RewardedXP = 0;
        };

        class GarrisonLandingPage final : public ServerPacket
        {
        public:
            GarrisonLandingPage() : ServerPacket(SMSG_GARRISON_LANDING_PAGE_SHIPMENT_INFO) { }

            WorldPacket const* Write() override;

            std::vector<Shipment> Shipments;
        };

        class GetShipmentInfoResponse final : public ServerPacket
        {
        public:
            GetShipmentInfoResponse() : ServerPacket(SMSG_GET_SHIPMENT_INFO_RESPONSE) { }

            WorldPacket const* Write() override;

            bool Success = false;
            uint32 ShipmentID = 0;
            uint32 MaxShipments = 0;
            uint32 PlotInstanceID = 0;
            std::vector<Shipment> Shipments;
        };

        class CreateShipmentResponse final : public ServerPacket
        {
        public:
            CreateShipmentResponse() : ServerPacket(SMSG_CREATE_SHIPMENT_RESPONSE, 8 + 4 + 4) { }

            WorldPacket const* Write() override;

            uint64 ShipmentID = 0;
            uint32 Result = 0;
            uint32 ShipmentRecID = 0;
        };

        class GarrisonOpenArchitect final : public ServerPacket
        {
        public:
            GarrisonOpenArchitect() : ServerPacket(SMSG_GARRISON_OPEN_ARCHITECT, 18) { }

            WorldPacket const* Write() override;

            ObjectGuid NpcGUID;
        };

        class GarrisonOpenMissionNpc final : public ServerPacket
        {
        public:
            GarrisonOpenMissionNpc() : ServerPacket(SMSG_GARRISON_OPEN_MISSION_NPC, 18) { }

            WorldPacket const* Write() override;

            ObjectGuid NpcGUID;
        };

        class GarrisonAssignFollowerToBuildingResult final : public ServerPacket
        {
        public:
            GarrisonAssignFollowerToBuildingResult() : ServerPacket(SMSG_GARRISON_ASSIGN_FOLLOWER_TO_BUILDING_RESULT, 8 + 4 + 4) { }

            WorldPacket const* Write() override;

            uint64 FollowerDBID = 0;
            int32 Result = 0;
            int32 PlotInstanceID = 0;
        };

        class GarrisonRemoveFollowerFromBuildingResult final : public ServerPacket
        {
        public:
            GarrisonRemoveFollowerFromBuildingResult() : ServerPacket(SMSG_GARRISON_REMOVE_FOLLOWER_FROM_BUILDING_RESULT, 8 + 4) { }

            WorldPacket const* Write() override;

            uint64 FollowerDBID = 0;
            int32 Result = 0;
        };

        class GarrisonAddMissionResult final : public ServerPacket
        {
        public:
            GarrisonAddMissionResult() : ServerPacket(SMSG_GARRISON_ADD_MISSION_RESULT) { }

            WorldPacket const* Write() override;

            uint32 Result = 0;
            GarrisonMission MissionData;
        };

        class GarrisonStartMissionResult final : public ServerPacket
        {
        public:
            GarrisonStartMissionResult() : ServerPacket(SMSG_GARRISON_START_MISSION_RESULT) { }

            WorldPacket const* Write() override;

            uint32 Result = 0;
            GarrisonMission MissionData;
            std::vector<uint64> FollowerDBIDs;
        };

        class GarrisonCompleteMissionResult final : public ServerPacket
        {
        public:
            GarrisonCompleteMissionResult() : ServerPacket(SMSG_GARRISON_COMPLETE_MISSION_RESULT) { }

            WorldPacket const* Write() override;

            uint32 Result = 0;
            GarrisonMission MissionData;
            uint32 MissionRecID = 0;
            bool Succeeded = false;
        };

        class GarrisonMissionBonusRollResult final : public ServerPacket
        {
        public:
            GarrisonMissionBonusRollResult() : ServerPacket(SMSG_GARRISON_MISSION_BONUS_ROLL_RESULT) { }

            WorldPacket const* Write() override;

            GarrisonMission MissionData;
            uint32 MissionRecID = 0;
            uint32 Result = 0;
        };

        class GarrisonFollowerChangedItemLevel final : public ServerPacket
        {
        public:
            GarrisonFollowerChangedItemLevel() : ServerPacket(SMSG_GARRISON_FOLLOWER_CHANGED_ITEM_LEVEL) { }

            WorldPacket const* Write() override;

            GarrisonFollower Follower;
        };

        class GarrisonFollowerChangedXP final : public ServerPacket
        {
        public:
            GarrisonFollowerChangedXP() : ServerPacket(SMSG_GARRISON_FOLLOWER_CHANGED_XP) { }

            WorldPacket const* Write() override;

            uint32 TotalXp = 0;
            uint32 Result = 0;
            GarrisonFollower Follower;
            GarrisonFollower Follower2;
        };

        class GarrisonNumFollowerActivationsRemaining final : public ServerPacket
        {
        public:
            GarrisonNumFollowerActivationsRemaining() : ServerPacket(SMSG_GARRISON_NUM_FOLLOWER_ACTIVATIONS_REMAINING, 4) { }

            WorldPacket const* Write() override;

            uint32 Amount = 0;
        };

        class GarrisonStartMission final : public ClientPacket
        {
        public:
            GarrisonStartMission(WorldPacket&& packet) : ClientPacket(CMSG_GARRISON_START_MISSION, std::move(packet)) { }

            void Read() override;

            ObjectGuid NpcGUID;
            uint32 MissionRecID = 0;
            std::vector<uint64> FollowerDBIDs;
        };

        class GarrisonCompleteMission final : public ClientPacket
        {
        public:
            GarrisonCompleteMission(WorldPacket&& packet) : ClientPacket(CMSG_GARRISON_COMPLETE_MISSION, std::move(packet)) { }

            void Read() override;

            ObjectGuid NpcGUID;
            uint32 MissionRecID = 0;
        };

        class GarrisonMissionBonusRoll final : public ClientPacket
        {
        public:
            GarrisonMissionBonusRoll(WorldPacket&& packet) : ClientPacket(CMSG_GARRISON_MISSION_BONUS_ROLL, std::move(packet)) { }

            void Read() override;

            ObjectGuid NpcGUID;
            uint32 MissionRecID = 0;
        };

        class GarrisonAssignFollowerToBuilding final : public ClientPacket
        {
        public:
            GarrisonAssignFollowerToBuilding(WorldPacket&& packet) : ClientPacket(CMSG_GARRISON_ASSIGN_FOLLOWER_TO_BUILDING, std::move(packet)) { }

            void Read() override;

            ObjectGuid NpcGUID;
            int32 PlotInstanceID = 0;
            uint64 FollowerDBID = 0;
        };

        class GarrisonRemoveFollowerFromBuilding final : public ClientPacket
        {
        public:
            GarrisonRemoveFollowerFromBuilding(WorldPacket&& packet) : ClientPacket(CMSG_GARRISON_REMOVE_FOLLOWER_FROM_BUILDING, std::move(packet)) { }

            void Read() override;

            ObjectGuid NpcGUID;
            uint64 FollowerDBID = 0;
        };

        class GarrisonSetFollowerInactive final : public ClientPacket
        {
        public:
            GarrisonSetFollowerInactive(WorldPacket&& packet) : ClientPacket(CMSG_GARRISON_SET_FOLLOWER_INACTIVE, std::move(packet)) { }

            void Read() override;

            uint64 FollowerDBID = 0;
            bool Inactive = false;
        };

        class CreateShipment final : public ClientPacket
        {
        public:
            CreateShipment(WorldPacket&& packet) : ClientPacket(CMSG_CREATE_SHIPMENT, std::move(packet)) { }

            void Read() override;

            ObjectGuid NpcGUID;
            uint32 Count = 0;
        };

        class GarrisonRequestShipmentInfo final : public ClientPacket
        {
        public:
            GarrisonRequestShipmentInfo(WorldPacket&& packet) : ClientPacket(CMSG_GARRISON_REQUEST_SHIPMENT_INFO, std::move(packet)) { }

            void Read() override;

            ObjectGuid NpcGUID;
        };
    }
}

#endif // GarrisonPackets_h__
