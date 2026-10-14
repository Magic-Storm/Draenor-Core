///////////////////////////////////////////////////////////////////////////////
///
///  MILLENIUM-STUDIO
///  Copyright 2016 Millenium-studio SARL
///  All Rights Reserved.
///
///////////////////////////////////////////////////////////////////////////////

#include "boss_zakuun_hfc.h"
#include "hellfire_citadel.h"

/// Fel Lord Zakuun - 89890
class boss_zakuun_hfc : public CreatureScript
{
public:
    boss_zakuun_hfc() : CreatureScript("boss_zakuun_hfc") { }

    enum eTalks
    {
        TalkAggro   = 0,
        TalkSlay    = 1,
        TalkDeath   = 2,
        TalkPhase   = 3,   ///< Armed -> Disarmed transition
        TalkEnrage  = 4,
        TalkWipe    = 5,
    };

    struct boss_zakuun_hfcAI : public BossAI
    {
        boss_zakuun_hfcAI(Creature* p_Creature) : BossAI(p_Creature, eCitadelDatas::DataZakuun)
        {
            m_Instance  = p_Creature->GetInstanceScript();
            m_Phase     = eZakuunPhases::PhaseArmed;
            m_PhaseSwitchCount = 0;
        }

        InstanceScript* m_Instance;
        uint8           m_Phase;
        uint8           m_PhaseSwitchCount;

        void Reset() override
        {
            _Reset();

            m_Phase             = eZakuunPhases::PhaseArmed;
            m_PhaseSwitchCount  = 0;

            me->SetReactState(ReactStates::REACT_AGGRESSIVE);
            me->RemoveAura(eZakuunSpells::SpellArmedPhase);
            me->RemoveAura(eZakuunSpells::SpellDisarmedPhase);
            me->RemoveAura(eZakuunSpells::SpellEnrage);

            if (m_Instance != nullptr)
                m_Instance->SendEncounterUnit(EncounterFrameType::ENCOUNTER_FRAME_DISENGAGE, me);
        }

        void EnterCombat(Unit* /*p_Who*/) override
        {
            _EnterCombat();

            Talk(eTalks::TalkAggro);

            me->AddAura(eZakuunSpells::SpellArmedPhase, me);

            if (m_Instance != nullptr)
            {
                m_Instance->SendEncounterUnit(EncounterFrameType::ENCOUNTER_FRAME_ENGAGE, me, 1);
                m_Instance->SetBossState(eCitadelDatas::DataZakuun, EncounterState::IN_PROGRESS);
            }

            ScheduleArmedPhaseEvents();
            events.ScheduleEvent(eZakuunEvents::EventPhaseSwitch, eZakuunTimers::TimerPhaseSwitch);
        }

        void JustReachedHome() override
        {
            _Reset();
            Talk(eTalks::TalkWipe);

            if (m_Instance != nullptr)
            {
                m_Instance->SendEncounterUnit(EncounterFrameType::ENCOUNTER_FRAME_DISENGAGE, me);
                m_Instance->SetBossState(eCitadelDatas::DataZakuun, EncounterState::FAIL);
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

            /// Clean up Shadow Realm auras
            Map::PlayerList const& l_Players = me->GetMap()->GetPlayers();
            for (Map::PlayerList::const_iterator l_Iter = l_Players.begin(); l_Iter != l_Players.end(); ++l_Iter)
            {
                if (Player* l_Player = l_Iter->getSource())
                {
                    l_Player->RemoveAura(eZakuunSpells::SpellSoulCleaveDebuff);
                    l_Player->RemoveAura(eZakuunSpells::SpellShadowRealm);
                }
            }

            if (m_Instance != nullptr)
            {
                m_Instance->SendEncounterUnit(EncounterFrameType::ENCOUNTER_FRAME_DISENGAGE, me);
                m_Instance->SetBossState(eCitadelDatas::DataZakuun, EncounterState::DONE);

                if (sObjectMgr->IsDisabledEncounter(m_Instance->GetEncounterIDForBoss(me), GetDifficulty()))
                    me->SetLootRecipient(nullptr);
                else
                    CastSpellToPlayers(me->GetMap(), me, eZakuunSpells::SpellZakuunBonusLoot, true);
            }
        }

        void DamageTaken(Unit* /*p_Attacker*/, uint32& p_Damage, SpellInfo const* /*p_SpellInfo*/) override
        {
            if (m_Phase != eZakuunPhases::PhaseEnrage && me->HealthBelowPctDamaged(30, p_Damage))
            {
                DoAction(eZakuunActions::ActionEnterEnrage);
            }
        }

        void DoAction(int32 const p_Action) override
        {
            switch (p_Action)
            {
                case eZakuunActions::ActionSwitchPhase:
                    SwitchPhase();
                    break;
                case eZakuunActions::ActionEnterEnrage:
                    EnterEnragePhase();
                    break;
                default:
                    break;
            }
        }

        void SwitchPhase()
        {
            if (m_Phase == eZakuunPhases::PhaseEnrage)
                return;

            Talk(eTalks::TalkPhase);

            events.CancelEvent(eZakuunEvents::EventSoulCleave);
            events.CancelEvent(eZakuunEvents::EventWakeOfDestruction);
            events.CancelEvent(eZakuunEvents::EventRumblingFissure);
            events.CancelEvent(eZakuunEvents::EventSeedOfDestruction);
            events.CancelEvent(eZakuunEvents::EventCavitation);
            events.CancelEvent(eZakuunEvents::EventLatentEnergy);
            events.CancelEvent(eZakuunEvents::EventSummonFelCrystal);

            ++m_PhaseSwitchCount;

            if (m_Phase == eZakuunPhases::PhaseArmed)
            {
                m_Phase = eZakuunPhases::PhaseDisarmed;
                me->RemoveAura(eZakuunSpells::SpellArmedPhase);
                me->AddAura(eZakuunSpells::SpellDisarmedPhase, me);

                /// Throw axe - trigger Cavitation at a random player
                if (Unit* l_Target = SelectTarget(SELECT_TARGET_RANDOM, 0, 60.0f, true))
                    me->CastSpell(l_Target, eZakuunSpells::SpellCavitation, false);

                ScheduleDisarmedPhaseEvents();
            }
            else
            {
                m_Phase = eZakuunPhases::PhaseArmed;
                me->RemoveAura(eZakuunSpells::SpellDisarmedPhase);
                me->AddAura(eZakuunSpells::SpellArmedPhase, me);

                ScheduleArmedPhaseEvents();
            }

            events.ScheduleEvent(eZakuunEvents::EventPhaseSwitch, eZakuunTimers::TimerPhaseSwitch);
        }

        void EnterEnragePhase()
        {
            m_Phase = eZakuunPhases::PhaseEnrage;

            Talk(eTalks::TalkEnrage);

            me->RemoveAura(eZakuunSpells::SpellArmedPhase);
            me->RemoveAura(eZakuunSpells::SpellDisarmedPhase);
            me->CastSpell(me, eZakuunSpells::SpellEnrage, true);

            events.CancelEvent(eZakuunEvents::EventPhaseSwitch);

            /// In enrage both phase abilities fire
            ScheduleArmedPhaseEvents();
            ScheduleDisarmedPhaseEvents();

            events.ScheduleEvent(eZakuunEvents::EventFelOutburst, eZakuunTimers::TimerFelOutburst);
        }

        void ScheduleArmedPhaseEvents()
        {
            events.ScheduleEvent(eZakuunEvents::EventSoulCleave,         eZakuunTimers::TimerSoulCleave);
            events.ScheduleEvent(eZakuunEvents::EventWakeOfDestruction,   eZakuunTimers::TimerWakeOfDestruction);
            events.ScheduleEvent(eZakuunEvents::EventRumblingFissure,     eZakuunTimers::TimerRumblingFissure);
            events.ScheduleEvent(eZakuunEvents::EventSeedOfDestruction,   eZakuunTimers::TimerSeedOfDestruction);
        }

        void ScheduleDisarmedPhaseEvents()
        {
            events.ScheduleEvent(eZakuunEvents::EventCavitation,          eZakuunTimers::TimerCavitation);
            events.ScheduleEvent(eZakuunEvents::EventLatentEnergy,         eZakuunTimers::TimerLatentEnergy);
            events.ScheduleEvent(eZakuunEvents::EventSummonFelCrystal,    eZakuunTimers::TimerSummonFelCrystal);
        }

        void UpdateAI(uint32 const p_Diff) override
        {
            if (!UpdateVictim())
                return;

            if (me->HasUnitState(UnitState::UNIT_STATE_CASTING))
                return;

            events.Update(p_Diff);

            while (uint32 l_EventId = events.ExecuteEvent())
            {
                switch (l_EventId)
                {
                    case eZakuunEvents::EventSoulCleave:
                    {
                        /// Hit current tank with Soul Cleave - applies Shadow Realm debuff
                        if (Unit* l_Tank = me->GetVictim())
                        {
                            me->CastSpell(l_Tank, eZakuunSpells::SpellSoulCleave, false);

                            /// Summon Residual Energy add at the tank's position (persists in Shadow Realm)
                            me->SummonCreature(eZakuunCreatures::NpcResidualEnergy,
                                l_Tank->GetPositionX(), l_Tank->GetPositionY(), l_Tank->GetPositionZ(),
                                l_Tank->GetOrientation(), TEMPSUMMON_CORPSE_DESPAWN, 0);
                        }
                        events.ScheduleEvent(eZakuunEvents::EventSoulCleave, eZakuunTimers::TimerSoulCleave);
                        break;
                    }
                    case eZakuunEvents::EventWakeOfDestruction:
                    {
                        me->CastSpell(me, eZakuunSpells::SpellWakeOfDestruction, false);
                        events.ScheduleEvent(eZakuunEvents::EventWakeOfDestruction, eZakuunTimers::TimerWakeOfDestruction);
                        break;
                    }
                    case eZakuunEvents::EventRumblingFissure:
                    {
                        /// Cast at a random player location
                        if (Unit* l_Target = SelectTarget(SELECT_TARGET_RANDOM, 1, 60.0f, true))
                            me->CastSpell(l_Target, eZakuunSpells::SpellRumblingFissure, false);
                        events.ScheduleEvent(eZakuunEvents::EventRumblingFissure, eZakuunTimers::TimerRumblingFissure);
                        break;
                    }
                    case eZakuunEvents::EventSeedOfDestruction:
                    {
                        if (Unit* l_Target = SelectTarget(SELECT_TARGET_RANDOM, 1, 60.0f, true))
                            me->CastSpell(l_Target, eZakuunSpells::SpellSeedOfDestruction, false);
                        events.ScheduleEvent(eZakuunEvents::EventSeedOfDestruction, eZakuunTimers::TimerSeedOfDestruction);
                        break;
                    }
                    case eZakuunEvents::EventCavitation:
                    {
                        /// Shockwave from axe impact position
                        me->CastSpell(me, eZakuunSpells::SpellCavitation, false);
                        events.ScheduleEvent(eZakuunEvents::EventCavitation, eZakuunTimers::TimerCavitation);
                        break;
                    }
                    case eZakuunEvents::EventLatentEnergy:
                    {
                        me->CastSpell(me, eZakuunSpells::SpellLatentEnergy, false);
                        events.ScheduleEvent(eZakuunEvents::EventLatentEnergy, eZakuunTimers::TimerLatentEnergy);
                        break;
                    }
                    case eZakuunEvents::EventSummonFelCrystal:
                    {
                        Position l_SpawnPos;
                        me->GetNearPosition(l_SpawnPos, 15.0f, frand(0.0f, 2.0f * M_PI));
                        if (Creature* l_Crystal = me->SummonCreature(eZakuunCreatures::NpcFelCrystal,
                            l_SpawnPos, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 30000))
                        {
                            l_Crystal->CastSpell(me, eZakuunSpells::SpellFelCrystalBeam, true);
                        }
                        events.ScheduleEvent(eZakuunEvents::EventSummonFelCrystal, eZakuunTimers::TimerSummonFelCrystal);
                        break;
                    }
                    case eZakuunEvents::EventPhaseSwitch:
                    {
                        SwitchPhase();
                        break;
                    }
                    case eZakuunEvents::EventFelOutburst:
                    {
                        me->CastSpell(me, eZakuunSpells::SpellFelOutburst, false);
                        events.ScheduleEvent(eZakuunEvents::EventFelOutburst, eZakuunTimers::TimerFelOutburst);
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
        return new boss_zakuun_hfcAI(p_Creature);
    }
};

/// Residual Energy - 90294
/// Shadow Realm add summoned by Soul Cleave
class npc_zakuun_residual_energy : public CreatureScript
{
public:
    npc_zakuun_residual_energy() : CreatureScript("npc_zakuun_residual_energy") { }

    struct npc_zakuun_residual_energyAI : public ScriptedAI
    {
        npc_zakuun_residual_energyAI(Creature* p_Creature) : ScriptedAI(p_Creature)
        {
            m_PulseTimer = 0;
        }

        uint32 m_PulseTimer;

        void Reset() override
        {
            me->SetReactState(ReactStates::REACT_PASSIVE);
            me->SetInCombatWithZone();
            me->AddAura(eZakuunSpells::SpellResidualEnergyAura, me);
            m_PulseTimer = 3000;
        }

        void UpdateAI(uint32 const p_Diff) override
        {
            if (m_PulseTimer <= p_Diff)
            {
                me->CastSpell(me, eZakuunSpells::SpellResidualEnergyAura, true);
                m_PulseTimer = 3000;
            }
            else
                m_PulseTimer -= p_Diff;
        }
    };

    CreatureAI* GetAI(Creature* p_Creature) const override
    {
        return new npc_zakuun_residual_energyAI(p_Creature);
    }
};

/// Fel Crystal - 90305
/// Summoned during Disarmed phase; channels a beam to Zakuun
class npc_zakuun_fel_crystal : public CreatureScript
{
public:
    npc_zakuun_fel_crystal() : CreatureScript("npc_zakuun_fel_crystal") { }

    struct npc_zakuun_fel_crystalAI : public ScriptedAI
    {
        npc_zakuun_fel_crystalAI(Creature* p_Creature) : ScriptedAI(p_Creature) { }

        void Reset() override
        {
            me->SetReactState(ReactStates::REACT_PASSIVE);
            me->SetFlag(EUnitFields::UNIT_FIELD_FLAGS, UnitFlags::UNIT_FLAG_NON_ATTACKABLE | UnitFlags::UNIT_FLAG_NOT_SELECTABLE);
        }

        void UpdateAI(uint32 const /*p_Diff*/) override { }
    };

    CreatureAI* GetAI(Creature* p_Creature) const override
    {
        return new npc_zakuun_fel_crystalAI(p_Creature);
    }
};

/// Spell: Soul Cleave - 179406
/// Marks the target with Shadow Realm debuff
class spell_zakuun_soul_cleave : public SpellScript
{
    PrepareSpellScript(spell_zakuun_soul_cleave);

    void HandleAfterHit()
    {
        if (Unit* l_Target = GetHitUnit())
        {
            /// Apply the lingering Shadow Realm mark
            l_Target->CastSpell(l_Target, eZakuunSpells::SpellSoulCleaveDebuff, true);
        }
    }

    void Register() override
    {
        AfterHit += SpellHitFn(spell_zakuun_soul_cleave::HandleAfterHit);
    }
};

/// Spell: Seed of Destruction - 181508
/// AuraScript: explode when removed / on expire
class spell_zakuun_seed_of_destruction : public AuraScript
{
    PrepareAuraScript(spell_zakuun_seed_of_destruction);

    void HandleRemove(AuraEffect const* /*p_AurEff*/, AuraEffectHandleModes /*p_Mode*/)
    {
        if (Unit* l_Target = GetTarget())
            l_Target->CastSpell(l_Target, eZakuunSpells::SpellSeedExplosion, true);
    }

    void Register() override
    {
        OnEffectRemove += AuraEffectRemoveFn(spell_zakuun_seed_of_destruction::HandleRemove,
            EFFECT_0, SPELL_AURA_PERIODIC_DUMMY, AURA_EFFECT_HANDLE_REAL);
    }
};

#ifndef __clang_analyzer__
void AddSC_boss_zakuun_hfc()
{
    /// Boss
    new boss_zakuun_hfc();

    /// Creatures
    new npc_zakuun_residual_energy();
    new npc_zakuun_fel_crystal();

    /// Spells
    new spell_zakuun_soul_cleave();
    new spell_zakuun_seed_of_destruction();
}
#endif
