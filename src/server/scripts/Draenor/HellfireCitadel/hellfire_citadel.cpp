///////////////////////////////////////////////////////////////////////////////
///
///  MILLENIUM-STUDIO
///  Copyright 2016 Millenium-studio SARL
///  All Rights Reserved.
///
///////////////////////////////////////////////////////////////////////////////
///
///  Shared Hellfire Citadel scripts (trash / GO / helpers).
///  Boss scripts live in boss_*_hfc.cpp.
///
///  Trash reference:
///  https://www.wowhead.com/zone=7545/hellfire-citadel
///  https://www.wowhead.com/guide/raiding/hellfire-citadel/kormrok-strategy-guide
///  https://www.wowhead.com/guide/raiding/hellfire-citadel/hellfire-high-council-strategy-guide
///
///////////////////////////////////////////////////////////////////////////////

#include "hellfire_citadel.h"

/// ---------------------------------------------------------------------------
/// Hellfire Antechamber / Passage
/// ---------------------------------------------------------------------------

/// Hellfire Guardian - 94808 / 94985
class npc_hfc_hellfire_guardian : public CreatureScript
{
public:
	npc_hfc_hellfire_guardian() : CreatureScript("npc_hfc_hellfire_guardian") { }

	enum eEvents
	{
		EventFelStrike = 1,
		EventShieldLeap,
	};

	struct npc_hfc_hellfire_guardianAI : public ScriptedAI
	{
		npc_hfc_hellfire_guardianAI(Creature* p_Creature) : ScriptedAI(p_Creature) { }

		EventMap m_Events;

		void Reset() override
		{
			m_Events.Reset();
		}

		void EnterCombat(Unit* /*p_Who*/) override
		{
			m_Events.ScheduleEvent(eEvents::EventFelStrike, 5 * TimeConstants::IN_MILLISECONDS);
			m_Events.ScheduleEvent(eEvents::EventShieldLeap, 12 * TimeConstants::IN_MILLISECONDS);
		}

		void UpdateAI(uint32 const p_Diff) override
		{
			if (!UpdateVictim())
				return;

			m_Events.Update(p_Diff);

			if (me->HasUnitState(UnitState::UNIT_STATE_CASTING))
				return;

			switch (m_Events.ExecuteEvent())
			{
				case eEvents::EventFelStrike:
				{
					if (Unit* l_Target = SelectTarget(SelectAggroTarget::SELECT_TARGET_TOPAGGRO))
						me->CastSpell(l_Target, eCitadelSpells::SpellFelStrike, false);
					m_Events.ScheduleEvent(eEvents::EventFelStrike, 8 * TimeConstants::IN_MILLISECONDS);
					break;
				}
				case eEvents::EventShieldLeap:
				{
					if (Unit* l_Target = SelectTarget(SelectAggroTarget::SELECT_TARGET_RANDOM, 0, 40.0f, true))
					{
						me->CastSpell(l_Target, eCitadelSpells::SpellShieldLeap, false);
						me->CastSpell(l_Target, eCitadelSpells::SpellShieldLeapImpact, true);
						me->CastSpell(me, eCitadelSpells::SpellFelStrike, true);
					}
					m_Events.ScheduleEvent(eEvents::EventShieldLeap, 18 * TimeConstants::IN_MILLISECONDS);
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
		return new npc_hfc_hellfire_guardianAI(p_Creature);
	}
};

/// Fel Hellweaver - 94806
class npc_hfc_fel_hellweaver : public CreatureScript
{
public:
	npc_hfc_fel_hellweaver() : CreatureScript("npc_hfc_fel_hellweaver") { }

	enum eEvents
	{
		EventFelFireball = 1,
		EventHellweaving,
		EventOrbOfDestruction,
	};

	struct npc_hfc_fel_hellweaverAI : public ScriptedAI
	{
		npc_hfc_fel_hellweaverAI(Creature* p_Creature) : ScriptedAI(p_Creature) { }

		EventMap m_Events;

		void Reset() override
		{
			m_Events.Reset();
		}

		void EnterCombat(Unit* /*p_Who*/) override
		{
			m_Events.ScheduleEvent(eEvents::EventFelFireball, 3 * TimeConstants::IN_MILLISECONDS);
			m_Events.ScheduleEvent(eEvents::EventHellweaving, 8 * TimeConstants::IN_MILLISECONDS);
			m_Events.ScheduleEvent(eEvents::EventOrbOfDestruction, 15 * TimeConstants::IN_MILLISECONDS);
		}

		void UpdateAI(uint32 const p_Diff) override
		{
			if (!UpdateVictim())
				return;

			m_Events.Update(p_Diff);

			if (me->HasUnitState(UnitState::UNIT_STATE_CASTING))
				return;

			switch (m_Events.ExecuteEvent())
			{
				case eEvents::EventFelFireball:
				{
					if (Unit* l_Target = SelectTarget(SelectAggroTarget::SELECT_TARGET_RANDOM, 0, 40.0f, true))
						me->CastSpell(l_Target, eCitadelSpells::SpellFelFireball, false);
					m_Events.ScheduleEvent(eEvents::EventFelFireball, 5 * TimeConstants::IN_MILLISECONDS);
					break;
				}
				case eEvents::EventHellweaving:
				{
					if (Unit* l_Target = SelectTarget(SelectAggroTarget::SELECT_TARGET_RANDOM, 0, 40.0f, true))
						me->CastSpell(l_Target, eCitadelSpells::SpellHellweaving, false);
					m_Events.ScheduleEvent(eEvents::EventHellweaving, 14 * TimeConstants::IN_MILLISECONDS);
					break;
				}
				case eEvents::EventOrbOfDestruction:
				{
					if (Unit* l_Target = SelectTarget(SelectAggroTarget::SELECT_TARGET_RANDOM, 0, 40.0f, true))
						me->CastSpell(l_Target, eCitadelSpells::SpellOrbOfDestruction, false);
					m_Events.ScheduleEvent(eEvents::EventOrbOfDestruction, 20 * TimeConstants::IN_MILLISECONDS);
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
		return new npc_hfc_fel_hellweaverAI(p_Creature);
	}
};

/// Fel Touched Seer - 94916
class npc_hfc_fel_touched_seer : public CreatureScript
{
public:
	npc_hfc_fel_touched_seer() : CreatureScript("npc_hfc_fel_touched_seer") { }

	enum eEvents
	{
		EventFelCauterize = 1,
		EventFelLavaBeam,
	};

	struct npc_hfc_fel_touched_seerAI : public ScriptedAI
	{
		npc_hfc_fel_touched_seerAI(Creature* p_Creature) : ScriptedAI(p_Creature) { }

		EventMap m_Events;

		void Reset() override
		{
			m_Events.Reset();
		}

		void EnterCombat(Unit* /*p_Who*/) override
		{
			m_Events.ScheduleEvent(eEvents::EventFelCauterize, 6 * TimeConstants::IN_MILLISECONDS);
			m_Events.ScheduleEvent(eEvents::EventFelLavaBeam, 4 * TimeConstants::IN_MILLISECONDS);
		}

		void UpdateAI(uint32 const p_Diff) override
		{
			if (!UpdateVictim())
				return;

			m_Events.Update(p_Diff);

			if (me->HasUnitState(UnitState::UNIT_STATE_CASTING))
				return;

			switch (m_Events.ExecuteEvent())
			{
				case eEvents::EventFelCauterize:
				{
					if (Unit* l_Ally = me->SelectNearbyMostInjuredAlly())
						me->CastSpell(l_Ally, eCitadelSpells::SpellFelCauterize, false);
					m_Events.ScheduleEvent(eEvents::EventFelCauterize, 12 * TimeConstants::IN_MILLISECONDS);
					break;
				}
				case eEvents::EventFelLavaBeam:
				{
					if (Unit* l_Target = SelectTarget(SelectAggroTarget::SELECT_TARGET_TOPAGGRO))
						me->CastSpell(l_Target, eCitadelSpells::SpellFelLavaBeam, false);
					m_Events.ScheduleEvent(eEvents::EventFelLavaBeam, 10 * TimeConstants::IN_MILLISECONDS);
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
		return new npc_hfc_fel_touched_seerAI(p_Creature);
	}
};

/// Shambling Hulk - 94804
class npc_hfc_shambling_hulk : public CreatureScript
{
public:
	npc_hfc_shambling_hulk() : CreatureScript("npc_hfc_shambling_hulk") { }

	enum eEvents
	{
		EventFelSlam = 1,
		EventBrutalShout,
	};

	struct npc_hfc_shambling_hulkAI : public ScriptedAI
	{
		npc_hfc_shambling_hulkAI(Creature* p_Creature) : ScriptedAI(p_Creature) { }

		EventMap m_Events;

		void Reset() override
		{
			m_Events.Reset();
		}

		void EnterCombat(Unit* /*p_Who*/) override
		{
			m_Events.ScheduleEvent(eEvents::EventFelSlam, 10 * TimeConstants::IN_MILLISECONDS);
			m_Events.ScheduleEvent(eEvents::EventBrutalShout, 15 * TimeConstants::IN_MILLISECONDS);
		}

		void UpdateAI(uint32 const p_Diff) override
		{
			if (!UpdateVictim())
				return;

			m_Events.Update(p_Diff);

			if (me->HasUnitState(UnitState::UNIT_STATE_CASTING))
				return;

			switch (m_Events.ExecuteEvent())
			{
				case eEvents::EventFelSlam:
				{
					me->CastSpell(me, eCitadelSpells::SpellFelSlam, false);
					m_Events.ScheduleEvent(eEvents::EventFelSlam, 20 * TimeConstants::IN_MILLISECONDS);
					break;
				}
				case eEvents::EventBrutalShout:
				{
					me->CastSpell(me, eCitadelSpells::SpellBrutalShout, false);
					/// Casts more often as health drops
					uint32 l_CD = me->GetHealthPct() < 40.0f ? 8 * TimeConstants::IN_MILLISECONDS
															: 16 * TimeConstants::IN_MILLISECONDS;
					m_Events.ScheduleEvent(eEvents::EventBrutalShout, l_CD);
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
		return new npc_hfc_shambling_hulkAI(p_Creature);
	}
};

/// Iron Peon - 94924
class npc_hfc_iron_peon : public CreatureScript
{
public:
	npc_hfc_iron_peon() : CreatureScript("npc_hfc_iron_peon") { }

	enum eEvents
	{
		EventHammerThrow = 1,
		EventExplosiveCrate,
	};

	struct npc_hfc_iron_peonAI : public ScriptedAI
	{
		npc_hfc_iron_peonAI(Creature* p_Creature) : ScriptedAI(p_Creature) { }

		EventMap m_Events;

		void Reset() override
		{
			m_Events.Reset();
		}

		void EnterCombat(Unit* /*p_Who*/) override
		{
			m_Events.ScheduleEvent(eEvents::EventHammerThrow, 6 * TimeConstants::IN_MILLISECONDS);
			m_Events.ScheduleEvent(eEvents::EventExplosiveCrate, 14 * TimeConstants::IN_MILLISECONDS);
		}

		void UpdateAI(uint32 const p_Diff) override
		{
			if (!UpdateVictim())
				return;

			m_Events.Update(p_Diff);

			if (me->HasUnitState(UnitState::UNIT_STATE_CASTING))
				return;

			switch (m_Events.ExecuteEvent())
			{
				case eEvents::EventHammerThrow:
				{
					if (Unit* l_Target = SelectTarget(SelectAggroTarget::SELECT_TARGET_RANDOM, 0, 40.0f, true))
						me->CastSpell(l_Target, eCitadelSpells::SpellHammerThrow, false);
					m_Events.ScheduleEvent(eEvents::EventHammerThrow, 10 * TimeConstants::IN_MILLISECONDS);
					break;
				}
				case eEvents::EventExplosiveCrate:
				{
					if (Unit* l_Target = SelectTarget(SelectAggroTarget::SELECT_TARGET_RANDOM, 0, 40.0f, true))
						me->CastSpell(l_Target, eCitadelSpells::SpellThrowExplosiveCrate, false);
					m_Events.ScheduleEvent(eEvents::EventExplosiveCrate, 18 * TimeConstants::IN_MILLISECONDS);
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
		return new npc_hfc_iron_peonAI(p_Creature);
	}
};

/// Gorebound Assassin - 94832
class npc_hfc_gorebound_assassin : public CreatureScript
{
public:
	npc_hfc_gorebound_assassin() : CreatureScript("npc_hfc_gorebound_assassin") { }

	enum eEvents
	{
		EventFelPoison = 1,
		EventCheapShot,
	};

	struct npc_hfc_gorebound_assassinAI : public ScriptedAI
	{
		npc_hfc_gorebound_assassinAI(Creature* p_Creature) : ScriptedAI(p_Creature) { }

		EventMap m_Events;
		uint64 m_FixateGuid;

		void Reset() override
		{
			m_Events.Reset();
			m_FixateGuid = 0;
			me->SetReactState(REACT_AGGRESSIVE);
		}

		void EnterCombat(Unit* p_Who) override
		{
			if (p_Who)
			{
				me->CastSpell(p_Who, eCitadelSpells::SpellCheapShot, true);
				m_FixateGuid = p_Who->GetGUID();
			}

			m_Events.ScheduleEvent(eEvents::EventFelPoison, 8 * TimeConstants::IN_MILLISECONDS);
			m_Events.ScheduleEvent(eEvents::EventCheapShot, 20 * TimeConstants::IN_MILLISECONDS);
		}

		void UpdateAI(uint32 const p_Diff) override
		{
			if (!UpdateVictim())
				return;

			m_Events.Update(p_Diff);

			if (me->HasUnitState(UnitState::UNIT_STATE_CASTING))
				return;

			switch (m_Events.ExecuteEvent())
			{
				case eEvents::EventFelPoison:
				{
					if (Unit* l_Target = SelectTarget(SelectAggroTarget::SELECT_TARGET_RANDOM, 0, 30.0f, true))
						me->CastSpell(l_Target, eCitadelSpells::SpellFelPoison, false);
					m_Events.ScheduleEvent(eEvents::EventFelPoison, 12 * TimeConstants::IN_MILLISECONDS);
					break;
				}
				case eEvents::EventCheapShot:
				{
					if (Unit* l_Target = SelectTarget(SelectAggroTarget::SELECT_TARGET_RANDOM, 0, 10.0f, true))
					{
						me->CastSpell(l_Target, eCitadelSpells::SpellCheapShot, true);
						m_FixateGuid = l_Target->GetGUID();
					}
					m_Events.ScheduleEvent(eEvents::EventCheapShot, 22 * TimeConstants::IN_MILLISECONDS);
					break;
				}
				default:
					break;
			}

			if (Unit* l_Fixate = Unit::GetUnit(*me, m_FixateGuid))
			{
				if (l_Fixate->IsAlive())
					AttackStart(l_Fixate);
			}

			DoMeleeAttackIfReady();
		}
	};

	CreatureAI* GetAI(Creature* p_Creature) const override
	{
		return new npc_hfc_gorebound_assassinAI(p_Creature);
	}
};

/// Fel-Starved Trainee - 94947
class npc_hfc_fel_starved_trainee : public CreatureScript
{
public:
	npc_hfc_fel_starved_trainee() : CreatureScript("npc_hfc_fel_starved_trainee") { }

	enum eEvents
	{
		EventFelStrike = 1,
	};

	struct npc_hfc_fel_starved_traineeAI : public ScriptedAI
	{
		npc_hfc_fel_starved_traineeAI(Creature* p_Creature) : ScriptedAI(p_Creature) { }

		EventMap m_Events;

		void Reset() override
		{
			m_Events.Reset();
		}

		void EnterCombat(Unit* /*p_Who*/) override
		{
			m_Events.ScheduleEvent(eEvents::EventFelStrike, 4 * TimeConstants::IN_MILLISECONDS);
		}

		void UpdateAI(uint32 const p_Diff) override
		{
			if (!UpdateVictim())
				return;

			m_Events.Update(p_Diff);

			if (me->HasUnitState(UnitState::UNIT_STATE_CASTING))
				return;

			if (m_Events.ExecuteEvent() == eEvents::EventFelStrike)
			{
				if (Unit* l_Target = SelectTarget(SelectAggroTarget::SELECT_TARGET_TOPAGGRO))
					me->CastSpell(l_Target, eCitadelSpells::SpellFelStrike, false);
				m_Events.ScheduleEvent(eEvents::EventFelStrike, 7 * TimeConstants::IN_MILLISECONDS);
			}

			DoMeleeAttackIfReady();
		}
	};

	CreatureAI* GetAI(Creature* p_Creature) const override
	{
		return new npc_hfc_fel_starved_traineeAI(p_Creature);
	}
};

/// ---------------------------------------------------------------------------
/// Graggra - 94995 (mini-boss before High Council)
/// ---------------------------------------------------------------------------
class npc_hfc_graggra : public CreatureScript
{
public:
	npc_hfc_graggra() : CreatureScript("npc_hfc_graggra") { }

	enum eEvents
	{
		EventBlazingFelTouch = 1,
		EventGraggraSmash,
	};

	struct npc_hfc_graggraAI : public ScriptedAI
	{
		npc_hfc_graggraAI(Creature* p_Creature) : ScriptedAI(p_Creature) { }

		EventMap m_Events;

		void Reset() override
		{
			m_Events.Reset();
		}

		void EnterCombat(Unit* /*p_Who*/) override
		{
			m_Events.ScheduleEvent(eEvents::EventBlazingFelTouch, 12 * TimeConstants::IN_MILLISECONDS);
			m_Events.ScheduleEvent(eEvents::EventGraggraSmash, 20 * TimeConstants::IN_MILLISECONDS);
		}

		void UpdateAI(uint32 const p_Diff) override
		{
			if (!UpdateVictim())
				return;

			m_Events.Update(p_Diff);

			if (me->HasUnitState(UnitState::UNIT_STATE_CASTING))
				return;

			switch (m_Events.ExecuteEvent())
			{
				case eEvents::EventBlazingFelTouch:
				{
					me->CastSpell(me, eCitadelSpells::SpellBlazingFelTouch, false);
					m_Events.ScheduleEvent(eEvents::EventBlazingFelTouch, 30 * TimeConstants::IN_MILLISECONDS);
					break;
				}
				case eEvents::EventGraggraSmash:
				{
					if (Unit* l_Target = SelectTarget(SelectAggroTarget::SELECT_TARGET_RANDOM, 0, 40.0f, true))
					{
						me->CastSpell(l_Target, eCitadelSpells::SpellGraggraSmashLeap, false);
						me->CastSpell(l_Target, eCitadelSpells::SpellGraggraSmash, true);
					}
					m_Events.ScheduleEvent(eEvents::EventGraggraSmash, 25 * TimeConstants::IN_MILLISECONDS);
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
		return new npc_hfc_graggraAI(p_Creature);
	}
};

/// ---------------------------------------------------------------------------
/// Pits of Mannoroth — named magnaron trash
/// ---------------------------------------------------------------------------
class npc_hfc_pit_magnaron : public CreatureScript
{
public:
	npc_hfc_pit_magnaron() : CreatureScript("npc_hfc_pit_magnaron") { }

	enum eEvents
	{
		EventGraspingHands = 1,
		EventSwat,
		EventLocalizedExplosion,
		EventAgitate,
	};

	struct npc_hfc_pit_magnaronAI : public ScriptedAI
	{
		npc_hfc_pit_magnaronAI(Creature* p_Creature) : ScriptedAI(p_Creature) { }

		EventMap m_Events;

		void Reset() override
		{
			m_Events.Reset();
		}

		void EnterCombat(Unit* /*p_Who*/) override
		{
			m_Events.ScheduleEvent(eEvents::EventGraspingHands, 15 * TimeConstants::IN_MILLISECONDS);

			switch (me->GetEntry())
			{
				case eCitadelCreatures::NpcSovokk:
					m_Events.ScheduleEvent(eEvents::EventSwat, 12 * TimeConstants::IN_MILLISECONDS);
					m_Events.ScheduleEvent(eEvents::EventAgitate, 18 * TimeConstants::IN_MILLISECONDS);
					break;
				case eCitadelCreatures::NpcMorkronn:
					m_Events.ScheduleEvent(eEvents::EventLocalizedExplosion, 14 * TimeConstants::IN_MILLISECONDS);
					m_Events.ScheduleEvent(eEvents::EventAgitate, 18 * TimeConstants::IN_MILLISECONDS);
					break;
				default: ///< Togdrov
					break;
			}
		}

		void UpdateAI(uint32 const p_Diff) override
		{
			if (!UpdateVictim())
				return;

			m_Events.Update(p_Diff);

			if (me->HasUnitState(UnitState::UNIT_STATE_CASTING))
				return;

			switch (m_Events.ExecuteEvent())
			{
				case eEvents::EventGraspingHands:
				{
					me->CastSpell(me, eCitadelSpells::SpellTrashGraspingHands, false);
					m_Events.ScheduleEvent(eEvents::EventGraspingHands, 25 * TimeConstants::IN_MILLISECONDS);
					break;
				}
				case eEvents::EventSwat:
				{
					if (Unit* l_Target = SelectTarget(SelectAggroTarget::SELECT_TARGET_TOPAGGRO))
						me->CastSpell(l_Target, eCitadelSpells::SpellSwat, false);
					m_Events.ScheduleEvent(eEvents::EventSwat, 16 * TimeConstants::IN_MILLISECONDS);
					break;
				}
				case eEvents::EventLocalizedExplosion:
				{
					if (Unit* l_Target = SelectTarget(SelectAggroTarget::SELECT_TARGET_TOPAGGRO))
						me->CastSpell(l_Target, eCitadelSpells::SpellLocalizedExplosion, false);
					m_Events.ScheduleEvent(eEvents::EventLocalizedExplosion, 20 * TimeConstants::IN_MILLISECONDS);
					break;
				}
				case eEvents::EventAgitate:
				{
					me->CastSpell(me, eCitadelSpells::SpellAgitate, false);
					m_Events.ScheduleEvent(eEvents::EventAgitate, 22 * TimeConstants::IN_MILLISECONDS);
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
		return new npc_hfc_pit_magnaronAI(p_Creature);
	}
};

/// Fel Extractor - 94285
class npc_hfc_fel_extractor : public CreatureScript
{
public:
	npc_hfc_fel_extractor() : CreatureScript("npc_hfc_fel_extractor") { }

	enum eEvents
	{
		EventPrimalEnergies = 1,
	};

	struct npc_hfc_fel_extractorAI : public ScriptedAI
	{
		npc_hfc_fel_extractorAI(Creature* p_Creature) : ScriptedAI(p_Creature) { }

		EventMap m_Events;

		void Reset() override
		{
			m_Events.Reset();
		}

		void EnterCombat(Unit* /*p_Who*/) override
		{
			m_Events.ScheduleEvent(eEvents::EventPrimalEnergies, 8 * TimeConstants::IN_MILLISECONDS);
		}

		void UpdateAI(uint32 const p_Diff) override
		{
			if (!UpdateVictim())
				return;

			m_Events.Update(p_Diff);

			if (me->HasUnitState(UnitState::UNIT_STATE_CASTING))
				return;

			if (m_Events.ExecuteEvent() == eEvents::EventPrimalEnergies)
			{
				if (Unit* l_Target = SelectTarget(SelectAggroTarget::SELECT_TARGET_RANDOM, 0, 40.0f, true))
					me->CastSpell(l_Target, eCitadelSpells::SpellPrimalEnergies, false);
				m_Events.ScheduleEvent(eEvents::EventPrimalEnergies, 16 * TimeConstants::IN_MILLISECONDS);
			}

			DoMeleeAttackIfReady();
		}
	};

	CreatureAI* GetAI(Creature* p_Creature) const override
	{
		return new npc_hfc_fel_extractorAI(p_Creature);
	}
};

/// Shadow Infuser - 94283
class npc_hfc_shadow_infuser : public CreatureScript
{
public:
	npc_hfc_shadow_infuser() : CreatureScript("npc_hfc_shadow_infuser") { }

	enum eEvents
	{
		EventResidualShadows = 1,
	};

	struct npc_hfc_shadow_infuserAI : public ScriptedAI
	{
		npc_hfc_shadow_infuserAI(Creature* p_Creature) : ScriptedAI(p_Creature) { }

		EventMap m_Events;

		void Reset() override
		{
			m_Events.Reset();
		}

		void EnterCombat(Unit* /*p_Who*/) override
		{
			m_Events.ScheduleEvent(eEvents::EventResidualShadows, 9 * TimeConstants::IN_MILLISECONDS);
		}

		void UpdateAI(uint32 const p_Diff) override
		{
			if (!UpdateVictim())
				return;

			m_Events.Update(p_Diff);

			if (me->HasUnitState(UnitState::UNIT_STATE_CASTING))
				return;

			if (m_Events.ExecuteEvent() == eEvents::EventResidualShadows)
			{
				if (Unit* l_Target = SelectTarget(SelectAggroTarget::SELECT_TARGET_RANDOM, 0, 40.0f, true))
					me->CastSpell(l_Target, eCitadelSpells::SpellResidualShadows, false);
				m_Events.ScheduleEvent(eEvents::EventResidualShadows, 16 * TimeConstants::IN_MILLISECONDS);
			}

			DoMeleeAttackIfReady();
		}
	};

	CreatureAI* GetAI(Creature* p_Creature) const override
	{
		return new npc_hfc_shadow_infuserAI(p_Creature);
	}
};

/// Fiery Enkindler - 94284
class npc_hfc_fiery_enkindler : public CreatureScript
{
public:
	npc_hfc_fiery_enkindler() : CreatureScript("npc_hfc_fiery_enkindler") { }

	enum eEvents
	{
		EventFocusedFire = 1,
	};

	struct npc_hfc_fiery_enkindlerAI : public ScriptedAI
	{
		npc_hfc_fiery_enkindlerAI(Creature* p_Creature) : ScriptedAI(p_Creature) { }

		EventMap m_Events;

		void Reset() override
		{
			m_Events.Reset();
		}

		void EnterCombat(Unit* /*p_Who*/) override
		{
			m_Events.ScheduleEvent(eEvents::EventFocusedFire, 10 * TimeConstants::IN_MILLISECONDS);
		}

		void UpdateAI(uint32 const p_Diff) override
		{
			if (!UpdateVictim())
				return;

			m_Events.Update(p_Diff);

			if (me->HasUnitState(UnitState::UNIT_STATE_CASTING))
				return;

			if (m_Events.ExecuteEvent() == eEvents::EventFocusedFire)
			{
				if (Unit* l_Target = SelectTarget(SelectAggroTarget::SELECT_TARGET_RANDOM, 0, 40.0f, true))
					me->CastSpell(l_Target, eCitadelSpells::SpellFocusedFire, false);
				m_Events.ScheduleEvent(eEvents::EventFocusedFire, 16 * TimeConstants::IN_MILLISECONDS);
			}

			DoMeleeAttackIfReady();
		}
	};

	CreatureAI* GetAI(Creature* p_Creature) const override
	{
		return new npc_hfc_fiery_enkindlerAI(p_Creature);
	}
};

/// Armored Skullsmasher - 94859
class npc_hfc_armored_skullsmasher : public CreatureScript
{
public:
	npc_hfc_armored_skullsmasher() : CreatureScript("npc_hfc_armored_skullsmasher") { }

	enum eEvents
	{
		EventMuleKick = 1,
		EventChargingStrike,
		EventMightySmash,
	};

	struct npc_hfc_armored_skullsmasherAI : public ScriptedAI
	{
		npc_hfc_armored_skullsmasherAI(Creature* p_Creature) : ScriptedAI(p_Creature) { }

		EventMap m_Events;

		void Reset() override
		{
			m_Events.Reset();
		}

		void EnterCombat(Unit* /*p_Who*/) override
		{
			m_Events.ScheduleEvent(eEvents::EventMuleKick, 8 * TimeConstants::IN_MILLISECONDS);
			m_Events.ScheduleEvent(eEvents::EventMightySmash, 16 * TimeConstants::IN_MILLISECONDS);
		}

		void UpdateAI(uint32 const p_Diff) override
		{
			if (!UpdateVictim())
				return;

			m_Events.Update(p_Diff);

			if (me->HasUnitState(UnitState::UNIT_STATE_CASTING))
				return;

			switch (m_Events.ExecuteEvent())
			{
				case eEvents::EventMuleKick:
				{
					if (Unit* l_Target = SelectTarget(SelectAggroTarget::SELECT_TARGET_TOPAGGRO))
						me->CastSpell(l_Target, eCitadelSpells::SpellMuleKick, false);
					m_Events.ScheduleEvent(eEvents::EventChargingStrike, 1 * TimeConstants::IN_MILLISECONDS);
					m_Events.ScheduleEvent(eEvents::EventMuleKick, 18 * TimeConstants::IN_MILLISECONDS);
					break;
				}
				case eEvents::EventChargingStrike:
				{
					if (Unit* l_Target = SelectTarget(SelectAggroTarget::SELECT_TARGET_FARTHEST, 0, 50.0f, true))
						me->CastSpell(l_Target, eCitadelSpells::SpellChargingStrike, false);
					break;
				}
				case eEvents::EventMightySmash:
				{
					me->CastSpell(me, eCitadelSpells::SpellMightySmash, false);
					m_Events.ScheduleEvent(eEvents::EventMightySmash, 20 * TimeConstants::IN_MILLISECONDS);
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
		return new npc_hfc_armored_skullsmasherAI(p_Creature);
	}
};

/// Keen-Eyed Gronnstalker - 94894
class npc_hfc_keen_eyed_gronnstalker : public CreatureScript
{
public:
	npc_hfc_keen_eyed_gronnstalker() : CreatureScript("npc_hfc_keen_eyed_gronnstalker") { }

	enum eEvents
	{
		EventSlagTrap = 1,
	};

	struct npc_hfc_keen_eyed_gronnstalkerAI : public ScriptedAI
	{
		npc_hfc_keen_eyed_gronnstalkerAI(Creature* p_Creature) : ScriptedAI(p_Creature) { }

		EventMap m_Events;

		void Reset() override
		{
			m_Events.Reset();
		}

		void EnterCombat(Unit* /*p_Who*/) override
		{
			m_Events.ScheduleEvent(eEvents::EventSlagTrap, 10 * TimeConstants::IN_MILLISECONDS);
		}

		void UpdateAI(uint32 const p_Diff) override
		{
			if (!UpdateVictim())
				return;

			m_Events.Update(p_Diff);

			if (me->HasUnitState(UnitState::UNIT_STATE_CASTING))
				return;

			if (m_Events.ExecuteEvent() == eEvents::EventSlagTrap)
			{
				if (Unit* l_Target = SelectTarget(SelectAggroTarget::SELECT_TARGET_RANDOM, 0, 40.0f, true))
					me->CastSpell(l_Target, eCitadelSpells::SpellSlagTrap, false);
				m_Events.ScheduleEvent(eEvents::EventSlagTrap, 14 * TimeConstants::IN_MILLISECONDS);
			}

			DoMeleeAttackIfReady();
		}
	};

	CreatureAI* GetAI(Creature* p_Creature) const override
	{
		return new npc_hfc_keen_eyed_gronnstalkerAI(p_Creature);
	}
};

/// Grim Ambusher - 94861 (melee + occasional leap)
class npc_hfc_grim_ambusher : public CreatureScript
{
public:
	npc_hfc_grim_ambusher() : CreatureScript("npc_hfc_grim_ambusher") { }

	enum eEvents
	{
		EventLeap = 1,
	};

	struct npc_hfc_grim_ambusherAI : public ScriptedAI
	{
		npc_hfc_grim_ambusherAI(Creature* p_Creature) : ScriptedAI(p_Creature) { }

		EventMap m_Events;

		void Reset() override
		{
			m_Events.Reset();
		}

		void EnterCombat(Unit* /*p_Who*/) override
		{
			m_Events.ScheduleEvent(eEvents::EventLeap, 10 * TimeConstants::IN_MILLISECONDS);
		}

		void UpdateAI(uint32 const p_Diff) override
		{
			if (!UpdateVictim())
				return;

			m_Events.Update(p_Diff);

			if (m_Events.ExecuteEvent() == eEvents::EventLeap)
			{
				if (Unit* l_Target = SelectTarget(SelectAggroTarget::SELECT_TARGET_RANDOM, 0, 30.0f, true))
					me->GetMotionMaster()->MoveJump(l_Target->GetPositionX(), l_Target->GetPositionY(), l_Target->GetPositionZ(), 15.0f, 15.0f);
				m_Events.ScheduleEvent(eEvents::EventLeap, 15 * TimeConstants::IN_MILLISECONDS);
			}

			DoMeleeAttackIfReady();
		}
	};

	CreatureAI* GetAI(Creature* p_Creature) const override
	{
		return new npc_hfc_grim_ambusherAI(p_Creature);
	}
};

/// ---------------------------------------------------------------------------
/// Destructor's Rise trash (Zakuun approach) — first pass
/// https://www.wowhead.com/guide/raiding/hellfire-citadel/fel-lord-zakuun-strategy-guide
/// ---------------------------------------------------------------------------
class npc_hfc_mekbarash : public CreatureScript
{
public:
	npc_hfc_mekbarash() : CreatureScript("npc_hfc_mekbarash") { }

	enum eEvents
	{
		EventBurningBlades = 1,
	};

	enum eLocalSpells
	{
		/// Placeholder until exact aura ID is confirmed from sniff
		SpellBurningBlades = eCitadelSpells::SpellFelStrike,
	};

	struct npc_hfc_mekbarashAI : public ScriptedAI
	{
		npc_hfc_mekbarashAI(Creature* p_Creature) : ScriptedAI(p_Creature) { }

		EventMap m_Events;

		void Reset() override
		{
			m_Events.Reset();
		}

		void EnterCombat(Unit* /*p_Who*/) override
		{
			m_Events.ScheduleEvent(eEvents::EventBurningBlades, 5 * TimeConstants::IN_MILLISECONDS);
		}

		void UpdateAI(uint32 const p_Diff) override
		{
			if (!UpdateVictim())
				return;

			m_Events.Update(p_Diff);

			if (me->HasUnitState(UnitState::UNIT_STATE_CASTING))
				return;

			if (m_Events.ExecuteEvent() == eEvents::EventBurningBlades)
			{
				if (Unit* l_Target = SelectTarget(SelectAggroTarget::SELECT_TARGET_TOPAGGRO))
					me->CastSpell(l_Target, eLocalSpells::SpellBurningBlades, true);
				m_Events.ScheduleEvent(eEvents::EventBurningBlades, 6 * TimeConstants::IN_MILLISECONDS);
			}

			DoMeleeAttackIfReady();
		}
	};

	CreatureAI* GetAI(Creature* p_Creature) const override
	{
		return new npc_hfc_mekbarashAI(p_Creature);
	}
};

class npc_hfc_named_felguard : public CreatureScript
{
public:
	npc_hfc_named_felguard() : CreatureScript("npc_hfc_named_felguard") { }

	enum eEvents
	{
		EventWhirl = 1,
		EventHurl,
	};

	struct npc_hfc_named_felguardAI : public ScriptedAI
	{
		npc_hfc_named_felguardAI(Creature* p_Creature) : ScriptedAI(p_Creature) { }

		EventMap m_Events;

		void Reset() override
		{
			m_Events.Reset();
		}

		void EnterCombat(Unit* /*p_Who*/) override
		{
			m_Events.ScheduleEvent(eEvents::EventWhirl, 8 * TimeConstants::IN_MILLISECONDS);
			m_Events.ScheduleEvent(eEvents::EventHurl, 14 * TimeConstants::IN_MILLISECONDS);
		}

		void UpdateAI(uint32 const p_Diff) override
		{
			if (!UpdateVictim())
				return;

			m_Events.Update(p_Diff);

			if (me->HasUnitState(UnitState::UNIT_STATE_CASTING))
				return;

			switch (m_Events.ExecuteEvent())
			{
				case eEvents::EventWhirl:
				{
					me->CastSpell(me, eCitadelSpells::SpellMightySmash, false);
					m_Events.ScheduleEvent(eEvents::EventWhirl, 16 * TimeConstants::IN_MILLISECONDS);
					break;
				}
				case eEvents::EventHurl:
				{
					if (Unit* l_Target = SelectTarget(SelectAggroTarget::SELECT_TARGET_FARTHEST, 0, 40.0f, true))
						me->CastSpell(l_Target, eCitadelSpells::SpellChargingStrike, false);
					m_Events.ScheduleEvent(eEvents::EventHurl, 18 * TimeConstants::IN_MILLISECONDS);
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
		return new npc_hfc_named_felguardAI(p_Creature);
	}
};

class npc_hfc_succubus_temptress : public CreatureScript
{
public:
	npc_hfc_succubus_temptress() : CreatureScript("npc_hfc_succubus_temptress") { }

	enum eEvents
	{
		EventFelPoison = 1,
	};

	struct npc_hfc_succubus_temptressAI : public ScriptedAI
	{
		npc_hfc_succubus_temptressAI(Creature* p_Creature) : ScriptedAI(p_Creature) { }

		EventMap m_Events;

		void Reset() override
		{
			m_Events.Reset();
		}

		void EnterCombat(Unit* /*p_Who*/) override
		{
			m_Events.ScheduleEvent(eEvents::EventFelPoison, 6 * TimeConstants::IN_MILLISECONDS);
		}

		void UpdateAI(uint32 const p_Diff) override
		{
			if (!UpdateVictim())
				return;

			m_Events.Update(p_Diff);

			if (me->HasUnitState(UnitState::UNIT_STATE_CASTING))
				return;

			if (m_Events.ExecuteEvent() == eEvents::EventFelPoison)
			{
				if (Unit* l_Target = SelectTarget(SelectAggroTarget::SELECT_TARGET_RANDOM, 0, 30.0f, true))
					me->CastSpell(l_Target, eCitadelSpells::SpellFelPoison, false);
				m_Events.ScheduleEvent(eEvents::EventFelPoison, 10 * TimeConstants::IN_MILLISECONDS);
			}

			DoMeleeAttackIfReady();
		}
	};

	CreatureAI* GetAI(Creature* p_Creature) const override
	{
		return new npc_hfc_succubus_temptressAI(p_Creature);
	}
};

#ifndef __clang_analyzer__
void AddSC_hellfire_citadel()
{
	/// Hellfire Antechamber / Passage
	new npc_hfc_hellfire_guardian();
	new npc_hfc_fel_hellweaver();
	new npc_hfc_fel_touched_seer();
	new npc_hfc_shambling_hulk();
	new npc_hfc_iron_peon();
	new npc_hfc_gorebound_assassin();
	new npc_hfc_fel_starved_trainee();

	/// Mini-boss
	new npc_hfc_graggra();

	/// Pits of Mannoroth
	new npc_hfc_pit_magnaron();
	new npc_hfc_fel_extractor();
	new npc_hfc_shadow_infuser();
	new npc_hfc_fiery_enkindler();
	new npc_hfc_armored_skullsmasher();
	new npc_hfc_keen_eyed_gronnstalker();
	new npc_hfc_grim_ambusher();

	/// Destructor's Rise approach (first pass)
	new npc_hfc_mekbarash();
	new npc_hfc_named_felguard();
	new npc_hfc_succubus_temptress();
}
#endif
