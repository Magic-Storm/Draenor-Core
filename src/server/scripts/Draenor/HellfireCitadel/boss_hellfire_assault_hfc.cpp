///////////////////////////////////////////////////////////////////////////////
///
///  MILLENIUM-STUDIO
///  Copyright 2016 Millenium-studio SARL
///  All Rights Reserved.
///
///////////////////////////////////////////////////////////////////////////////

#include "boss_hellfire_assault_hfc.h"
#include "hellfire_citadel.h"

//////////////////////////////////////////////////////////////////////////////
///  Siegemaster Mar'tak — 95068
///
///  Primary fight objective: players must deliver g_MunitionsToBreachDoor
///  Felfire Munitions to the Alliance Hellfire Cannons to breach the citadel
///  door.  Mar'tak must also be kept alive — if both cannons are destroyed
///  he enrages.  Mar'tak still has tank-swap and raid-damage abilities that
///  must be managed while the munitions mechanic is resolved.
//////////////////////////////////////////////////////////////////////////////
class boss_martak_hfc : public CreatureScript
{
public:
	boss_martak_hfc() : CreatureScript("boss_martak_hfc") {}

	enum eTalks
	{
		TalkAggro  = 0,
		TalkWave   = 1,
		TalkSlay   = 2,
		TalkWipe   = 3,
		TalkDeath  = 4,
	};

	struct boss_martak_hfcAI : public BossAI
	{
		boss_martak_hfcAI(Creature* p_Creature) : BossAI(p_Creature, eCitadelDatas::DataMartak)
		{
			m_Instance           = p_Creature->GetInstanceScript();
			m_MunitionsDelivered = 0;
			m_CannonsAlive       = 2;
			m_WaveCount          = 0;
		}

		InstanceScript* m_Instance;
		EventMap        m_Events;
		uint8           m_MunitionsDelivered;
		uint8           m_CannonsAlive;
		uint8           m_WaveCount;

		/// ----- Reset / State -----------------------------------------------

		void Reset() override
		{
			m_Events.Reset();
			ClearDelayedOperations();
			summons.DespawnAll();
			me->SetReactState(ReactStates::REACT_AGGRESSIVE);

			m_MunitionsDelivered = 0;
			m_CannonsAlive       = 2;
			m_WaveCount          = 0;

			_Reset();

			if (m_Instance != nullptr)
				m_Instance->SendEncounterUnit(EncounterFrameType::ENCOUNTER_FRAME_DISENGAGE, me);
		}

		void JustReachedHome() override
		{
			Reset();
			Talk(eTalks::TalkWipe);

			if (m_Instance != nullptr)
			{
				m_Instance->SendEncounterUnit(EncounterFrameType::ENCOUNTER_FRAME_DISENGAGE, me);
				m_Instance->SetBossState(eCitadelDatas::DataMartak, EncounterState::FAIL);
			}
		}

		void EnterCombat(Unit* /*p_Attacker*/) override
		{
			_EnterCombat();
			Talk(eTalks::TalkAggro);

			m_Events.ScheduleEvent(eHellfireAssaultEvents::EventHowlingAxe,  eHellfireAssaultTimers::TimerHowlingAxe);
			m_Events.ScheduleEvent(eHellfireAssaultEvents::EventShockwave,   eHellfireAssaultTimers::TimerShockwave);
			m_Events.ScheduleEvent(eHellfireAssaultEvents::EventTraitorsVow, eHellfireAssaultTimers::TimerTraitorsVow);
			m_Events.ScheduleEvent(eHellfireAssaultEvents::EventSpawnWave,   eHellfireAssaultTimers::TimerFirstWave);

			if (m_Instance != nullptr)
			{
				m_Instance->SendEncounterUnit(EncounterFrameType::ENCOUNTER_FRAME_ENGAGE, me, 1);
				m_Instance->SetBossState(eCitadelDatas::DataMartak, EncounterState::IN_PROGRESS);
			}
		}

		void KilledUnit(Unit* p_Who) override
		{
			if (!p_Who->IsPlayer())
				return;
			Talk(eTalks::TalkSlay);
		}

		void JustDied(Unit* /*p_Killer*/) override
		{
			_JustDied();
			Talk(eTalks::TalkDeath);
			summons.DespawnAll();

			if (m_Instance != nullptr)
			{
				m_Instance->SendEncounterUnit(EncounterFrameType::ENCOUNTER_FRAME_DISENGAGE, me);
				m_Instance->SetBossState(eCitadelDatas::DataMartak, EncounterState::DONE);

				if (sObjectMgr->IsDisabledEncounter(m_Instance->GetEncounterIDForBoss(me), GetDifficulty()))
					me->SetLootRecipient(nullptr);
				else
					CastSpellToPlayers(me->GetMap(), me, eHellfireAssaultSpells::SpellHellfireAssaultLoot, true);
			}
		}

		/// ----- Cross-NPC actions -------------------------------------------

		void DoAction(int32 const p_Action) override
		{
			switch (p_Action)
			{
				/// Called by Hellfire Cannon when a player delivers munitions
				case eHellfireAssaultActions::ActionMunitionDelivered:
				{
					++m_MunitionsDelivered;
					if (m_MunitionsDelivered >= g_MunitionsToBreachDoor)
						BreachwallEncounter();
					break;
				}
				/// Called by Hellfire Cannon when it is destroyed (wipe condition)
				case eHellfireAssaultActions::ActionCannonDestroyed:
				{
					if (m_CannonsAlive > 0)
						--m_CannonsAlive;

					if (m_CannonsAlive == 0)
					{
						/// Both cannons lost — Mar'tak enrages (raid wipe)
						CastSpellToPlayers(me->GetMap(), me, eCitadelSpells::Berserker, true);
						me->CastSpell(me, eCitadelSpells::Berserker, true);
					}
					break;
				}
				default:
					break;
			}
		}

		/// ----- Internal helpers --------------------------------------------

		/// Door is breached: kill Mar'tak to trigger JustDied → loot + DONE
		void BreachwallEncounter()
		{
			summons.DespawnAll();
			me->Kill(me, false);
		}

		void SpawnWave()
		{
			++m_WaveCount;
			Talk(eTalks::TalkWave);

			/// Spread grunts evenly around Mar'tak at 18 yd range
			for (uint8 i = 0; i < g_WaveGruntCount; ++i)
			{
				float l_Angle = float(i) * (2.0f * float(M_PI) / float(g_WaveGruntCount));
				float l_X     = me->GetPositionX() + 18.0f * std::cos(l_Angle);
				float l_Y     = me->GetPositionY() + 18.0f * std::sin(l_Angle);

				if (Creature* l_Grunt = me->SummonCreature(
					eHellfireAssaultCreatures::NpcHellfireGrunt,
					l_X, l_Y, me->GetPositionZ(), 0.0f,
					TEMPSUMMON_CORPSE_TIMED_DESPAWN, 10 * TimeConstants::IN_MILLISECONDS))
				{
					l_Grunt->SetInCombatWithZone();
				}
			}

			/// Every second wave also spawns a Felfire siege vehicle that drops munitions
			if ((m_WaveCount % 2) == 0)
			{
				float l_Ex = me->GetPositionX() + frand(-22.0f, 22.0f);
				float l_Ey = me->GetPositionY() + frand(-22.0f, 22.0f);

				if (Creature* l_Vehicle = me->SummonCreature(
					eHellfireAssaultCreatures::NpcFelfireSiegeVehicle,
					l_Ex, l_Ey, me->GetPositionZ(), 0.0f,
					TEMPSUMMON_CORPSE_TIMED_DESPAWN, 10 * TimeConstants::IN_MILLISECONDS))
				{
					l_Vehicle->SetInCombatWithZone();
				}
			}
		}

		/// ----- AI Update ---------------------------------------------------

		void UpdateAI(uint32 const p_Diff) override
		{
			UpdateOperations(p_Diff);
			m_Events.Update(p_Diff);

			if (!UpdateVictim())
				return;

			if (me->HasUnitState(UnitState::UNIT_STATE_CASTING))
				return;

			switch (m_Events.ExecuteEvent())
			{
				case eHellfireAssaultEvents::EventHowlingAxe:
				{
					if (Unit* l_Target = SelectTarget(SELECT_TARGET_TOPAGGRO, 0, 0.0f, true))
						me->CastSpell(l_Target, eHellfireAssaultSpells::SpellHowlingAxe, false);
					m_Events.ScheduleEvent(eHellfireAssaultEvents::EventHowlingAxe,
						eHellfireAssaultTimers::TimerHowlingAxeCD);
					break;
				}
				case eHellfireAssaultEvents::EventShockwave:
				{
					me->CastSpell(me, eHellfireAssaultSpells::SpellShockwave, false);
					m_Events.ScheduleEvent(eHellfireAssaultEvents::EventShockwave,
						eHellfireAssaultTimers::TimerShockwaveCD);
					break;
				}
				case eHellfireAssaultEvents::EventTraitorsVow:
				{
					if (Unit* l_Target = SelectTarget(SELECT_TARGET_RANDOM, 0, 40.0f, true))
						me->CastSpell(l_Target, eHellfireAssaultSpells::SpellTraitorsVow, false);
					m_Events.ScheduleEvent(eHellfireAssaultEvents::EventTraitorsVow,
						eHellfireAssaultTimers::TimerTraitorsVowCD);
					break;
				}
				case eHellfireAssaultEvents::EventSpawnWave:
				{
					SpawnWave();
					m_Events.ScheduleEvent(eHellfireAssaultEvents::EventSpawnWave,
						eHellfireAssaultTimers::TimerWaveCD);
					break;
				}
				default:
					break;
			}

			DoMeleeAttackIfReady();
		}
	};

	CreatureAI* GetAI(Creature* p_Creature) const override
	{
		return new boss_martak_hfcAI(p_Creature);
	}
};

//////////////////////////////////////////////////////////////////////////////
///  Hellfire Cannon — 90018
///
///  Players deliver Felfire Munitions via gossip interaction.
///  After g_MunitionsToBreachDoor deliveries the citadel door is breached.
///  If this cannon is killed it notifies Mar'tak; two dead cannons = enrage.
//////////////////////////////////////////////////////////////////////////////
class npc_hellfire_cannon_hfa : public CreatureScript
{
public:
	npc_hellfire_cannon_hfa() : CreatureScript("npc_hellfire_cannon_hfa") {}

	bool OnGossipHello(Player* p_Player, Creature* p_Creature) override
	{
		if (p_Player->HasAura(eHellfireAssaultSpells::SpellFelfireMunitions))
		{
			p_Player->RemoveAura(eHellfireAssaultSpells::SpellFelfireMunitions);

			/// Notify Mar'tak that munitions were delivered
			std::list<Creature*> l_BossCreatures;
			GetCreatureListWithEntryInGrid(l_BossCreatures, p_Creature,
				eCitadelCreatures::BossMartak, 600.0f);

			for (Creature* l_Boss : l_BossCreatures)
			{
				if (l_Boss->IsAIEnabled)
					l_Boss->AI()->DoAction(eHellfireAssaultActions::ActionMunitionDelivered);
			}
		}

		p_Player->PlayerTalkClass->SendCloseGossip();
		return true;
	}

	struct npc_hellfire_cannon_hfaAI : public ScriptedAI
	{
		npc_hellfire_cannon_hfaAI(Creature* p_Creature) : ScriptedAI(p_Creature) {}

		void JustDied(Unit* /*p_Killer*/) override
		{
			/// Notify Mar'tak that this cannon was destroyed
			std::list<Creature*> l_BossCreatures;
			GetCreatureListWithEntryInGrid(l_BossCreatures, me,
				eCitadelCreatures::BossMartak, 600.0f);

			for (Creature* l_Boss : l_BossCreatures)
			{
				if (l_Boss->IsAIEnabled)
					l_Boss->AI()->DoAction(eHellfireAssaultActions::ActionCannonDestroyed);
			}
		}

		void UpdateAI(uint32 /*p_Diff*/) override {}
	};

	CreatureAI* GetAI(Creature* p_Creature) const override
	{
		return new npc_hellfire_cannon_hfaAI(p_Creature);
	}
};

//////////////////////////////////////////////////////////////////////////////
///  Felfire Demolisher — 91103  (ScriptName: npc_hellfire_engineer_hfa)
///
///  Siege vehicle wave add that drops Felfire Munitions when killed.
//////////////////////////////////////////////////////////////////////////////
class npc_hellfire_engineer_hfa : public CreatureScript
{
public:
	npc_hellfire_engineer_hfa() : CreatureScript("npc_hellfire_engineer_hfa") {}

	struct npc_hellfire_engineer_hfaAI : public ScriptedAI
	{
		npc_hellfire_engineer_hfaAI(Creature* p_Creature) : ScriptedAI(p_Creature) {}

		void Reset() override
		{
			me->SetReactState(ReactStates::REACT_AGGRESSIVE);
		}

		void JustDied(Unit* /*p_Killer*/) override
		{
			/// Spawn collectible Felfire Munitions at the vehicle's death location
			me->SummonCreature(
				eHellfireAssaultCreatures::NpcFelfireMunitionsCrate,
				me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(),
				me->GetOrientation(),
				TEMPSUMMON_TIMED_DESPAWN, 120 * TimeConstants::IN_MILLISECONDS);
		}

		void UpdateAI(uint32 /*p_Diff*/) override
		{
			if (!UpdateVictim())
				return;
			DoMeleeAttackIfReady();
		}
	};

	CreatureAI* GetAI(Creature* p_Creature) const override
	{
		return new npc_hellfire_engineer_hfaAI(p_Creature);
	}
};

//////////////////////////////////////////////////////////////////////////////
///  Felfire Munitions — 90610
///
///  Interactable creature spawned when a siege vehicle dies.
///  Players click it (gossip) to receive the Felfire Munitions carrier aura,
///  then walk to a Hellfire Cannon and interact there to deliver.
//////////////////////////////////////////////////////////////////////////////
class npc_felfire_munitions_crate_hfa : public CreatureScript
{
public:
	npc_felfire_munitions_crate_hfa() : CreatureScript("npc_felfire_munitions_crate_hfa") {}

	bool OnGossipHello(Player* p_Player, Creature* p_Creature) override
	{
		/// Only allow pickup if the player is not already carrying munitions
		if (!p_Player->HasAura(eHellfireAssaultSpells::SpellFelfireMunitions))
		{
			p_Creature->CastSpell(p_Player, eHellfireAssaultSpells::SpellFelfireMunitions, true);
			p_Creature->DespawnOrUnsummon(500);
		}

		p_Player->PlayerTalkClass->SendCloseGossip();
		return true;
	}

	struct npc_felfire_munitions_crate_hfaAI : public ScriptedAI
	{
		npc_felfire_munitions_crate_hfaAI(Creature* p_Creature) : ScriptedAI(p_Creature) {}

		void Reset() override
		{
			me->SetReactState(ReactStates::REACT_PASSIVE);
			me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
		}

		void UpdateAI(uint32 /*p_Diff*/) override {}
	};

	CreatureAI* GetAI(Creature* p_Creature) const override
	{
		return new npc_felfire_munitions_crate_hfaAI(p_Creature);
	}
};

//////////////////////////////////////////////////////////////////////////////
///  Hellfire Grunt — 93858
///
///  Basic melee add summoned during Mar'tak's assault waves.
//////////////////////////////////////////////////////////////////////////////
class npc_hellfire_grunt_hfa : public CreatureScript
{
public:
	npc_hellfire_grunt_hfa() : CreatureScript("npc_hellfire_grunt_hfa") {}

	struct npc_hellfire_grunt_hfaAI : public ScriptedAI
	{
		npc_hellfire_grunt_hfaAI(Creature* p_Creature) : ScriptedAI(p_Creature) {}

		void Reset() override
		{
			me->SetReactState(ReactStates::REACT_AGGRESSIVE);
		}

		void UpdateAI(uint32 /*p_Diff*/) override
		{
			if (!UpdateVictim())
				return;
			DoMeleeAttackIfReady();
		}
	};

	CreatureAI* GetAI(Creature* p_Creature) const override
	{
		return new npc_hellfire_grunt_hfaAI(p_Creature);
	}
};

#ifndef __clang_analyzer__
void AddSC_boss_hellfire_assault_hfc()
{
	/// Boss
	new boss_martak_hfc();

	/// Creatures
	new npc_hellfire_cannon_hfa();
	new npc_hellfire_engineer_hfa();
	new npc_felfire_munitions_crate_hfa();
	new npc_hellfire_grunt_hfa();
}
#endif
