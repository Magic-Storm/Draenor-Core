///////////////////////////////////////////////////////////////////////////////
///
///  MILLENIUM-STUDIO
///  Copyright 2016 Millenium-studio SARL
///  All Rights Reserved.
///
///////////////////////////////////////////////////////////////////////////////

#include "boss_xhulhorac_hfc.h"
#include "hellfire_citadel.h"

/// Xhul'horac - 93068
class boss_xhulhorac_hfc : public CreatureScript
{
public:
    boss_xhulhorac_hfc() : CreatureScript("boss_xhulhorac_hfc") { }

    enum eTalks
    {
        TalkAggro       = 0,
        TalkSlay        = 1,
        TalkDeath       = 2,
        TalkFelPhase    = 3,
        TalkVoidPhase   = 4,
        TalkCombined    = 5,
        TalkWipe        = 6,
    };

    struct boss_xhulhorac_hfcAI : public BossAI
    {
        boss_xhulhorac_hfcAI(Creature* p_Creature) : BossAI(p_Creature, eCitadelDatas::DataXhulhorac)
        {
            m_Instance      = p_Creature->GetInstanceScript();
            m_Phase         = eXhulPhases::PhaseFel;
            m_PhaseSwitchCount = 0;
        }

        InstanceScript* m_Instance;
        uint8           m_Phase;
        uint8           m_PhaseSwitchCount;

        void Reset() override
        {
            _Reset();

            m_Phase             = eXhulPhases::PhaseFel;
            m_PhaseSwitchCount  = 0;

            me->SetReactState(ReactStates::REACT_AGGRESSIVE);
            me->RemoveAura(eXhulSpells::SpellFelAffinity);
            me->RemoveAura(eXhulSpells::SpellVoidAffinity);

            if (m_Instance != nullptr)
                m_Instance->SendEncounterUnit(EncounterFrameType::ENCOUNTER_FRAME_DISENGAGE, me);
        }

        void EnterCombat(Unit* /*p_Who*/) override
        {
            _EnterCombat();

            Talk(eTalks::TalkAggro);

            me->AddAura(eXhulSpells::SpellFelAffinity, me);

            if (m_Instance != nullptr)
            {
                m_Instance->SendEncounterUnit(EncounterFrameType::ENCOUNTER_FRAME_ENGAGE, me, 1);
                m_Instance->SetBossState(eCitadelDatas::DataXhulhorac, EncounterState::IN_PROGRESS);
            }

            ScheduleFelPhaseEvents();
            events.ScheduleEvent(eXhulEvents::EventPhaseSwitch, eXhulTimers::TimerPhaseSwitch);
        }

        void JustReachedHome() override
        {
            _Reset();
            Talk(eTalks::TalkWipe);
            summons.DespawnAll();

            if (m_Instance != nullptr)
            {
                m_Instance->SendEncounterUnit(EncounterFrameType::ENCOUNTER_FRAME_DISENGAGE, me);
                m_Instance->SetBossState(eCitadelDatas::DataXhulhorac, EncounterState::FAIL);
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

            /// Remove lingering auras from players
            Map::PlayerList const& l_Players = me->GetMap()->GetPlayers();
            for (Map::PlayerList::const_iterator l_Iter = l_Players.begin(); l_Iter != l_Players.end(); ++l_Iter)
            {
                if (Player* l_Player = l_Iter->getSource())
                {
                    l_Player->RemoveAura(eXhulSpells::SpellChainsOfFel);
                    l_Player->RemoveAura(eXhulSpells::SpellCreepingVoid);
                }
            }

            if (m_Instance != nullptr)
            {
                m_Instance->SendEncounterUnit(EncounterFrameType::ENCOUNTER_FRAME_DISENGAGE, me);
                m_Instance->SetBossState(eCitadelDatas::DataXhulhorac, EncounterState::DONE);

                if (sObjectMgr->IsDisabledEncounter(m_Instance->GetEncounterIDForBoss(me), GetDifficulty()))
                    me->SetLootRecipient(nullptr);
                else
                    CastSpellToPlayers(me->GetMap(), me, eXhulSpells::SpellXhulBonusLoot, true);
            }
        }

        void DamageTaken(Unit* /*p_Attacker*/, uint32& p_Damage, SpellInfo const* /*p_SpellInfo*/) override
        {
            if (m_Phase != eXhulPhases::PhaseCombined && me->HealthBelowPctDamaged(30, p_Damage))
                DoAction(eXhulActions::ActionEnterCombined);
        }

        void DoAction(int32 const p_Action) override
        {
            switch (p_Action)
            {
                case eXhulActions::ActionSwitchPhase:
                    SwitchPhase();
                    break;
                case eXhulActions::ActionEnterCombined:
                    EnterCombinedPhase();
                    break;
                default:
                    break;
            }
        }

        void SwitchPhase()
        {
            if (m_Phase == eXhulPhases::PhaseCombined)
                return;

            CancelCurrentPhaseEvents();
            ++m_PhaseSwitchCount;

            if (m_Phase == eXhulPhases::PhaseFel)
            {
                m_Phase = eXhulPhases::PhaseVoid;
                me->RemoveAura(eXhulSpells::SpellFelAffinity);
                me->AddAura(eXhulSpells::SpellVoidAffinity, me);
                Talk(eTalks::TalkVoidPhase);
                ScheduleVoidPhaseEvents();
            }
            else
            {
                m_Phase = eXhulPhases::PhaseFel;
                me->RemoveAura(eXhulSpells::SpellVoidAffinity);
                me->AddAura(eXhulSpells::SpellFelAffinity, me);
                Talk(eTalks::TalkFelPhase);
                ScheduleFelPhaseEvents();
            }

            events.ScheduleEvent(eXhulEvents::EventPhaseSwitch, eXhulTimers::TimerPhaseSwitch);
        }

        void EnterCombinedPhase()
        {
            m_Phase = eXhulPhases::PhaseCombined;

            Talk(eTalks::TalkCombined);

            me->RemoveAura(eXhulSpells::SpellFelAffinity);
            me->RemoveAura(eXhulSpells::SpellVoidAffinity);
            me->AddAura(eXhulSpells::SpellFelAffinity, me);
            me->AddAura(eXhulSpells::SpellVoidAffinity, me);

            events.CancelEvent(eXhulEvents::EventPhaseSwitch);

            /// Enable all abilities
            ScheduleFelPhaseEvents();
            ScheduleVoidPhaseEvents();

            events.ScheduleEvent(eXhulEvents::EventVoidFelExplosion, eXhulTimers::TimerVoidFelExplosion);
        }

        void CancelCurrentPhaseEvents()
        {
            events.CancelEvent(eXhulEvents::EventFelblazeFlurry);
            events.CancelEvent(eXhulEvents::EventChainsOfFel);
            events.CancelEvent(eXhulEvents::EventFelstar);
            events.CancelEvent(eXhulEvents::EventOpenFelPortal);
            events.CancelEvent(eXhulEvents::EventCreepingVoid);
            events.CancelEvent(eXhulEvents::EventBlackHole);
            events.CancelEvent(eXhulEvents::EventVoidStep);
            events.CancelEvent(eXhulEvents::EventVoidstar);
            events.CancelEvent(eXhulEvents::EventOpenVoidPortal);
        }

        void ScheduleFelPhaseEvents()
        {
            events.ScheduleEvent(eXhulEvents::EventFelblazeFlurry,  eXhulTimers::TimerFelblazeFlurry);
            events.ScheduleEvent(eXhulEvents::EventChainsOfFel,     eXhulTimers::TimerChainsOfFel);
            events.ScheduleEvent(eXhulEvents::EventFelstar,          eXhulTimers::TimerFelstar);
            events.ScheduleEvent(eXhulEvents::EventOpenFelPortal,   eXhulTimers::TimerOpenFelPortal);
        }

        void ScheduleVoidPhaseEvents()
        {
            events.ScheduleEvent(eXhulEvents::EventCreepingVoid,    eXhulTimers::TimerCreepingVoid);
            events.ScheduleEvent(eXhulEvents::EventBlackHole,       eXhulTimers::TimerBlackHole);
            events.ScheduleEvent(eXhulEvents::EventVoidStep,         eXhulTimers::TimerVoidStep);
            events.ScheduleEvent(eXhulEvents::EventVoidstar,         eXhulTimers::TimerVoidstar);
            events.ScheduleEvent(eXhulEvents::EventOpenVoidPortal,  eXhulTimers::TimerOpenVoidPortal);
        }

        void SummonFelPortalAdd()
        {
            Position l_SpawnPos;
            me->GetNearPosition(l_SpawnPos, frand(20.0f, 35.0f), frand(0.0f, 2.0f * M_PI));

            /// Spawn Omnus - large Fel add
            if (Creature* l_Omnus = me->SummonCreature(eXhulCreatures::NpcOmnus,
                l_SpawnPos, TEMPSUMMON_CORPSE_DESPAWN, 0))
            {
                l_Omnus->SetInCombatWithZone();
            }

            /// Accompany with a pair of Fel Imps
            for (uint8 i = 0; i < 2; ++i)
            {
                Position l_ImpPos;
                me->GetNearPosition(l_ImpPos, frand(10.0f, 20.0f), frand(0.0f, 2.0f * M_PI));
                if (Creature* l_Imp = me->SummonCreature(eXhulCreatures::NpcFelImpXhul,
                    l_ImpPos, TEMPSUMMON_CORPSE_DESPAWN, 0))
                {
                    l_Imp->SetInCombatWithZone();
                }
            }
        }

        void SummonVoidPortalAdd()
        {
            Position l_SpawnPos;
            me->GetNearPosition(l_SpawnPos, frand(20.0f, 35.0f), frand(0.0f, 2.0f * M_PI));

            if (Creature* l_Void = me->SummonCreature(eXhulCreatures::NpcUnstableVoidfiend,
                l_SpawnPos, TEMPSUMMON_CORPSE_DESPAWN, 0))
            {
                l_Void->SetInCombatWithZone();
            }
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
                    case eXhulEvents::EventFelblazeFlurry:
                    {
                        if (Unit* l_Target = me->GetVictim())
                            me->CastSpell(l_Target, eXhulSpells::SpellFelblazeFlurry, false);
                        events.ScheduleEvent(eXhulEvents::EventFelblazeFlurry, eXhulTimers::TimerFelblazeFlurry);
                        break;
                    }
                    case eXhulEvents::EventChainsOfFel:
                    {
                        if (Unit* l_Target = SelectTarget(SELECT_TARGET_RANDOM, 1, 60.0f, true))
                            me->CastSpell(l_Target, eXhulSpells::SpellChainsOfFel, false);
                        events.ScheduleEvent(eXhulEvents::EventChainsOfFel, eXhulTimers::TimerChainsOfFel);
                        break;
                    }
                    case eXhulEvents::EventFelstar:
                    {
                        if (Unit* l_Target = SelectTarget(SELECT_TARGET_RANDOM, 0, 60.0f, true))
                            me->CastSpell(l_Target, eXhulSpells::SpellFelstar, false);
                        events.ScheduleEvent(eXhulEvents::EventFelstar, eXhulTimers::TimerFelstar);
                        break;
                    }
                    case eXhulEvents::EventCreepingVoid:
                    {
                        if (Unit* l_Target = SelectTarget(SELECT_TARGET_RANDOM, 1, 60.0f, true))
                            me->CastSpell(l_Target, eXhulSpells::SpellCreepingVoid, false);
                        events.ScheduleEvent(eXhulEvents::EventCreepingVoid, eXhulTimers::TimerCreepingVoid);
                        break;
                    }
                    case eXhulEvents::EventBlackHole:
                    {
                        me->CastSpell(me, eXhulSpells::SpellBlackHole, false);
                        events.ScheduleEvent(eXhulEvents::EventBlackHole, eXhulTimers::TimerBlackHole);
                        break;
                    }
                    case eXhulEvents::EventVoidStep:
                    {
                        if (Unit* l_Target = SelectTarget(SELECT_TARGET_RANDOM, 1, 60.0f, true))
                        {
                            /// Leave void pool at current position then teleport
                            me->CastSpell(me, eXhulSpells::SpellVoidstar, true);
                            me->CastSpell(l_Target, eXhulSpells::SpellVoidStep, false);
                        }
                        events.ScheduleEvent(eXhulEvents::EventVoidStep, eXhulTimers::TimerVoidStep);
                        break;
                    }
                    case eXhulEvents::EventVoidstar:
                    {
                        if (Unit* l_Target = SelectTarget(SELECT_TARGET_RANDOM, 0, 60.0f, true))
                            me->CastSpell(l_Target, eXhulSpells::SpellVoidstar, false);
                        events.ScheduleEvent(eXhulEvents::EventVoidstar, eXhulTimers::TimerVoidstar);
                        break;
                    }
                    case eXhulEvents::EventOpenFelPortal:
                    {
                        me->CastSpell(me, eXhulSpells::SpellOpenFelPortal, true);
                        SummonFelPortalAdd();
                        events.ScheduleEvent(eXhulEvents::EventOpenFelPortal, eXhulTimers::TimerOpenFelPortal);
                        break;
                    }
                    case eXhulEvents::EventOpenVoidPortal:
                    {
                        me->CastSpell(me, eXhulSpells::SpellOpenVoidPortal, true);
                        SummonVoidPortalAdd();
                        events.ScheduleEvent(eXhulEvents::EventOpenVoidPortal, eXhulTimers::TimerOpenVoidPortal);
                        break;
                    }
                    case eXhulEvents::EventPhaseSwitch:
                    {
                        SwitchPhase();
                        break;
                    }
                    case eXhulEvents::EventVoidFelExplosion:
                    {
                        /// In combined phase, collision zones explode
                        me->CastSpell(me, eXhulSpells::SpellVoidFelExplosion, false);
                        events.ScheduleEvent(eXhulEvents::EventVoidFelExplosion, eXhulTimers::TimerVoidFelExplosion);
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
        return new boss_xhulhorac_hfcAI(p_Creature);
    }
};

/// Omnus - 93344
/// Large Fel add from Xhul'horac Fel portals
class npc_xhulhorac_omnus : public CreatureScript
{
public:
    npc_xhulhorac_omnus() : CreatureScript("npc_xhulhorac_omnus") { }

    struct npc_xhulhorac_omnusAI : public ScriptedAI
    {
        npc_xhulhorac_omnusAI(Creature* p_Creature) : ScriptedAI(p_Creature) { }

        void Reset() override
        {
            me->SetReactState(ReactStates::REACT_AGGRESSIVE);
            events.ScheduleEvent(eXhulEvents::EventOmnusFelBolt, eXhulTimers::TimerOmnusFelBolt);
        }

        void UpdateAI(uint32 const p_Diff) override
        {
            if (!UpdateVictim())
                return;

            events.Update(p_Diff);

            while (uint32 l_EventId = events.ExecuteEvent())
            {
                switch (l_EventId)
                {
                    case eXhulEvents::EventOmnusFelBolt:
                    {
                        if (Unit* l_Target = SelectTarget(SELECT_TARGET_RANDOM, 0, 40.0f, true))
                            me->CastSpell(l_Target, eXhulSpells::SpellFelstar, false);
                        events.ScheduleEvent(eXhulEvents::EventOmnusFelBolt, eXhulTimers::TimerOmnusFelBolt);
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
        return new npc_xhulhorac_omnusAI(p_Creature);
    }
};

/// Unstable Voidfiend - 93071
/// Void add from Xhul'horac Void portals; pulses void damage
class npc_xhulhorac_unstable_voidfiend : public CreatureScript
{
public:
    npc_xhulhorac_unstable_voidfiend() : CreatureScript("npc_xhulhorac_unstable_voidfiend") { }

    struct npc_xhulhorac_unstable_voidfiendAI : public ScriptedAI
    {
        npc_xhulhorac_unstable_voidfiendAI(Creature* p_Creature) : ScriptedAI(p_Creature) { }

        void Reset() override
        {
            me->SetReactState(ReactStates::REACT_AGGRESSIVE);
            events.ScheduleEvent(eXhulEvents::EventUnstableVoidPulse, eXhulTimers::TimerUnstableVoidPulse);
        }

        void UpdateAI(uint32 const p_Diff) override
        {
            if (!UpdateVictim())
                return;

            events.Update(p_Diff);

            while (uint32 l_EventId = events.ExecuteEvent())
            {
                switch (l_EventId)
                {
                    case eXhulEvents::EventUnstableVoidPulse:
                    {
                        me->CastSpell(me, eXhulSpells::SpellCreepingVoid, false);
                        events.ScheduleEvent(eXhulEvents::EventUnstableVoidPulse, eXhulTimers::TimerUnstableVoidPulse);
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
        return new npc_xhulhorac_unstable_voidfiendAI(p_Creature);
    }
};

/// Spell: Chains of Fel - 186500
/// Roots the target and deals periodic fel damage
class spell_xhulhorac_chains_of_fel : public SpellScriptLoader
{
public:
    spell_xhulhorac_chains_of_fel() : SpellScriptLoader("spell_xhulhorac_chains_of_fel") { }

    class spell_xhulhorac_chains_of_fel_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_xhulhorac_chains_of_fel_AuraScript);

        void HandlePeriodic(AuraEffect const* /*p_AurEff*/)
        {
            if (Unit* l_Target = GetTarget())
            {
                if (l_Target->IsAlive())
                    l_Target->CastSpell(l_Target, eXhulSpells::SpellFelstar, true);
            }
        }

        void Register() override
        {
            OnEffectPeriodic += AuraEffectPeriodicFn(spell_xhulhorac_chains_of_fel_AuraScript::HandlePeriodic,
                EFFECT_0, SPELL_AURA_PERIODIC_DAMAGE);
        }
    };

    AuraScript* GetAuraScript() const override { return new spell_xhulhorac_chains_of_fel_AuraScript(); }
};

#ifndef __clang_analyzer__
void AddSC_boss_xhulhorac_hfc()
{
    /// Boss
    new boss_xhulhorac_hfc();

    /// Creatures
    new npc_xhulhorac_omnus();
    new npc_xhulhorac_unstable_voidfiend();

    /// Spells
    new spell_xhulhorac_chains_of_fel();
}
#endif
