///////////////////////////////////////////////////////////////////////////////
///
///  MILLENIUM-STUDIO
///  Copyright 2016 Millenium-studio SARL
///  All Rights Reserved.
///
///////////////////////////////////////////////////////////////////////////////

#include "boss_high_council_hfc.h"
#include "hellfire_citadel.h"

/// ---------------------------------------------------------------------------
/// Shared helpers
/// ---------------------------------------------------------------------------

/// Returns true when all three council members are dead.
static bool CouncilAllDead(InstanceScript* p_Instance)
{
	if (!p_Instance)
		return false;

	for (uint32 l_Entry : g_CouncilEntries)
	{
		uint64 l_Guid = p_Instance->GetData64(l_Entry);
		if (Creature* l_Boss = p_Instance->instance->GetCreature(l_Guid))
		{
			if (l_Boss->IsAlive())
				return false;
		}
	}

	return true;
}

/// Alert all alive council members to enter combat.
static void CouncilEngageAll(InstanceScript* p_Instance, uint32 p_IgnoreEntry)
{
	if (!p_Instance)
		return;

	for (uint32 l_Entry : g_CouncilEntries)
	{
		if (l_Entry == p_IgnoreEntry)
			continue;

		uint64 l_Guid = p_Instance->GetData64(l_Entry);
		if (Creature* l_Boss = p_Instance->instance->GetCreature(l_Guid))
		{
			if (l_Boss->IsAlive() && !l_Boss->isInCombat())
				l_Boss->SetInCombatWithZone();
		}
	}
}

/// Force all alive council members into evade (called on wipe).
static void CouncilEvadeAll(InstanceScript* p_Instance, uint32 p_IgnoreEntry)
{
	if (!p_Instance)
		return;

	for (uint32 l_Entry : g_CouncilEntries)
	{
		if (l_Entry == p_IgnoreEntry)
			continue;

		uint64 l_Guid = p_Instance->GetData64(l_Entry);
		if (Creature* l_Boss = p_Instance->instance->GetCreature(l_Guid))
		{
			if (l_Boss->IsAlive() && l_Boss->isInCombat())
				l_Boss->AI()->EnterEvadeMode();
		}
	}
}

/// ---------------------------------------------------------------------------
/// Dia Darkwhisper - 92144
/// ---------------------------------------------------------------------------
class boss_dia_darkwhisper : public CreatureScript
{
public:
	boss_dia_darkwhisper() : CreatureScript("boss_dia_darkwhisper") { }

	enum eTalks
	{
		TalkAggro   = 0,
		TalkSlay    = 1,
		TalkDeath   = 2,
		TalkAbility = 3,
	};

	struct boss_dia_darkwhisperAI : public BossAI
	{
		boss_dia_darkwhisperAI(Creature* p_Creature)
			: BossAI(p_Creature, eCitadelDatas::DataCouncil)
		{
			m_Instance = p_Creature->GetInstanceScript();
			m_Evading  = false;
		}

		InstanceScript* m_Instance;
		bool m_Evading;

		void Reset() override
		{
			_Reset();
			m_Evading = false;
			me->SetReactState(REACT_AGGRESSIVE);
		}

		void EnterCombat(Unit* /*p_Who*/) override
		{
			_EnterCombat();
			Talk(eTalks::TalkAggro);

			events.ScheduleEvent(eCouncilEvents::EventVoidBolt,         eCouncilTimers::TimerVoidBolt);
			events.ScheduleEvent(eCouncilEvents::EventNightmareVisage,  eCouncilTimers::TimerNightmareVisage);
			events.ScheduleEvent(eCouncilEvents::EventVoidHaze,         eCouncilTimers::TimerVoidHaze);

			CouncilEngageAll(m_Instance, eCitadelCreatures::BossDia);
		}

		void KilledUnit(Unit* p_Who) override
		{
			if (p_Who->IsPlayer())
				Talk(eTalks::TalkSlay);
		}

		void EnterEvadeMode() override
		{
			if (m_Evading)
				return;

			m_Evading = true;
			BossAI::EnterEvadeMode();
			CouncilEvadeAll(m_Instance, eCitadelCreatures::BossDia);
			m_Evading = false;
		}

		void JustDied(Unit* /*p_Killer*/) override
		{
			Talk(eTalks::TalkDeath);
			events.Reset();
			summons.DespawnAll();

			/// Apply Void Shroud damage reduction to nearby surviving allies - cosmetic flavour
			for (uint32 l_Entry : g_CouncilEntries)
			{
				if (l_Entry == eCitadelCreatures::BossDia)
					continue;

				if (!m_Instance)
					break;

				uint64 l_Guid = m_Instance->GetData64(l_Entry);
				if (Creature* l_Ally = m_Instance->instance->GetCreature(l_Guid))
				{
					if (l_Ally->IsAlive())
						l_Ally->CastSpell(l_Ally, eCouncilSpells::SpellVoidShroud, true);
				}
			}

			if (CouncilAllDead(m_Instance))
			{
				if (m_Instance)
					m_Instance->SetBossState(eCitadelDatas::DataCouncil, EncounterState::DONE);
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
					case eCouncilEvents::EventVoidBolt:
					{
						if (Unit* l_Target = me->GetVictim())
							me->CastSpell(l_Target, eCouncilSpells::SpellVoidBolt, false);
						events.ScheduleEvent(eCouncilEvents::EventVoidBolt, eCouncilTimers::TimerVoidBolt);
						break;
					}
					case eCouncilEvents::EventNightmareVisage:
					{
						Talk(eTalks::TalkAbility);
						if (Unit* l_Target = SelectTarget(SELECT_TARGET_RANDOM, 1, 40.0f, true))
							me->CastSpell(l_Target, eCouncilSpells::SpellNightmareVisage, false);
						events.ScheduleEvent(eCouncilEvents::EventNightmareVisage, eCouncilTimers::TimerNightmareVisage);
						break;
					}
					case eCouncilEvents::EventVoidHaze:
					{
						if (Unit* l_Target = SelectTarget(SELECT_TARGET_RANDOM, 0, 40.0f, true))
							me->CastSpell(l_Target, eCouncilSpells::SpellVoidHaze, false);
						events.ScheduleEvent(eCouncilEvents::EventVoidHaze, eCouncilTimers::TimerVoidHaze);
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
		return new boss_dia_darkwhisperAI(p_Creature);
	}
};

/// ---------------------------------------------------------------------------
/// Gurtogg Bloodboil - 92146
/// ---------------------------------------------------------------------------
class boss_gurtogg_bloodboil : public CreatureScript
{
public:
	boss_gurtogg_bloodboil() : CreatureScript("boss_gurtogg_bloodboil") { }

	enum eTalks
	{
		TalkAggro   = 0,
		TalkSlay    = 1,
		TalkDeath   = 2,
		TalkFelRage = 3,
		TalkAbility = 4,
	};

	struct boss_gurtogg_bloodboilAI : public BossAI
	{
		boss_gurtogg_bloodboilAI(Creature* p_Creature)
			: BossAI(p_Creature, eCitadelDatas::DataCouncil)
		{
			m_Instance = p_Creature->GetInstanceScript();
			m_Evading  = false;
		}

		InstanceScript* m_Instance;
		bool m_Evading;

		void Reset() override
		{
			_Reset();
			m_Evading = false;
			me->SetReactState(REACT_AGGRESSIVE);
		}

		void EnterCombat(Unit* /*p_Who*/) override
		{
			_EnterCombat();
			Talk(eTalks::TalkAggro);

			events.ScheduleEvent(eCouncilEvents::EventAcidicWound,  eCouncilTimers::TimerAcidicWound);
			events.ScheduleEvent(eCouncilEvents::EventFelRage,       eCouncilTimers::TimerFelRage);
			events.ScheduleEvent(eCouncilEvents::EventBloodyCleave,  eCouncilTimers::TimerBloodyCleave);

			CouncilEngageAll(m_Instance, eCitadelCreatures::BossGurtogg);
		}

		void KilledUnit(Unit* p_Who) override
		{
			if (p_Who->IsPlayer())
				Talk(eTalks::TalkSlay);
		}

		void EnterEvadeMode() override
		{
			if (m_Evading)
				return;

			m_Evading = true;
			BossAI::EnterEvadeMode();
			CouncilEvadeAll(m_Instance, eCitadelCreatures::BossGurtogg);
			m_Evading = false;
		}

		void JustDied(Unit* /*p_Killer*/) override
		{
			Talk(eTalks::TalkDeath);
			events.Reset();
			summons.DespawnAll();

			if (CouncilAllDead(m_Instance))
			{
				if (m_Instance)
					m_Instance->SetBossState(eCitadelDatas::DataCouncil, EncounterState::DONE);
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
					case eCouncilEvents::EventAcidicWound:
					{
						if (Unit* l_Target = me->GetVictim())
							me->CastSpell(l_Target, eCouncilSpells::SpellAcidicWound, false);
						events.ScheduleEvent(eCouncilEvents::EventAcidicWound, eCouncilTimers::TimerAcidicWound);
						break;
					}
					case eCouncilEvents::EventFelRage:
					{
						Talk(eTalks::TalkFelRage);
						me->CastSpell(me, eCouncilSpells::SpellFelRage, true);
						events.ScheduleEvent(eCouncilEvents::EventFelRage, eCouncilTimers::TimerFelRage);
						break;
					}
					case eCouncilEvents::EventBloodyCleave:
					{
						Talk(eTalks::TalkAbility);
						me->CastSpell(me, eCouncilSpells::SpellBloodyCleave, false);
						events.ScheduleEvent(eCouncilEvents::EventBloodyCleave, eCouncilTimers::TimerBloodyCleave);
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
		return new boss_gurtogg_bloodboilAI(p_Creature);
	}
};

/// ---------------------------------------------------------------------------
/// Jubei'thos - 92142
/// ---------------------------------------------------------------------------
class boss_jubeithos : public CreatureScript
{
public:
	boss_jubeithos() : CreatureScript("boss_jubeithos") { }

	enum eTalks
	{
		TalkMirror = 0,
		TalkSlay   = 1,
		TalkDeath  = 2,
	};

	struct boss_jubeithosAI : public BossAI
	{
		boss_jubeithosAI(Creature* p_Creature)
			: BossAI(p_Creature, eCitadelDatas::DataCouncil)
		{
			m_Instance = p_Creature->GetInstanceScript();
			m_Evading  = false;
		}

		InstanceScript* m_Instance;
		bool m_Evading;

		void Reset() override
		{
			_Reset();
			m_Evading = false;
			me->SetReactState(REACT_AGGRESSIVE);
		}

		void EnterCombat(Unit* /*p_Who*/) override
		{
			_EnterCombat();

			events.ScheduleEvent(eCouncilEvents::EventFelstormBlade, eCouncilTimers::TimerFelstormBlade);
			events.ScheduleEvent(eCouncilEvents::EventMirrorImage,   eCouncilTimers::TimerMirrorImage);

			CouncilEngageAll(m_Instance, eCitadelCreatures::BossJubeithos);
		}

		void KilledUnit(Unit* p_Who) override
		{
			if (p_Who->IsPlayer())
				Talk(eTalks::TalkSlay);
		}

		void EnterEvadeMode() override
		{
			if (m_Evading)
				return;

			m_Evading = true;
			BossAI::EnterEvadeMode();
			CouncilEvadeAll(m_Instance, eCitadelCreatures::BossJubeithos);
			m_Evading = false;
		}

		void JustDied(Unit* /*p_Killer*/) override
		{
			Talk(eTalks::TalkDeath);
			events.Reset();
			summons.DespawnAll();

			if (CouncilAllDead(m_Instance))
			{
				if (m_Instance)
					m_Instance->SetBossState(eCitadelDatas::DataCouncil, EncounterState::DONE);
			}
		}

		void JustSummoned(Creature* p_Summon) override
		{
			summons.Summon(p_Summon);
			p_Summon->SetInCombatWithZone();
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
					case eCouncilEvents::EventFelstormBlade:
					{
						Talk(eTalks::TalkMirror);
						me->CastSpell(me, eCouncilSpells::SpellFelstormBlade, false);
						events.ScheduleEvent(eCouncilEvents::EventFelstormBlade, eCouncilTimers::TimerFelstormBlade);
						break;
					}
					case eCouncilEvents::EventMirrorImage:
					{
						Talk(eTalks::TalkMirror);
						me->CastSpell(me, eCouncilSpells::SpellMirrorImage, false);

						/// Spawn images near Jubei'thos that fight the raid
						for (uint8 l_I = 0; l_I < 3; ++l_I)
						{
							float l_Angle = (float(l_I) / 3.0f) * 2.0f * M_PI;
							float l_X     = me->GetPositionX() + 5.0f * std::cos(l_Angle);
							float l_Y     = me->GetPositionY() + 5.0f * std::sin(l_Angle);
							float l_Z     = me->GetPositionZ();

							if (Creature* l_Image = me->SummonCreature(eCouncilCreatures::NpcJubeithosImage, l_X, l_Y, l_Z, me->GetOrientation(), TEMPSUMMON_TIMED_DESPAWN, 20000))
								l_Image->SetInCombatWithZone();
						}

						events.ScheduleEvent(eCouncilEvents::EventMirrorImage, eCouncilTimers::TimerMirrorImage);
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
		return new boss_jubeithosAI(p_Creature);
	}
};

/// ---------------------------------------------------------------------------
/// Jubei'thos Mirror Image NPC - 92143
/// ---------------------------------------------------------------------------
class npc_jubeithos_mirror_image : public CreatureScript
{
public:
	npc_jubeithos_mirror_image() : CreatureScript("npc_jubeithos_mirror_image") { }

	struct npc_jubeithos_mirror_imageAI : public ScriptedAI
	{
		npc_jubeithos_mirror_imageAI(Creature* p_Creature)
			: ScriptedAI(p_Creature) { }

		void Reset() override
		{
			me->SetReactState(REACT_AGGRESSIVE);
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
		return new npc_jubeithos_mirror_imageAI(p_Creature);
	}
};

/// ---------------------------------------------------------------------------
void AddSC_boss_high_council_hfc()
{
	/// Bosses
	new boss_dia_darkwhisper();
	new boss_gurtogg_bloodboil();
	new boss_jubeithos();

	/// Creatures
	new npc_jubeithos_mirror_image();
}
