///////////////////////////////////////////////////////////////////////////////
///
///  MILLENIUM-STUDIO
///  Copyright 2016 Millenium-studio SARL
///  All Rights Reserved.
///
///////////////////////////////////////////////////////////////////////////////

#include "boss_velhari_hfc.h"
#include "hellfire_citadel.h"

/// Tyrant Velhari - 93439
class boss_velhari_hfc : public CreatureScript
{
public:
    boss_velhari_hfc() : CreatureScript("boss_velhari_hfc") { }

    enum eTalks
    {
        TalkAggro       = 0,
        TalkSlay        = 1,
        TalkDeath       = 2,
        TalkPhase2      = 3,
        TalkPhase3      = 4,
        TalkWipe        = 5,
    };

    struct boss_velhari_hfcAI : public BossAI
    {
        boss_velhari_hfcAI(Creature* p_Creature) : BossAI(p_Creature, eCitadelDatas::DataVelhari)
        {
            m_Instance  = p_Creature->GetInstanceScript();
            m_Phase     = eVelhariPhases::PhaseContempt;
        }

        InstanceScript* m_Instance;
        uint8           m_Phase;

        void Reset() override
        {
            _Reset();

            m_Phase = eVelhariPhases::PhaseContempt;

            me->SetReactState(ReactStates::REACT_AGGRESSIVE);

            /// Remove all phase auras
            me->RemoveAura(eVelhariSpells::SpellAuraOfContempt);
            me->RemoveAura(eVelhariSpells::SpellAuraOfMalice);
            me->RemoveAura(eVelhariSpells::SpellAuraOfOppression);
            me->RemoveAura(eVelhariSpells::SpellEmpoweredAura);

            /// Clean players
            Map::PlayerList const& l_Players = me->GetMap()->GetPlayers();
            for (Map::PlayerList::const_iterator l_Iter = l_Players.begin(); l_Iter != l_Players.end(); ++l_Iter)
            {
                if (Player* l_Player = l_Iter->getSource())
                {
                    l_Player->RemoveAura(eVelhariSpells::SpellSealOfDecay);
                    l_Player->RemoveAura(eVelhariSpells::SpellAncientCorruption);
                    l_Player->RemoveAura(eVelhariSpells::SpellEdictOfCondemnation);
                }
            }

            if (m_Instance != nullptr)
                m_Instance->SendEncounterUnit(EncounterFrameType::ENCOUNTER_FRAME_DISENGAGE, me);
        }

        void EnterCombat(Unit* /*p_Who*/) override
        {
            _EnterCombat();

            Talk(eTalks::TalkAggro);

            m_Phase = eVelhariPhases::PhaseContempt;
            me->AddAura(eVelhariSpells::SpellAuraOfContempt, me);

            if (m_Instance != nullptr)
            {
                m_Instance->SendEncounterUnit(EncounterFrameType::ENCOUNTER_FRAME_ENGAGE, me, 1);
                m_Instance->SetBossState(eCitadelDatas::DataVelhari, EncounterState::IN_PROGRESS);
            }

            SchedulePhase1Events();

            /// Ancient Corruption builds throughout entire fight
            events.ScheduleEvent(eVelhariEvents::EventAncientCorruption, eVelhariTimers::TimerAncientCorruption);
        }

        void JustReachedHome() override
        {
            _Reset();
            Talk(eTalks::TalkWipe);
            summons.DespawnAll();

            if (m_Instance != nullptr)
            {
                m_Instance->SendEncounterUnit(EncounterFrameType::ENCOUNTER_FRAME_DISENGAGE, me);
                m_Instance->SetBossState(eCitadelDatas::DataVelhari, EncounterState::FAIL);
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

            /// Remove fight auras
            Map::PlayerList const& l_Players = me->GetMap()->GetPlayers();
            for (Map::PlayerList::const_iterator l_Iter = l_Players.begin(); l_Iter != l_Players.end(); ++l_Iter)
            {
                if (Player* l_Player = l_Iter->getSource())
                {
                    l_Player->RemoveAura(eVelhariSpells::SpellSealOfDecay);
                    l_Player->RemoveAura(eVelhariSpells::SpellAncientCorruption);
                    l_Player->RemoveAura(eVelhariSpells::SpellEdictOfCondemnation);
                    l_Player->RemoveAura(eVelhariSpells::SpellAnimaFont);
                }
            }

            if (m_Instance != nullptr)
            {
                m_Instance->SendEncounterUnit(EncounterFrameType::ENCOUNTER_FRAME_DISENGAGE, me);
                m_Instance->SetBossState(eCitadelDatas::DataVelhari, EncounterState::DONE);

                if (sObjectMgr->IsDisabledEncounter(m_Instance->GetEncounterIDForBoss(me), GetDifficulty()))
                    me->SetLootRecipient(nullptr);
                else
                    CastSpellToPlayers(me->GetMap(), me, eVelhariSpells::SpellVelhariBonusLoot, true);
            }
        }

        void DamageTaken(Unit* /*p_Attacker*/, uint32& p_Damage, SpellInfo const* /*p_SpellInfo*/) override
        {
            if (m_Phase == eVelhariPhases::PhaseContempt && me->HealthBelowPctDamaged(70, p_Damage))
                DoAction(eVelhariActions::ActionPhaseTwo);
            else if (m_Phase == eVelhariPhases::PhaseMalice && me->HealthBelowPctDamaged(40, p_Damage))
                DoAction(eVelhariActions::ActionPhaseThree);
        }

        void DoAction(int32 const p_Action) override
        {
            switch (p_Action)
            {
                case eVelhariActions::ActionPhaseTwo:
                    TransitionToPhase2();
                    break;
                case eVelhariActions::ActionPhaseThree:
                    TransitionToPhase3();
                    break;
                default:
                    break;
            }
        }

        void TransitionToPhase2()
        {
            if (m_Phase != eVelhariPhases::PhaseContempt)
                return;

            m_Phase = eVelhariPhases::PhaseMalice;

            Talk(eTalks::TalkPhase2);

            me->RemoveAura(eVelhariSpells::SpellAuraOfContempt);
            me->AddAura(eVelhariSpells::SpellAuraOfMalice, me);
            me->AddAura(eVelhariSpells::SpellEmpoweredAura, me);

            /// Cancel phase 1 specific events
            events.CancelEvent(eVelhariEvents::EventSealOfDecay);
            events.CancelEvent(eVelhariEvents::EventEdictOfCondemnation);
            events.CancelEvent(eVelhariEvents::EventGavelOfTheTyrant);

            SchedulePhase2Events();
        }

        void TransitionToPhase3()
        {
            if (m_Phase != eVelhariPhases::PhaseMalice)
                return;

            m_Phase = eVelhariPhases::PhaseOppression;

            Talk(eTalks::TalkPhase3);

            me->RemoveAura(eVelhariSpells::SpellAuraOfMalice);
            me->AddAura(eVelhariSpells::SpellAuraOfOppression, me);

            /// Cancel phase 2 specific events
            events.CancelEvent(eVelhariEvents::EventAnimaFont);
            events.CancelEvent(eVelhariEvents::EventAnnihilationStrike);

            SchedulePhase3Events();
        }

        void SchedulePhase1Events()
        {
            events.ScheduleEvent(eVelhariEvents::EventSealOfDecay,          eVelhariTimers::TimerSealOfDecay);
            events.ScheduleEvent(eVelhariEvents::EventEdictOfCondemnation,  eVelhariTimers::TimerEdictOfCondemnation);
            events.ScheduleEvent(eVelhariEvents::EventGavelOfTheTyrant,     eVelhariTimers::TimerGavelOfTheTyrant);
        }

        void SchedulePhase2Events()
        {
            events.ScheduleEvent(eVelhariEvents::EventAnimaFont,            eVelhariTimers::TimerAnimaFont);
            events.ScheduleEvent(eVelhariEvents::EventAnnihilationStrike,   eVelhariTimers::TimerAnnihilationStrike);
            /// Keep Seal of Decay and Edict active in phase 2 as well
            events.ScheduleEvent(eVelhariEvents::EventSealOfDecay,          eVelhariTimers::TimerSealOfDecay);
            events.ScheduleEvent(eVelhariEvents::EventEdictOfCondemnation,  eVelhariTimers::TimerEdictOfCondemnation);
        }

        void SchedulePhase3Events()
        {
            events.ScheduleEvent(eVelhariEvents::EventFinalVerdict,         eVelhariTimers::TimerFinalVerdict);
            events.ScheduleEvent(eVelhariEvents::EventSanguineFont,         eVelhariTimers::TimerSanguineFont);
            /// All previous abilities continue during phase 3
            events.ScheduleEvent(eVelhariEvents::EventSealOfDecay,          eVelhariTimers::TimerSealOfDecay);
            events.ScheduleEvent(eVelhariEvents::EventEdictOfCondemnation,  eVelhariTimers::TimerEdictOfCondemnation);
            events.ScheduleEvent(eVelhariEvents::EventAnimaFont,            eVelhariTimers::TimerAnimaFont);
            events.ScheduleEvent(eVelhariEvents::EventAnnihilationStrike,   eVelhariTimers::TimerAnnihilationStrike);
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
                    case eVelhariEvents::EventSealOfDecay:
                    {
                        /// Apply to current tank - stacking DoT that forces swaps
                        if (Unit* l_Tank = me->GetVictim())
                            me->CastSpell(l_Tank, eVelhariSpells::SpellSealOfDecay, false);
                        events.ScheduleEvent(eVelhariEvents::EventSealOfDecay, eVelhariTimers::TimerSealOfDecay);
                        break;
                    }
                    case eVelhariEvents::EventEdictOfCondemnation:
                    {
                        /// Apply to a random non-tank player
                        if (Unit* l_Target = SelectTarget(SELECT_TARGET_RANDOM, 1, 60.0f, true))
                            me->CastSpell(l_Target, eVelhariSpells::SpellEdictOfCondemnation, false);
                        events.ScheduleEvent(eVelhariEvents::EventEdictOfCondemnation, eVelhariTimers::TimerEdictOfCondemnation);
                        break;
                    }
                    case eVelhariEvents::EventGavelOfTheTyrant:
                    {
                        me->CastSpell(me->GetVictim(), eVelhariSpells::SpellGavelOfTheTyrant, false);
                        events.ScheduleEvent(eVelhariEvents::EventGavelOfTheTyrant, eVelhariTimers::TimerGavelOfTheTyrant);
                        break;
                    }
                    case eVelhariEvents::EventAnimaFont:
                    {
                        /// Spawn Anima Fountain add - channels Font of Corruption beam
                        Position l_SpawnPos;
                        me->GetNearPosition(l_SpawnPos, frand(8.0f, 20.0f), frand(0.0f, 2.0f * M_PI));
                        me->CastSpell(l_SpawnPos, eVelhariSpells::SpellAnimaFont, true);

                        if (Creature* l_Font = me->SummonCreature(eVelhariCreatures::NpcAnimaFountain,
                            l_SpawnPos, TEMPSUMMON_TIMED_DESPAWN, 30000))
                        {
                            l_Font->CastSpell(l_Font, eVelhariSpells::SpellFontOfCorruption, true);
                        }
                        events.ScheduleEvent(eVelhariEvents::EventAnimaFont, eVelhariTimers::TimerAnimaFont);
                        break;
                    }
                    case eVelhariEvents::EventAnnihilationStrike:
                    {
                        me->CastSpell(me->GetVictim(), eVelhariSpells::SpellAnnihilationStrike, false);
                        events.ScheduleEvent(eVelhariEvents::EventAnnihilationStrike, eVelhariTimers::TimerAnnihilationStrike);
                        break;
                    }
                    case eVelhariEvents::EventFinalVerdict:
                    {
                        /// Massive unavoidable raid damage in phase 3
                        me->CastSpell(me, eVelhariSpells::SpellFinalVerdict, false);
                        events.ScheduleEvent(eVelhariEvents::EventFinalVerdict, eVelhariTimers::TimerFinalVerdict);
                        break;
                    }
                    case eVelhariEvents::EventSanguineFont:
                    {
                        /// Spawn Sanguine Font blood pool add
                        Position l_SpawnPos;
                        me->GetNearPosition(l_SpawnPos, frand(8.0f, 25.0f), frand(0.0f, 2.0f * M_PI));
                        me->CastSpell(l_SpawnPos, eVelhariSpells::SpellSanguineFont, true);

                        if (Creature* l_Font = me->SummonCreature(eVelhariCreatures::NpcSanguineFontAdd,
                            l_SpawnPos, TEMPSUMMON_TIMED_DESPAWN, 25000))
                        {
                            l_Font->SetInCombatWithZone();
                        }
                        events.ScheduleEvent(eVelhariEvents::EventSanguineFont, eVelhariTimers::TimerSanguineFont);
                        break;
                    }
                    case eVelhariEvents::EventAncientCorruption:
                    {
                        /// Apply a stack of Ancient Corruption to all players (intensifies by phase)
                        Map::PlayerList const& l_Players = me->GetMap()->GetPlayers();
                        for (Map::PlayerList::const_iterator l_Iter = l_Players.begin(); l_Iter != l_Players.end(); ++l_Iter)
                        {
                            if (Player* l_Player = l_Iter->getSource())
                            {
                                if (l_Player->IsAlive())
                                    me->CastSpell(l_Player, eVelhariSpells::SpellAncientCorruption, true);
                            }
                        }
                        events.ScheduleEvent(eVelhariEvents::EventAncientCorruption, eVelhariTimers::TimerAncientCorruption);
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
        return new boss_velhari_hfcAI(p_Creature);
    }
};

/// Anima Fountain - 93569
/// Channeling add spawned during Phase 2; pulses Font of Corruption
class npc_velhari_anima_fountain : public CreatureScript
{
public:
    npc_velhari_anima_fountain() : CreatureScript("npc_velhari_anima_fountain") { }

    struct npc_velhari_anima_fountainAI : public ScriptedAI
    {
        npc_velhari_anima_fountainAI(Creature* p_Creature) : ScriptedAI(p_Creature)
        {
            m_PulseTimer = 0;
        }

        uint32 m_PulseTimer;

        void Reset() override
        {
            me->SetReactState(ReactStates::REACT_PASSIVE);
            me->SetFlag(EUnitFields::UNIT_FIELD_FLAGS, eUnitFlags::UNIT_FLAG_NOT_SELECTABLE);
            m_PulseTimer = eVelhariTimers::TimerFontCorruptionPulse;
        }

        void UpdateAI(uint32 const p_Diff) override
        {
            if (m_PulseTimer <= p_Diff)
            {
                me->CastSpell(me, eVelhariSpells::SpellFontOfCorruption, false);
                m_PulseTimer = eVelhariTimers::TimerFontCorruptionPulse;
            }
            else
                m_PulseTimer -= p_Diff;
        }
    };

    CreatureAI* GetAI(Creature* p_Creature) const override
    {
        return new npc_velhari_anima_fountainAI(p_Creature);
    }
};

/// Sanguine Font add - 93570
/// Phase 3 blood pool add that deals pulsing shadow damage
class npc_velhari_sanguine_font : public CreatureScript
{
public:
    npc_velhari_sanguine_font() : CreatureScript("npc_velhari_sanguine_font") { }

    struct npc_velhari_sanguine_fontAI : public ScriptedAI
    {
        npc_velhari_sanguine_fontAI(Creature* p_Creature) : ScriptedAI(p_Creature)
        {
            m_PulseTimer = 0;
        }

        uint32 m_PulseTimer;

        void Reset() override
        {
            me->SetReactState(ReactStates::REACT_PASSIVE);
            me->SetFlag(EUnitFields::UNIT_FIELD_FLAGS, eUnitFlags::UNIT_FLAG_NOT_SELECTABLE);
            m_PulseTimer = 3000;
        }

        void UpdateAI(uint32 const p_Diff) override
        {
            if (m_PulseTimer <= p_Diff)
            {
                me->CastSpell(me, eVelhariSpells::SpellSanguineFont, false);
                m_PulseTimer = 3000;
            }
            else
                m_PulseTimer -= p_Diff;
        }
    };

    CreatureAI* GetAI(Creature* p_Creature) const override
    {
        return new npc_velhari_sanguine_fontAI(p_Creature);
    }
};

/// Spell: Seal of Decay - 180353
/// AuraScript: stacks debuff each tick; high stacks trigger a tank swap mechanic
class spell_velhari_seal_of_decay : public SpellScriptLoader
{
public:
    spell_velhari_seal_of_decay() : SpellScriptLoader("spell_velhari_seal_of_decay") { }

    class spell_velhari_seal_of_decay_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_velhari_seal_of_decay_AuraScript);

        void HandlePeriodic(AuraEffect const* /*p_AurEff*/)
        {
            Unit* l_Target = GetTarget();
            if (!l_Target)
                return;

            /// At 5 stacks warn the tank they need to be swapped
            if (GetStackAmount() >= 5)
            {
                /// The actual swap is handled by players taunting; here we amplify
                /// damage for gameplay pressure
                l_Target->CastSpell(l_Target, eVelhariSpells::SpellAncientCorruption, true);
            }
        }

        void Register() override
        {
            OnEffectPeriodic += AuraEffectPeriodicFn(spell_velhari_seal_of_decay_AuraScript::HandlePeriodic,
                EFFECT_0, SPELL_AURA_PERIODIC_DAMAGE);
        }
    };

    AuraScript* GetAuraScript() const override { return new spell_velhari_seal_of_decay_AuraScript(); }
};

/// Spell: Edict of Condemnation - 182884
/// AuraScript: explodes at expiry dealing massive damage to nearby players
class spell_velhari_edict_of_condemnation : public SpellScriptLoader
{
public:
    spell_velhari_edict_of_condemnation() : SpellScriptLoader("spell_velhari_edict_of_condemnation") { }

    class spell_velhari_edict_of_condemnation_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_velhari_edict_of_condemnation_AuraScript);

        void HandleRemove(AuraEffect const* /*p_AurEff*/, AuraEffectHandleModes /*p_Mode*/)
        {
            Unit* l_Target = GetTarget();
            if (!l_Target)
                return;

            /// Trigger the final condemnation damage burst
            l_Target->CastSpell(l_Target, eVelhariSpells::SpellFinalVerdict, true);
        }

        void Register() override
        {
            OnEffectRemove += AuraEffectRemoveFn(spell_velhari_edict_of_condemnation_AuraScript::HandleRemove,
                EFFECT_0, SPELL_AURA_PERIODIC_DUMMY, AURA_EFFECT_HANDLE_REAL);
        }
    };

    AuraScript* GetAuraScript() const override { return new spell_velhari_edict_of_condemnation_AuraScript(); }
};

/// Spell: Ancient Corruption - 180351
/// AuraScript: intensifies (increases damage) based on Velhari's current phase
class spell_velhari_ancient_corruption : public SpellScriptLoader
{
public:
    spell_velhari_ancient_corruption() : SpellScriptLoader("spell_velhari_ancient_corruption") { }

    class spell_velhari_ancient_corruption_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_velhari_ancient_corruption_AuraScript);

        void HandleApply(AuraEffect const* /*p_AurEff*/, AuraEffectHandleModes /*p_Mode*/)
        {
            // Stacking corruption - handled by server-side stack amount
        }

        void HandleRemove(AuraEffect const* /*p_AurEff*/, AuraEffectHandleModes /*p_Mode*/)
        {
            // On expiry trigger a burst if too many stacks
            Unit* l_Target = GetTarget();
            if (l_Target && GetStackAmount() >= 10)
                l_Target->CastSpell(l_Target, eVelhariSpells::SpellEdictOfCondemnation, true);
        }

        void Register() override
        {
            OnEffectApply  += AuraEffectApplyFn(spell_velhari_ancient_corruption_AuraScript::HandleApply,
                EFFECT_0, SPELL_AURA_PERIODIC_DAMAGE, AURA_EFFECT_HANDLE_REAL);
            OnEffectRemove += AuraEffectRemoveFn(spell_velhari_ancient_corruption_AuraScript::HandleRemove,
                EFFECT_0, SPELL_AURA_PERIODIC_DAMAGE, AURA_EFFECT_HANDLE_REAL);
        }
    };

    AuraScript* GetAuraScript() const override { return new spell_velhari_ancient_corruption_AuraScript(); }
};

#ifndef __clang_analyzer__
void AddSC_boss_velhari_hfc()
{
    /// Boss
    new boss_velhari_hfc();

    /// Creatures
    new npc_velhari_anima_fountain();
    new npc_velhari_sanguine_font();

    /// Spells
    new spell_velhari_seal_of_decay();
    new spell_velhari_edict_of_condemnation();
    new spell_velhari_ancient_corruption();
}
#endif
