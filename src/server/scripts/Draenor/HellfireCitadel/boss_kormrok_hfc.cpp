///////////////////////////////////////////////////////////////////////////////
///
///  MILLENIUM-STUDIO
///  Copyright 2016 Millenium-studio SARL
///  All Rights Reserved.
///
///////////////////////////////////////////////////////////////////////////////

#include "boss_kormrok_hfc.h"
#include "hellfire_citadel.h"

/// Helper: the three pools as a flat array for distance comparisons
static Position const g_KormrokPools[3] =
{
	g_KormrokFieryPoolPos,   ///< index 0 → PoolFiery
	g_KormrokFoulPoolPos,    ///< index 1 → PoolFoul
	g_KormrokShadowPoolPos,  ///< index 2 → PoolShadow
};

/// Map from flat index → eKormrokPools enum value
static eKormrokPools const g_KormrokPoolTypes[3] =
{
	eKormrokPools::PoolFiery,
	eKormrokPools::PoolFoul,
	eKormrokPools::PoolShadow,
};

//////////////////////////////////////////////////////////////////////////////
///  Kormrok — 90776
///
///  Encounter overview:
///    Kormrok alternates between melee combat and leaping to one of three
///    pools of power around the room every ~100 s.  Each pool empowers his
///    next 2 (Normal) or 3 (Heroic) casts of the associated ability:
///
///      Fiery  Pool → Empowered Pound
///      Foul   Pool → Empowered Fel Outpouring
///      Shadow Pool → Empowered Explosive Runes + Empowered Grasping Hands
///
///    Kormrok always leaps to the nearest pool, naturally cycling through
///    all three over successive leaps as his position shifts after each one.
//////////////////////////////////////////////////////////////////////////////
class boss_kormrok_hfc : public CreatureScript
{
public:
	boss_kormrok_hfc() : CreatureScript("boss_kormrok_hfc") {}

	enum eTalks
	{
		TalkAggro   = 0,
		TalkLeap    = 1,
		TalkSlay    = 2,
		TalkWipe    = 3,
		TalkDeath   = 4,
	};

	struct boss_kormrok_hfcAI : public BossAI
	{
		boss_kormrok_hfcAI(Creature* p_Creature) : BossAI(p_Creature, eCitadelDatas::DataKormrok)
		{
			m_Instance          = p_Creature->GetInstanceScript();
			m_CurrentPool       = eKormrokPools::PoolNone;
			m_EmpoweredCastsLeft = 0;
			m_IsLeaping         = false;
		}

		InstanceScript* m_Instance;
		EventMap        m_Events;

		uint8           m_CurrentPool;        ///< eKormrokPools — active empowerment type
		uint8           m_EmpoweredCastsLeft; ///< How many empowered casts remain
		bool            m_IsLeaping;          ///< True while performing a pool leap

		/// ----- Reset / State -----------------------------------------------

		void Reset() override
		{
			m_Events.Reset();
			ClearDelayedOperations();
			summons.DespawnAll();
			me->SetReactState(ReactStates::REACT_AGGRESSIVE);

			m_CurrentPool        = eKormrokPools::PoolNone;
			m_EmpoweredCastsLeft = 0;
			m_IsLeaping          = false;

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
				m_Instance->SetBossState(eCitadelDatas::DataKormrok, EncounterState::FAIL);
			}
		}

		void EnterCombat(Unit* /*p_Attacker*/) override
		{
			_EnterCombat();
			Talk(eTalks::TalkAggro);

			m_Events.ScheduleEvent(eKormrokEvents::EventPound,         eKormrokTimers::TimerPound);
			m_Events.ScheduleEvent(eKormrokEvents::EventExplosiveRunes, eKormrokTimers::TimerExplosiveRunes);
			m_Events.ScheduleEvent(eKormrokEvents::EventGraspingHands,  eKormrokTimers::TimerGraspingHands);
			m_Events.ScheduleEvent(eKormrokEvents::EventFelOutpouring,  eKormrokTimers::TimerFelOutpouring);
			m_Events.ScheduleEvent(eKormrokEvents::EventLeapToPool,     eKormrokTimers::TimerLeapToPool);

			if (m_Instance != nullptr)
			{
				m_Instance->SendEncounterUnit(EncounterFrameType::ENCOUNTER_FRAME_ENGAGE, me, 1);
				m_Instance->SetBossState(eCitadelDatas::DataKormrok, EncounterState::IN_PROGRESS);
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
				m_Instance->SetBossState(eCitadelDatas::DataKormrok, EncounterState::DONE);

				if (sObjectMgr->IsDisabledEncounter(m_Instance->GetEncounterIDForBoss(me), GetDifficulty()))
					me->SetLootRecipient(nullptr);
				else
					CastSpellToPlayers(me->GetMap(), me, eKormrokSpells::SpellKormrokLoot, true);
			}
		}

		/// ----- Pool leap ---------------------------------------------------

		/// Returns the index (0-2) of the pool closest to Kormrok's current position
		uint8 FindNearestPoolIndex() const
		{
			float l_BestDist = 9999.0f;
			uint8 l_BestIdx  = 0;

			for (uint8 i = 0; i < 3; ++i)
			{
				float l_Dist = me->GetDistance(
					g_KormrokPools[i].m_positionX,
					g_KormrokPools[i].m_positionY,
					g_KormrokPools[i].m_positionZ);

				if (l_Dist < l_BestDist)
				{
					l_BestDist = l_Dist;
					l_BestIdx  = i;
				}
			}
			return l_BestIdx;
		}

		void InitiatePoolLeap()
		{
			m_IsLeaping = true;
			me->SetReactState(ReactStates::REACT_PASSIVE);
			me->AttackStop();
			Talk(eTalks::TalkLeap);

			/// Cancel ability events; they will be rescheduled after landing
			m_Events.CancelEvent(eKormrokEvents::EventPound);
			m_Events.CancelEvent(eKormrokEvents::EventExplosiveRunes);
			m_Events.CancelEvent(eKormrokEvents::EventGraspingHands);
			m_Events.CancelEvent(eKormrokEvents::EventFelOutpouring);
			m_Events.CancelEvent(eKormrokEvents::EventLeapToPool);

			uint8 l_PoolIdx    = FindNearestPoolIndex();
			m_CurrentPool      = (uint8)g_KormrokPoolTypes[l_PoolIdx];

			me->GetMotionMaster()->MoveJump(
				g_KormrokPools[l_PoolIdx].m_positionX,
				g_KormrokPools[l_PoolIdx].m_positionY,
				g_KormrokPools[l_PoolIdx].m_positionZ,
				20.0f, 20.0f, 0.0f,
				eKormrokPoints::PointPoolLeap);
		}

		void OnPoolLanded()
		{
			/// Empowered cast count: 2 on Normal/LFR, 3 on Heroic/Mythic
			m_EmpoweredCastsLeft = me->GetMap()->IsHeroic() ? 3 : 2;

			/// Brief pause in the pool before re-engaging
			AddTimedDelayedOperation(eKormrokTimers::TimerResumeAfterLeap, [this]() -> void
			{
				m_IsLeaping = false;
				me->SetReactState(ReactStates::REACT_AGGRESSIVE);

				if (Unit* l_Victim = me->GetVictim())
					me->GetMotionMaster()->MoveChase(l_Victim);
				else
					me->SetInCombatWithZone();

				/// Reschedule all abilities with post-leap cooldowns
				m_Events.ScheduleEvent(eKormrokEvents::EventPound,         eKormrokTimers::TimerPoundCD);
				m_Events.ScheduleEvent(eKormrokEvents::EventExplosiveRunes, eKormrokTimers::TimerExplosiveRunesCD);
				m_Events.ScheduleEvent(eKormrokEvents::EventGraspingHands,  eKormrokTimers::TimerGraspingHandsCD);
				m_Events.ScheduleEvent(eKormrokEvents::EventFelOutpouring,  eKormrokTimers::TimerFelOutpouringCD);
				m_Events.ScheduleEvent(eKormrokEvents::EventLeapToPool,     eKormrokTimers::TimerLeapToPool);
			});
		}

		void MovementInform(uint32 p_Type, uint32 p_ID) override
		{
			if (p_Type != MovementGeneratorType::POINT_MOTION_TYPE &&
				p_Type != MovementGeneratorType::EFFECT_MOTION_TYPE)
				return;

			if (p_ID == eKormrokPoints::PointPoolLeap)
				OnPoolLanded();
		}

		/// ----- Ability cast helpers ----------------------------------------

		/// Returns true if this pool type should use an empowered cast and decrements counter
		bool ConsumeEmpower(eKormrokPools p_RequiredPool)
		{
			if (m_EmpoweredCastsLeft > 0 && m_CurrentPool == (uint8)p_RequiredPool)
			{
				--m_EmpoweredCastsLeft;
				if (m_EmpoweredCastsLeft == 0)
					m_CurrentPool = (uint8)eKormrokPools::PoolNone;
				return true;
			}
			return false;
		}

		/// ----- AI Update ---------------------------------------------------

		void UpdateAI(uint32 const p_Diff) override
		{
			UpdateOperations(p_Diff);
			m_Events.Update(p_Diff);

			/// Suspend normal combat logic while airborne
			if (m_IsLeaping)
				return;

			if (!UpdateVictim())
				return;

			if (me->HasUnitState(UnitState::UNIT_STATE_CASTING))
				return;

			switch (m_Events.ExecuteEvent())
			{
				/// ---- Pound -----------------------------------------------
				case eKormrokEvents::EventPound:
				{
					bool l_Empowered = ConsumeEmpower(eKormrokPools::PoolFiery);
					uint32 l_Spell   = l_Empowered
						? (uint32)eKormrokSpells::SpellEmpoweredPound
						: (uint32)eKormrokSpells::SpellPound;

					if (Unit* l_Target = SelectTarget(SELECT_TARGET_TOPAGGRO, 0, 0.0f, true))
						me->CastSpell(l_Target, l_Spell, false);

					m_Events.ScheduleEvent(eKormrokEvents::EventPound,
						eKormrokTimers::TimerPoundCD);
					break;
				}
				/// ---- Explosive Runes -------------------------------------
				case eKormrokEvents::EventExplosiveRunes:
				{
					bool l_Empowered = ConsumeEmpower(eKormrokPools::PoolShadow);
					uint32 l_Spell   = l_Empowered
						? (uint32)eKormrokSpells::SpellEmpoweredExplosiveRunes
						: (uint32)eKormrokSpells::SpellExplosiveRunes;

					me->CastSpell(me, l_Spell, false);

					m_Events.ScheduleEvent(eKormrokEvents::EventExplosiveRunes,
						eKormrokTimers::TimerExplosiveRunesCD);
					break;
				}
				/// ---- Grasping Hands --------------------------------------
				case eKormrokEvents::EventGraspingHands:
				{
					bool l_Empowered = ConsumeEmpower(eKormrokPools::PoolShadow);
					uint32 l_Spell   = l_Empowered
						? (uint32)eKormrokSpells::SpellEmpoweredGraspingHands
						: (uint32)eKormrokSpells::SpellGraspingHands;

					me->CastSpell(me, l_Spell, false);

					m_Events.ScheduleEvent(eKormrokEvents::EventGraspingHands,
						eKormrokTimers::TimerGraspingHandsCD);
					break;
				}
				/// ---- Fel Outpouring --------------------------------------
				case eKormrokEvents::EventFelOutpouring:
				{
					bool l_Empowered = ConsumeEmpower(eKormrokPools::PoolFoul);
					uint32 l_Spell   = l_Empowered
						? (uint32)eKormrokSpells::SpellEmpoweredFelOutpouring
						: (uint32)eKormrokSpells::SpellFelOutpouring;

					me->CastSpell(me, l_Spell, false);

					m_Events.ScheduleEvent(eKormrokEvents::EventFelOutpouring,
						eKormrokTimers::TimerFelOutpouringCD);
					break;
				}
				/// ---- Leap to pool ----------------------------------------
				case eKormrokEvents::EventLeapToPool:
				{
					InitiatePoolLeap();
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
		return new boss_kormrok_hfcAI(p_Creature);
	}
};

#ifndef __clang_analyzer__
void AddSC_boss_kormrok_hfc()
{
	/// Boss
	new boss_kormrok_hfc();
}
#endif
