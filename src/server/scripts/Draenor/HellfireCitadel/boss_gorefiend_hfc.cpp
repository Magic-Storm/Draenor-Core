///////////////////////////////////////////////////////////////////////////////
///
///  MILLENIUM-STUDIO
///  Copyright 2016 Millenium-studio SARL
///  All Rights Reserved.
///
///////////////////////////////////////////////////////////////////////////////

#include "boss_gorefiend_hfc.h"
#include "hellfire_citadel.h"

/// ---------------------------------------------------------------------------
/// Gorefiend - 91809
/// ---------------------------------------------------------------------------
class boss_gorefiend_hfc : public CreatureScript
{
public:
	boss_gorefiend_hfc() : CreatureScript("boss_gorefiend_hfc") { }

	struct boss_gorefiend_hfcAI : public BossAI
	{
		boss_gorefiend_hfcAI(Creature* p_Creature)
			: BossAI(p_Creature, eCitadelDatas::DataGorefiend)
		{
			m_Instance			= p_Creature->GetInstanceScript();
			m_ShadowRealmCount	= 0;
		}

		InstanceScript*	m_Instance;
		uint8			m_ShadowRealmCount;		///< How many souls currently trapped in the realm

		void Reset() override
		{
			_Reset();

			m_ShadowRealmCount = 0;
			me->SetReactState(REACT_AGGRESSIVE);
			me->RemoveAura(eGorefiendSpells::SpellConsumedSouls);
		}

		void EnterCombat(Unit* p_Who) override
		{
			_EnterCombat();

			events.ScheduleEvent(eGorefiendEvents::EventFeastOfSouls,  eGorefiendTimers::TimerFeastOfSouls);
			events.ScheduleEvent(eGorefiendEvents::EventSharedFate,    eGorefiendTimers::TimerSharedFate);
			events.ScheduleEvent(eGorefiendEvents::EventTouchOfDoom,   eGorefiendTimers::TimerTouchOfDoom);
			events.ScheduleEvent(eGorefiendEvents::EventShadowOfDeath, eGorefiendTimers::TimerShadowOfDeath);
			events.ScheduleEvent(eGorefiendEvents::EventHungerForLife, eGorefiendTimers::TimerHungerForLife);
		}

		void JustDied(Unit* /*p_Killer*/) override
		{
			_JustDied();

			/// Rescue any souls still trapped in the realm
			ReleaseShadowRealm();
		}

		void JustSummoned(Creature* p_Summon) override
		{
			summons.Summon(p_Summon);

			switch (p_Summon->GetEntry())
			{
				case eGorefiendCreatures::NpcTorturedEssence:
				case eGorefiendCreatures::NpcGoreboundSpirit:
					p_Summon->SetInCombatWithZone();
					break;
				default:
					break;
			}
		}

		void DoAction(int32 p_Action) override
		{
			switch (p_Action)
			{
				case eGorefiendActions::ActionSoulEscaped:
				{
					if (m_ShadowRealmCount > 0)
						--m_ShadowRealmCount;
					break;
				}
				default:
					break;
			}
		}

		/// Despawn all realm adds and release trapped players.
		void ReleaseShadowRealm()
		{
			DespawnCreaturesInArea(eGorefiendCreatures::NpcTorturedEssence, me);
			DespawnCreaturesInArea(eGorefiendCreatures::NpcGoreboundSpirit, me);

			/// Remove Shadow of Death aura from any player still marked
			if (Map* l_Map = me->GetMap())
			{
				Map::PlayerList const& l_Players = l_Map->GetPlayers();
				for (Map::PlayerList::const_iterator l_Iter = l_Players.begin(); l_Iter != l_Players.end(); ++l_Iter)
				{
					if (Player* l_Player = l_Iter->getSource())
					{
						l_Player->RemoveAura(eGorefiendSpells::SpellShadowOfDeath);
						l_Player->RemoveAura(eGorefiendSpells::SpellSharedFate);
						l_Player->RemoveAura(eGorefiendSpells::SpellSharedFateVisual);
					}
				}
			}

			m_ShadowRealmCount = 0;
		}

		/// Send a target to Gorefiend's shadow realm and spawn adds around them.
		void CastShadowOfDeath(Unit* p_Target)
		{
			if (!p_Target)
				return;

			/// Apply the realm aura to the target
			me->CastSpell(p_Target, eGorefiendSpells::SpellShadowOfDeath, true);
			++m_ShadowRealmCount;

			/// Gain a soul empowerment stack
			me->CastSpell(me, eGorefiendSpells::SpellConsumedSouls, true);

			/// Summon a Gorebound Spirit and a Tortured Essence near the target to threaten them
			float l_X = p_Target->GetPositionX();
			float l_Y = p_Target->GetPositionY();
			float l_Z = p_Target->GetPositionZ();

			if (Creature* l_Spirit = me->SummonCreature(eGorefiendCreatures::NpcGoreboundSpirit,
				l_X + frand(-6.0f, 6.0f), l_Y + frand(-6.0f, 6.0f), l_Z,
				0.0f, TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 45000))
			{
				l_Spirit->AI()->AttackStart(p_Target);
			}

			if (Creature* l_Essence = me->SummonCreature(eGorefiendCreatures::NpcTorturedEssence,
				l_X + frand(-6.0f, 6.0f), l_Y + frand(-6.0f, 6.0f), l_Z,
				0.0f, TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 45000))
			{
				l_Essence->AI()->AttackStart(p_Target);
			}
		}

		void UpdateAI(uint32 p_Diff) override
		{
			if (!UpdateVictim())
				return;

			events.Update(p_Diff);

			if (me->HasUnitState(UNIT_STATE_CASTING))
				return;

			while (uint32 l_EventId = events.ExecuteEvent())
			{
				switch (l_EventId)
				{
					/// Feast of Souls - drain the life of all players in front of Gorefiend
					case eGorefiendEvents::EventFeastOfSouls:
					{
						me->CastSpell(me, eGorefiendSpells::SpellFeastOfSouls, false);
						events.ScheduleEvent(eGorefiendEvents::EventFeastOfSouls, eGorefiendTimers::TimerFeastOfSouls);
						break;
					}

					/// Shared Fate - link two random players so that separation damages both
					case eGorefiendEvents::EventSharedFate:
					{
						/// Select two distinct random non-tank players if possible
						std::list<Unit*> l_Targets;

						if (Unit* l_A = SelectTarget(SELECT_TARGET_RANDOM, 1, 60.0f, true))
						{
							me->CastSpell(l_A, eGorefiendSpells::SpellSharedFate, true);

							if (Unit* l_B = SelectTarget(SELECT_TARGET_RANDOM, 1, 60.0f, true))
							{
								if (l_B != l_A)
									me->CastSpell(l_B, eGorefiendSpells::SpellSharedFate, true);
							}
						}

						events.ScheduleEvent(eGorefiendEvents::EventSharedFate, eGorefiendTimers::TimerSharedFate);
						break;
					}

					/// Touch of Doom - heavy DoT on current tank
					case eGorefiendEvents::EventTouchOfDoom:
					{
						if (Unit* l_Tank = me->GetVictim())
							me->CastSpell(l_Tank, eGorefiendSpells::SpellTouchOfDoom, false);
						events.ScheduleEvent(eGorefiendEvents::EventTouchOfDoom, eGorefiendTimers::TimerTouchOfDoom);
						break;
					}

					/// Shadow of Death - banish a random non-tank player into the realm
					case eGorefiendEvents::EventShadowOfDeath:
					{
						if (Unit* l_Target = SelectTarget(SELECT_TARGET_RANDOM, 1, 60.0f, true))
							CastShadowOfDeath(l_Target);
						events.ScheduleEvent(eGorefiendEvents::EventShadowOfDeath, eGorefiendTimers::TimerShadowOfDeath);
						break;
					}

					/// Hunger for Life - Gorefiend heals for each soul still trapped in the realm
					case eGorefiendEvents::EventHungerForLife:
					{
						if (m_ShadowRealmCount > 0)
							me->CastSpell(me, eGorefiendSpells::SpellHungerForLife, true);
						events.ScheduleEvent(eGorefiendEvents::EventHungerForLife, eGorefiendTimers::TimerHungerForLife);
						break;
					}

					default:
						break;
				}
			}

			DoMeleeAttackIfReady();
		}
	};

	CreatureAI* GetAI(Creature* p_Creature) const override
	{
		return new boss_gorefiend_hfcAI(p_Creature);
	}
};

/// ---------------------------------------------------------------------------
/// Gorebound Spirit - 91812
/// Melee add that spawns inside the shadow realm and attacks the trapped player.
/// ---------------------------------------------------------------------------
class npc_gorebound_spirit : public CreatureScript
{
public:
	npc_gorebound_spirit() : CreatureScript("npc_gorebound_spirit") { }

	struct npc_gorebound_spiritAI : public ScriptedAI
	{
		npc_gorebound_spiritAI(Creature* p_Creature)
			: ScriptedAI(p_Creature) { }

		EventMap m_Events;

		void Reset() override
		{
			m_Events.Reset();
			me->SetReactState(REACT_AGGRESSIVE);
			m_Events.ScheduleEvent(1 /*EventCrushingDarkness*/, 5000);
		}

		void UpdateAI(uint32 p_Diff) override
		{
			if (!UpdateVictim())
				return;

			m_Events.Update(p_Diff);

			while (uint32 l_EventId = m_Events.ExecuteEvent())
			{
				switch (l_EventId)
				{
					case 1: ///< Crushing Darkness
						me->CastSpell(me->GetVictim(), eGorefiendSpells::SpellCrushingDarkness, false);
						m_Events.ScheduleEvent(1, 8000);
						break;
					default:
						break;
				}
			}

			DoMeleeAttackIfReady();
		}
	};

	CreatureAI* GetAI(Creature* p_Creature) const override
	{
		return new npc_gorebound_spiritAI(p_Creature);
	}
};

/// ---------------------------------------------------------------------------
/// Tortured Essence - 93145
/// Ranged shadow add that applies a DoT to the trapped player.
/// ---------------------------------------------------------------------------
class npc_tortured_essence : public CreatureScript
{
public:
	npc_tortured_essence() : CreatureScript("npc_tortured_essence") { }

	struct npc_tortured_essenceAI : public ScriptedAI
	{
		npc_tortured_essenceAI(Creature* p_Creature)
			: ScriptedAI(p_Creature) { }

		EventMap m_Events;

		void Reset() override
		{
			m_Events.Reset();
			me->SetReactState(REACT_AGGRESSIVE);
			m_Events.ScheduleEvent(1 /*EventGoreboundCorrupt*/, 3000);
		}

		void UpdateAI(uint32 p_Diff) override
		{
			if (!UpdateVictim())
				return;

			m_Events.Update(p_Diff);

			if (me->HasUnitState(UNIT_STATE_CASTING))
				return;

			while (uint32 l_EventId = m_Events.ExecuteEvent())
			{
				switch (l_EventId)
				{
					case 1: ///< Gorebound Corrupt DoT
						me->CastSpell(me->GetVictim(), eGorefiendSpells::SpellGoreboundCorrupt, false);
						m_Events.ScheduleEvent(1, 6000);
						break;
					default:
						break;
				}
			}

			DoMeleeAttackIfReady();
		}
	};

	CreatureAI* GetAI(Creature* p_Creature) const override
	{
		return new npc_tortured_essenceAI(p_Creature);
	}
};

/// ---------------------------------------------------------------------------
void AddSC_boss_gorefiend_hfc()
{
	/// Boss
	new boss_gorefiend_hfc();

	/// Creatures
	new npc_gorebound_spirit();
	new npc_tortured_essence();
}
