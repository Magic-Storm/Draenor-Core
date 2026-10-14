///////////////////////////////////////////////////////////////////////////////
///
///  MILLENIUM-STUDIO
///  Copyright 2016 Millenium-studio SARL
///  All Rights Reserved.
///
////////////////////////////////////////////////////////////////////////////////

#include "hellfire_citadel.h"

DoorData const g_DoorData[] =
{
	{ eCitadelGameObjects::ReinforcedHellfireDoor, eCitadelDatas::DataMartak,     DoorType::DOOR_TYPE_PASSAGE, BoundaryType::BOUNDARY_NONE },
	{ eCitadelGameObjects::ReinforcedHellfireDoor, eCitadelDatas::DataReaver,     DoorType::DOOR_TYPE_PASSAGE, BoundaryType::BOUNDARY_NONE },
	{ eCitadelGameObjects::DarkPortalFireWall,     eCitadelDatas::DataArchimonde, DoorType::DOOR_TYPE_ROOM,    BoundaryType::BOUNDARY_NONE },
	{ 0,                                           0,                             DoorType::DOOR_TYPE_ROOM,    BoundaryType::BOUNDARY_NONE }
};

class instance_hellfire_citadel : public InstanceMapScript
{
public:
	instance_hellfire_citadel() : InstanceMapScript("instance_hellfire_citadel", 1448) { }

	struct instance_hellfire_citadelMapScript : public InstanceScript
	{
		instance_hellfire_citadelMapScript(Map* p_Map) : InstanceScript(p_Map)
		{
			m_Initialized = false;

			m_DungeonID = 0;

			m_MartakGuid = 0;
			m_ReinforcedHellfireDoor = 0;
			m_DarkPortalFireWall = 0;

			m_ReaverGuid = 0;
			m_KormrokGuid = 0;

			m_GurtoggGuid = 0;
			m_DiaGuid = 0;
			m_JubeithosGuid = 0;
			m_KilroggGuid = 0;
			m_AriokGuid = 0;
			m_GorefiendGuid = 0;

			m_IskarGuid = 0;
			m_SocretharGuid = 0;
			m_VelhariGuid = 0;

			m_ZakuunGuid = 0;
			m_XhulhoracGuid = 0;
			m_MannorothGuid = 0;

			m_AnetheronGuid = 0;
			m_KazrogalGuid = 0;
			m_AzgalorGuid = 0;
			m_ArchimondeGuid = 0;
		}

		bool m_Initialized;

		uint32 m_DungeonID;

		/// Hellbreach - Part 1
		uint64 m_MartakGuid;
		uint64 m_ReinforcedHellfireDoor;
		uint64 m_DarkPortalFireWall;
		uint64 m_ReaverGuid;
		uint64 m_KormrokGuid;

		/// Halls of Blood - Part 2
		uint64 m_GurtoggGuid;
		uint64 m_DiaGuid;
		uint64 m_JubeithosGuid;
		uint64 m_KilroggGuid;
		uint64 m_AriokGuid;
		uint64 m_GorefiendGuid;

		/// Bastion of Shadow - Part 3
		uint64 m_IskarGuid;
		uint64 m_SocretharGuid;
		uint64 m_VelhariGuid;

		/// Destructor's Rise - Part 4
		uint64 m_ZakuunGuid;
		uint64 m_XhulhoracGuid;
		uint64 m_MannorothGuid;

		/// The Black Gate - Part 5
		uint64 m_AnetheronGuid;
		uint64 m_KazrogalGuid;
		uint64 m_AzgalorGuid;
		uint64 m_ArchimondeGuid;

		void Initialize() override
		{
			SetBossNumber(eCitadelDatas::MaxBossData);

			LoadDoorData(g_DoorData);

			instance->SetObjectVisibility(500.0f);
		}

		void OnCreatureCreate(Creature* p_Creature) override
		{
			switch (p_Creature->GetEntry())
			{
			case eCitadelCreatures::BossMartak:
			{
				m_MartakGuid = p_Creature->GetGUID();
				break;
			}
			case eCitadelCreatures::BossReaver:
			{
				m_ReaverGuid = p_Creature->GetGUID();
				break;
			}
			case eCitadelCreatures::BossKormrok:
			{
				m_KormrokGuid = p_Creature->GetGUID();
				break;
			}
			case eCitadelCreatures::BossGurtogg:
			{
				m_GurtoggGuid = p_Creature->GetGUID();
				break;
			}
			case eCitadelCreatures::BossDia:
			{
				m_DiaGuid = p_Creature->GetGUID();
				break;
			}
			case eCitadelCreatures::BossJubeithos:
			{
				m_JubeithosGuid = p_Creature->GetGUID();
				break;
			}
			case eCitadelCreatures::BossKilrogg:
			{
				m_KilroggGuid = p_Creature->GetGUID();
				break;
			}
			case eCitadelCreatures::BossGorefiend:
			{
				m_GorefiendGuid = p_Creature->GetGUID();
				break;
			}
			case eCitadelCreatures::BossIskar:
			{
				m_IskarGuid = p_Creature->GetGUID();
				break;
			}
			case eCitadelCreatures::BossSocrethar:
			{
				m_SocretharGuid = p_Creature->GetGUID();
				break;
			}
			case eCitadelCreatures::BossVelhari:
			{
				m_VelhariGuid = p_Creature->GetGUID();
				break;
			}
			case eCitadelCreatures::BossZakuun:
			{
				m_ZakuunGuid = p_Creature->GetGUID();
				break;
			}
			case eCitadelCreatures::BossXhulhorac:
			{
				m_XhulhoracGuid = p_Creature->GetGUID();
				break;
			}
			case eCitadelCreatures::BossMannoroth:
			{
				m_MannorothGuid = p_Creature->GetGUID();
				break;
			}
			case eCitadelCreatures::Anetheron:
			{
				m_AnetheronGuid = p_Creature->GetGUID();
				break;
			}
			case eCitadelCreatures::Kazrogal:
			{
				m_KazrogalGuid = p_Creature->GetGUID();
				break;
			}
			case eCitadelCreatures::Azgalor:
			{
				m_AzgalorGuid = p_Creature->GetGUID();
				break;
			}
			case eCitadelCreatures::BossArchimonde:
			{
				m_ArchimondeGuid = p_Creature->GetGUID();
				break;
			}
			default:
				break;
			}
		}

		void OnGameObjectCreate(GameObject* p_GameObject) override
		{
			switch (p_GameObject->GetEntry())
			{
				case eCitadelGameObjects::ReinforcedHellfireDoor:
					m_ReinforcedHellfireDoor = p_GameObject->GetGUID();
					AddDoor(p_GameObject, true);
					break;
				case eCitadelGameObjects::DarkPortalFireWall:
					m_DarkPortalFireWall = p_GameObject->GetGUID();
					AddDoor(p_GameObject, true);
					break;
			default:
				break;
			}
		}

		void OnCreatureKilled(Creature* p_Creature, Player* p_Killer) override
		{
			/*switch (p_Creature->GetEntry())
			{
			case eIronMaidensCreatures::AquaticTechnician:
			case eIronMaidensCreatures::IronDockworker:
			case eIronMaidensCreatures::IronEarthbinder:
			case eIronMaidensCreatures::IronMauler:
			case eIronMaidensCreatures::IronCleaver:
			{
				if (Creature* l_Garan = instance->GetCreature(m_AdmiralGaranGuid))
				{
					if (l_Garan->IsAIEnabled)
						l_Garan->AI()->SetGUID(p_Creature->GetGUID());
				}

				break;
			}
			default:
				break;
			}*/
		}

		bool SetBossState(uint32 p_BossID, EncounterState p_State) override
		{
			uint32 l_OldState = GetBossState(p_BossID);

			if (!InstanceScript::SetBossState(p_BossID, p_State))
				return false;

			instance->SetObjectVisibility(500.0f);

			/// Don't handle the DONE state in case of loading
			if (p_State == EncounterState::DONE && l_OldState != EncounterState::IN_PROGRESS)
				return true;

			switch (p_BossID)
			{
			case eCitadelDatas::DataMartak:
			{
				switch (p_State)
				{
				case EncounterState::DONE:
				{
				}
				case EncounterState::NOT_STARTED:
				{
				}
				default:
					break;
				}

				break;
			}
			case eCitadelDatas::DataReaver:
			{
				switch (p_State)
				{
					case EncounterState::NOT_STARTED:
					{
						if (GameObject* l_Door = instance->GetGameObject(m_ReinforcedHellfireDoor))
							l_Door->SetGoState(GOState::GO_STATE_ACTIVE);
						break;
					}
					case EncounterState::IN_PROGRESS:
					{
						if (GameObject* l_Door = instance->GetGameObject(m_ReinforcedHellfireDoor))
							l_Door->SetGoState(GOState::GO_STATE_ACTIVE);
						break;
					}
					case EncounterState::DONE:
					{
						if (GameObject* l_Door = instance->GetGameObject(m_ReinforcedHellfireDoor))
							l_Door->SetGoState(GOState::GO_STATE_ACTIVE);
							break;
					}
				default:
					break;
				}
				break;
			}
			case eCitadelDatas::DataKormrok:
			{
				switch (p_State)
				{
				case EncounterState::DONE:
				{
					break;
				}
				case EncounterState::NOT_STARTED:
				{
					break;
				}
				default:
					break;
				}

				break;
			}
			case eCitadelDatas::DataCouncil:
			{
				switch (p_State)
				{
				case EncounterState::DONE:
				{
					break;
				}
				case EncounterState::NOT_STARTED:
				{
					break;
				}
				default:
					break;
				}

				break;
			}
			case eCitadelDatas::DataKilrogg:
			{
				switch (p_State)
				{
				case EncounterState::DONE:
				{
					break;
				}
				case EncounterState::NOT_STARTED:
				{
					break;
				}
				default:
					break;
				}

				break;
			}
			case eCitadelDatas::DataGorefiend:
			{
				switch (p_State)
				{
				case EncounterState::DONE:
				{

					break;
				}
				case EncounterState::NOT_STARTED:
				{
					break;
				}
				default:
					break;
				}

				break;
			}
			case eCitadelDatas::DataIskar:
			{
				switch (p_State)
				{
				case EncounterState::FAIL:
				case EncounterState::NOT_STARTED:
				{
					break;
				}
				case EncounterState::DONE:
				{
					break;
				}
				default:
					break;
				}

				break;
			}
			case eCitadelDatas::DataSocrethar:
			{
				switch (p_State)
				{
				case EncounterState::DONE:
				{
					break;
				}
				case EncounterState::FAIL:
				{
					break;
				}
				case EncounterState::IN_PROGRESS:
				{
					break;
				}
				default:
					break;
				}

				break;
			}
			case eCitadelDatas::DataVelhari:
			{
				switch (p_State)
				{
				case EncounterState::DONE:
				{
					/*AddTimedDelayedOperation(10 * TimeConstants::IN_MILLISECONDS, [this]() -> void
					{
						if (Creature* l_Khadgar = instance->GetCreature(m_CosmeticKhadgar))
						{
							if (l_Khadgar->IsAIEnabled)
								l_Khadgar->AI()->Talk(12, 0, TextRange::TEXT_RANGE_MAP);   ///< Khadgar one seal
						}
					});*/

					/// No break needed here
				}
				case EncounterState::FAIL:
				case EncounterState::NOT_STARTED:
				{
					break;
				}
				default:
					break;
				}

				break;
			}
			case eCitadelDatas::DataZakuun:
			{
				switch (p_State)
				{
				case EncounterState::DONE:
				{
					break;
				}
				default:
				{
					break;
				}
				}

				break;
			}
			case eCitadelDatas::DataXhulhorac:
			{
				switch (p_State)
				{
				case EncounterState::DONE:
				{
					break;
				}
				default:
				{
					break;
				}
				}

				break;
			}
			case eCitadelDatas::DataMannoroth:
			{
				switch (p_State)
				{
				case EncounterState::DONE:
				{
					/// Open the path toward the Black Gate after Mannoroth
					if (GameObject* l_Firewall = instance->GetGameObject(m_DarkPortalFireWall))
						l_Firewall->SetGoState(GOState::GO_STATE_ACTIVE);
					break;
				}
				default:
					break;
				}
				break;
			}
			case eCitadelDatas::DataArchimonde:
			{
				switch (p_State)
				{
				case EncounterState::DONE:
				{
					if (GameObject* l_Firewall = instance->GetGameObject(m_DarkPortalFireWall))
						l_Firewall->SetGoState(GOState::GO_STATE_ACTIVE);
					break;
				}
				case EncounterState::IN_PROGRESS:
				{
					if (GameObject* l_Firewall = instance->GetGameObject(m_DarkPortalFireWall))
						l_Firewall->SetGoState(GOState::GO_STATE_READY);
					break;
				}
				default:
					break;
				}
				break;
			}
			default:
				break;
			}

			return true;
		}

		uint64 GetData64(uint32 p_Type) override
		{
			switch (p_Type)
			{
			case eCitadelCreatures::BossMartak:
				return m_MartakGuid;
			case eCitadelGameObjects::ReinforcedHellfireDoor:
				return m_ReinforcedHellfireDoor;
			case eCitadelGameObjects::DarkPortalFireWall:
				return m_DarkPortalFireWall;
			case eCitadelCreatures::BossReaver:
				return m_ReaverGuid;
			case eCitadelCreatures::BossKormrok:
				return m_KormrokGuid;
			case eCitadelCreatures::BossGurtogg:
				return m_GurtoggGuid;
			case eCitadelCreatures::BossDia:
				return m_DiaGuid;
			case eCitadelCreatures::BossJubeithos:
				return m_JubeithosGuid;
			case eCitadelCreatures::BossKilrogg:
				return m_KilroggGuid;
			case eCitadelCreatures::BossGorefiend:
				return m_GorefiendGuid;
			case eCitadelCreatures::BossIskar:
				return m_IskarGuid;
			case eCitadelCreatures::BossSocrethar:
				return m_SocretharGuid;
			case eCitadelCreatures::BossVelhari:
				return m_VelhariGuid;
			case eCitadelCreatures::BossZakuun:
				return m_ZakuunGuid;
			case eCitadelCreatures::BossXhulhorac:
				return m_XhulhoracGuid;
			case eCitadelCreatures::BossMannoroth:
				return m_MannorothGuid;
			case eCitadelCreatures::BossArchimonde:
				return m_ArchimondeGuid;

			default:
				break;
			}

			return 0;
		}

		void OnUnitDeath(Unit* p_Unit) override
		{
			Creature* l_Creature = p_Unit->ToCreature();
			if (!l_Creature)
				return;

			switch (l_Creature->GetEntry())
			{
			case eCitadelCreatures::BossArchimonde:
			{
				if (l_Creature->GetMap()->IsMythic())
				{
					DoCompleteAchievement(eCitadelAchievements::AchievementArchimondeMythic);
					DoCompleteAchievement(eCitadelAchievements::AchievementArchimondeEdge);
				}
				else if (l_Creature->GetMap()->IsHeroic())
					DoCompleteAchievement(eCitadelAchievements::AchievementArchimondeCurve);
				else
					DoCompleteAchievement(eCitadelAchievements::AchievementArchimondeNormal);
				break;
			}
			default:
				break;
			}
		}

		bool CheckRequiredBosses(uint32 p_BossID, Player const* p_Player = nullptr) const override
		{
			if (!InstanceScript::CheckRequiredBosses(p_BossID, p_Player))
				return false;

			switch (p_BossID)
			{
			case eCitadelDatas::DataArchimonde:
			{
				/// Mannoroth must be defeated before Archimonde
				if (GetBossState(eCitadelDatas::DataMannoroth) != EncounterState::DONE)
					return false;
				return true;
			}
			default:
				break;
			}

			return true;
		}

		void OnPlayerEnter(Player* p_Player) override
		{
			InstanceScript::OnPlayerEnter(p_Player);

			/// Disable non available bosses for LFR
			if (!m_Initialized)
			{
				m_Initialized = true;

				m_DungeonID = p_Player->GetGroup() ? sLFGMgr->GetDungeon(p_Player->GetGroup()->GetGUID()) : 0;

				if (!instance->IsLFR())
					m_DungeonID = 0;

				switch (m_DungeonID)
				{
				case eCitadelDungeons::Hellbreach:
				{
					uint32 l_DisabledMask = 0;
					l_DisabledMask |= (1 << eCitadelDatas::DataCouncil);
					l_DisabledMask |= (1 << eCitadelDatas::DataKilrogg);
					l_DisabledMask |= (1 << eCitadelDatas::DataGorefiend);
					l_DisabledMask |= (1 << eCitadelDatas::DataIskar);
					l_DisabledMask |= (1 << eCitadelDatas::DataSocrethar);
					l_DisabledMask |= (1 << eCitadelDatas::DataVelhari);
					l_DisabledMask |= (1 << eCitadelDatas::DataZakuun);
					l_DisabledMask |= (1 << eCitadelDatas::DataXhulhorac);
					l_DisabledMask |= (1 << eCitadelDatas::DataMannoroth);
					l_DisabledMask |= (1 << eCitadelDatas::DataArchimonde);
					SetDisabledBosses(l_DisabledMask);
					break;
				}
				case eCitadelDungeons::HallsofBlood:
				{
					uint32 l_DisabledMask = 0;
					l_DisabledMask |= (1 << eCitadelDatas::DataMartak);
					l_DisabledMask |= (1 << eCitadelDatas::DataReaver);
					l_DisabledMask |= (1 << eCitadelDatas::DataKormrok);
					l_DisabledMask |= (1 << eCitadelDatas::DataIskar);
					l_DisabledMask |= (1 << eCitadelDatas::DataSocrethar);
					l_DisabledMask |= (1 << eCitadelDatas::DataVelhari);
					l_DisabledMask |= (1 << eCitadelDatas::DataZakuun);
					l_DisabledMask |= (1 << eCitadelDatas::DataXhulhorac);
					l_DisabledMask |= (1 << eCitadelDatas::DataMannoroth);
					l_DisabledMask |= (1 << eCitadelDatas::DataArchimonde);
					SetDisabledBosses(l_DisabledMask);
					break;
				}
				case eCitadelDungeons::BastionofShadows:
				{
					uint32 l_DisabledMask = 0;
					l_DisabledMask |= (1 << eCitadelDatas::DataMartak);
					l_DisabledMask |= (1 << eCitadelDatas::DataReaver);
					l_DisabledMask |= (1 << eCitadelDatas::DataKormrok);
					l_DisabledMask |= (1 << eCitadelDatas::DataCouncil);
					l_DisabledMask |= (1 << eCitadelDatas::DataKilrogg);
					l_DisabledMask |= (1 << eCitadelDatas::DataGorefiend);
					l_DisabledMask |= (1 << eCitadelDatas::DataZakuun);
					l_DisabledMask |= (1 << eCitadelDatas::DataXhulhorac);
					l_DisabledMask |= (1 << eCitadelDatas::DataMannoroth);
					l_DisabledMask |= (1 << eCitadelDatas::DataArchimonde);
					SetDisabledBosses(l_DisabledMask);
					break;
				}
				case eCitadelDungeons::DestructorRise:
				{
					uint32 l_DisabledMask = 0;
					l_DisabledMask |= (1 << eCitadelDatas::DataMartak);
					l_DisabledMask |= (1 << eCitadelDatas::DataReaver);
					l_DisabledMask |= (1 << eCitadelDatas::DataKormrok);
					l_DisabledMask |= (1 << eCitadelDatas::DataCouncil);
					l_DisabledMask |= (1 << eCitadelDatas::DataKilrogg);
					l_DisabledMask |= (1 << eCitadelDatas::DataGorefiend);
					l_DisabledMask |= (1 << eCitadelDatas::DataIskar);
					l_DisabledMask |= (1 << eCitadelDatas::DataSocrethar);
					l_DisabledMask |= (1 << eCitadelDatas::DataVelhari);
					l_DisabledMask |= (1 << eCitadelDatas::DataArchimonde);
					SetDisabledBosses(l_DisabledMask);
					break;
				}
				case eCitadelDungeons::BlackGate:
				{
					uint32 l_DisabledMask = 0;
					l_DisabledMask |= (1 << eCitadelDatas::DataMartak);
					l_DisabledMask |= (1 << eCitadelDatas::DataReaver);
					l_DisabledMask |= (1 << eCitadelDatas::DataKormrok);
					l_DisabledMask |= (1 << eCitadelDatas::DataCouncil);
					l_DisabledMask |= (1 << eCitadelDatas::DataKilrogg);
					l_DisabledMask |= (1 << eCitadelDatas::DataGorefiend);
					l_DisabledMask |= (1 << eCitadelDatas::DataIskar);
					l_DisabledMask |= (1 << eCitadelDatas::DataSocrethar);
					l_DisabledMask |= (1 << eCitadelDatas::DataVelhari);
					l_DisabledMask |= (1 << eCitadelDatas::DataZakuun);
					l_DisabledMask |= (1 << eCitadelDatas::DataXhulhorac);
					l_DisabledMask |= (1 << eCitadelDatas::DataMannoroth);
					SetDisabledBosses(l_DisabledMask);
					break;
				}
				default:
					break;
				}
			}
		}

		void OnPlayerExit(Player* p_Player) override
		{
			InstanceScript::OnPlayerExit(p_Player);
		}

		void SendUpdateWorldState(uint32 p_Field, uint32 p_Value)
		{
			Map::PlayerList const& l_Players = instance->GetPlayers();
			for (Map::PlayerList::const_iterator l_Iter = l_Players.begin(); l_Iter != l_Players.end(); ++l_Iter)
			{
				if (Player* l_Player = l_Iter->getSource())
					l_Player->SendUpdateWorldState(p_Field, p_Value);
			}
		}

		void Update(uint32 p_Diff) override
		{
			UpdateOperations(p_Diff);
			UpdateCombatResurrection(p_Diff);
		}
	};

	InstanceScript* GetInstanceScript(InstanceMap* p_Map) const override
	{
		return new instance_hellfire_citadelMapScript(p_Map);
	}
};

#ifndef __clang_analyzer__
void AddSC_instance_hellfire_citadel()
{
	new instance_hellfire_citadel();
}
#endif