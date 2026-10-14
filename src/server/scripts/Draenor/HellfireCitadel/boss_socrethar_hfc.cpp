///////////////////////////////////////////////////////////////////////////////
///
///  MILLENIUM-STUDIO
///  Copyright 2016 Millenium-studio SARL
///  All Rights Reserved.
///
///////////////////////////////////////////////////////////////////////////////

#include "boss_socrethar_hfc.h"
#include "hellfire_citadel.h"

/// Socrethar the Eternal - 92330
/// Two-phase encounter:
///   Phase 1: Players fight the Soulbound Construct (90296). Socrethar directs it from afar.
///   Phase 2: At 30% Construct HP, Socrethar abandons the Construct, becomes the primary target.
class boss_socrethar : public CreatureScript
{
public:
    boss_socrethar() : CreatureScript("boss_socrethar") { }

    struct boss_socretharAI : public BossAI
    {
        boss_socretharAI(Creature* creature) : BossAI(creature, eCitadelDatas::DataSocrethar)
        {
            m_Instance      = creature->GetInstanceScript();
            m_Phase         = eSocretharPhases::PHASE_CONSTRUCT;
            m_ConstructGUID = 0;
        }

        InstanceScript* m_Instance;
        uint8           m_Phase;
        uint64          m_ConstructGUID;

        void Reset() override
        {
            _Reset();

            m_Phase         = eSocretharPhases::PHASE_CONSTRUCT;
            m_ConstructGUID = 0;

            // Socrethar starts untargetable – players fight the Construct in Phase 1
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
            me->SetReactState(REACT_PASSIVE);

            DespawnCreaturesInArea(NPC_SOULBOUND_CONSTRUCT,    me);
            DespawnCreaturesInArea(NPC_SARGEREI_SPIRITBINDER,  me);
            DespawnCreaturesInArea(NPC_SARGEREI_DOMINATOR,     me);
        }

        void EnterCombat(Unit* /*who*/) override
        {
            _EnterCombat();

            m_Phase = eSocretharPhases::PHASE_CONSTRUCT;

            // Summon the Soulbound Construct – it will do the fighting in Phase 1
            Position constructPos = *me;
            me->MovePosition(constructPos, 10.0f, 0.0f);
            if (Creature* construct = me->SummonCreature(NPC_SOULBOUND_CONSTRUCT, constructPos))
            {
                m_ConstructGUID = construct->GetGUID();
                construct->SetInCombatWithZone();
            }
        }

        void JustDied(Unit* /*killer*/) override
        {
            _JustDied();

            DespawnCreaturesInArea(NPC_SOULBOUND_CONSTRUCT,    me);
            DespawnCreaturesInArea(NPC_SARGEREI_SPIRITBINDER,  me);
            DespawnCreaturesInArea(NPC_SARGEREI_DOMINATOR,     me);

            // Grant bonus loot
            me->CastSpell(me, SPELL_SOCRETHAR_BONUS_LOOT, true);
        }

        void KilledUnit(Unit* /*victim*/) override { }

        void DoAction(int32 action) override
        {
            switch (action)
            {
                case ACTION_CONSTRUCT_WEAK:
                case ACTION_CONSTRUCT_SLAIN:
                    EnterPhase2();
                    break;
                default:
                    break;
            }
        }

        void EnterPhase2()
        {
            if (m_Phase == eSocretharPhases::PHASE_SOCRETHAR)
                return;

            m_Phase = eSocretharPhases::PHASE_SOCRETHAR;

            // Despawn / kill the Construct
            if (Creature* construct = ObjectAccessor::GetCreature(*me, m_ConstructGUID))
            {
                construct->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
                construct->DespawnOrUnsummon(3000);
            }
            m_ConstructGUID = 0;

            // Socrethar becomes attackable
            me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
            me->SetReactState(REACT_AGGRESSIVE);
            me->SetInCombatWithZone();

            // Schedule Phase 2 abilities
            events.Reset();
            events.ScheduleEvent(EVENT_DOMINATING_POWER,   TIMER_DOMINATING_POWER);
            events.ScheduleEvent(EVENT_SHADOW_WORD_AGONY,  TIMER_SHADOW_WORD_AGONY);
            events.ScheduleEvent(EVENT_EXERTAR_OF_SOULS,   TIMER_EXERTAR_OF_SOULS);
            events.ScheduleEvent(EVENT_SPAWN_SARGEREI,     TIMER_SPAWN_SARGEREI);
            events.ScheduleEvent(EVENT_APOCALYPSE,         TIMER_APOCALYPSE);
        }

        void SpawnSargerei()
        {
            Position pos;
            me->GetRandomNearPosition(pos, 20.0f);
            if (Creature* binder = me->SummonCreature(NPC_SARGEREI_SPIRITBINDER, pos))
                binder->SetInCombatWithZone();

            me->GetRandomNearPosition(pos, 20.0f);
            if (Creature* dom = me->SummonCreature(NPC_SARGEREI_DOMINATOR, pos))
                dom->SetInCombatWithZone();
        }

        void UpdateAI(uint32 diff) override
        {
            if (m_Phase == eSocretharPhases::PHASE_CONSTRUCT)
                return; // During Phase 1 the Construct AI drives the encounter

            if (!UpdateVictim())
                return;

            events.Update(diff);

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    // ── Dominating Power – mind control on random player ──────────
                    case EVENT_DOMINATING_POWER:
                    {
                        if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 1, 50.0f))
                            me->CastSpell(target, SPELL_DOMINATING_POWER, false);
                        events.ScheduleEvent(EVENT_DOMINATING_POWER, TIMER_DOMINATING_POWER);
                        break;
                    }

                    // ── Shadow Word: Agony – stacking shadow DoT ─────────────────
                    case EVENT_SHADOW_WORD_AGONY:
                    {
                        if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 50.0f))
                            me->CastSpell(target, SPELL_SHADOW_WORD_AGONY, false);
                        events.ScheduleEvent(EVENT_SHADOW_WORD_AGONY, TIMER_SHADOW_WORD_AGONY);
                        break;
                    }

                    // ── Exertar of Souls – soul rip explosion ────────────────────
                    case EVENT_EXERTAR_OF_SOULS:
                    {
                        if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 50.0f))
                            me->CastSpell(target, SPELL_EXERTAR_OF_SOULS, false);
                        events.ScheduleEvent(EVENT_EXERTAR_OF_SOULS, TIMER_EXERTAR_OF_SOULS);
                        break;
                    }

                    // ── Sargerei reinforcements ───────────────────────────────────
                    case EVENT_SPAWN_SARGEREI:
                    {
                        SpawnSargerei();
                        events.ScheduleEvent(EVENT_SPAWN_SARGEREI, TIMER_SPAWN_SARGEREI);
                        break;
                    }

                    // ── Apocalypse – devastating AoE ─────────────────────────────
                    case EVENT_APOCALYPSE:
                    {
                        me->CastSpell(me, SPELL_APOCALYPSE_FINAL, false);
                        events.ScheduleEvent(EVENT_APOCALYPSE, TIMER_APOCALYPSE);
                        break;
                    }

                    default:
                        break;
                }
            }

            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new boss_socretharAI(creature);
    }
};

/// Soulbound Construct - 90296
/// Phase 1 actor. Players fight this creature; at 30% it triggers Phase 2.
class npc_soulbound_construct : public CreatureScript
{
public:
    npc_soulbound_construct() : CreatureScript("npc_soulbound_construct") { }

    struct npc_soulbound_constructAI : public ScriptedAI
    {
        npc_soulbound_constructAI(Creature* creature) : ScriptedAI(creature)
        {
            m_Phase2Triggered = false;
        }

        bool m_Phase2Triggered;

        void Reset() override
        {
            m_Phase2Triggered = false;
            me->SetReactState(REACT_AGGRESSIVE);

            events.ScheduleEvent(EVENT_CONSTRUCT_REVERB,    TIMER_REVERBERATING_BLOW);
            events.ScheduleEvent(EVENT_CONSTRUCT_FEL_PRISON, TIMER_FEL_PRISON);
            events.ScheduleEvent(EVENT_GIFT_OF_MANARI,      TIMER_GIFT_OF_MANARI);
            events.ScheduleEvent(EVENT_CONSTRUCT_APOCALYPSE, TIMER_CONSTRUCT_APOCALYPSE);
        }

        void DamageTaken(Unit* /*attacker*/, uint32& damage, SpellInfo const* /*spellInfo*/) override
        {
            if (!m_Phase2Triggered && me->HealthBelowPctDamaged(30, damage))
            {
                m_Phase2Triggered = true;
                NotifySocrethar(ACTION_CONSTRUCT_WEAK);
            }
        }

        void JustDied(Unit* /*killer*/) override
        {
            NotifySocrethar(ACTION_CONSTRUCT_SLAIN);
        }

        void NotifySocrethar(int32 action)
        {
            std::list<Creature*> bossList;
            GetCreatureListWithEntryInGrid(bossList, me, eCitadelCreatures::BossSocrethar, 300.0f);
            for (Creature* boss : bossList)
            {
                if (boss && boss->IsAlive())
                {
                    boss->AI()->DoAction(action);
                    break;
                }
            }
        }

        void UpdateAI(uint32 diff) override
        {
            if (!UpdateVictim())
                return;

            events.Update(diff);

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    // ── Reverberating Blow – massive tank cleave ──────────────────
                    case EVENT_CONSTRUCT_REVERB:
                    {
                        if (Unit* target = me->GetVictim())
                            me->CastSpell(target, SPELL_REVERBERATING_BLOW, false);
                        events.ScheduleEvent(EVENT_CONSTRUCT_REVERB, TIMER_REVERBERATING_BLOW);
                        break;
                    }

                    // ── Fel Prison – immobilise a random player ───────────────────
                    case EVENT_CONSTRUCT_FEL_PRISON:
                    {
                        if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 1, 50.0f))
                            me->CastSpell(target, SPELL_FEL_PRISON, false);
                        events.ScheduleEvent(EVENT_CONSTRUCT_FEL_PRISON, TIMER_FEL_PRISON);
                        break;
                    }

                    // ── Gift of the Man'ari – AoE heal for Construct ──────────────
                    case EVENT_GIFT_OF_MANARI:
                    {
                        me->CastSpell(me, SPELL_GIFT_OF_MANARI, false);
                        events.ScheduleEvent(EVENT_GIFT_OF_MANARI, TIMER_GIFT_OF_MANARI);
                        break;
                    }

                    // ── Construct Apocalypse – devastating channel ────────────────
                    case EVENT_CONSTRUCT_APOCALYPSE:
                    {
                        me->CastSpell(me, SPELL_CONSTRUCT_APOCALYPSE, false);
                        events.ScheduleEvent(EVENT_CONSTRUCT_APOCALYPSE, TIMER_CONSTRUCT_APOCALYPSE);
                        break;
                    }

                    default:
                        break;
                }
            }

            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_soulbound_constructAI(creature);
    }
};

/// Sargerei Spiritbinder - 92510 (Phase 2 healer add)
class npc_sargerei_spiritbinder : public CreatureScript
{
public:
    npc_sargerei_spiritbinder() : CreatureScript("npc_sargerei_spiritbinder") { }

    struct npc_sargerei_spiritbinderAI : public ScriptedAI
    {
        npc_sargerei_spiritbinderAI(Creature* creature) : ScriptedAI(creature) { }

        void Reset() override
        {
            me->SetReactState(REACT_AGGRESSIVE);
            events.ScheduleEvent(EVENT_SHADOW_WORD_AGONY, 8000);
        }

        void UpdateAI(uint32 diff) override
        {
            if (!UpdateVictim())
                return;

            events.Update(diff);

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_SHADOW_WORD_AGONY:
                    {
                        if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 40.0f))
                            me->CastSpell(target, SPELL_SHADOW_WORD_AGONY, false);
                        events.ScheduleEvent(EVENT_SHADOW_WORD_AGONY, 10000);
                        break;
                    }
                    default:
                        break;
                }
            }

            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_sargerei_spiritbinderAI(creature);
    }
};

/// Sargerei Dominator - 92507 (Phase 2 melee add)
class npc_sargerei_dominator : public CreatureScript
{
public:
    npc_sargerei_dominator() : CreatureScript("npc_sargerei_dominator") { }

    struct npc_sargerei_dominatorAI : public ScriptedAI
    {
        npc_sargerei_dominatorAI(Creature* creature) : ScriptedAI(creature) { }

        void Reset() override
        {
            me->SetReactState(REACT_AGGRESSIVE);
            events.ScheduleEvent(EVENT_DOMINATING_POWER, 10000);
        }

        void UpdateAI(uint32 diff) override
        {
            if (!UpdateVictim())
                return;

            events.Update(diff);

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_DOMINATING_POWER:
                    {
                        if (Unit* target = me->GetVictim())
                            me->CastSpell(target, SPELL_DOMINATING_POWER, false);
                        events.ScheduleEvent(EVENT_DOMINATING_POWER, 15000);
                        break;
                    }
                    default:
                        break;
                }
            }

            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_sargerei_dominatorAI(creature);
    }
};

void AddSC_boss_socrethar_hfc()
{
    /// Boss
    new boss_socrethar();

    /// Creatures
    new npc_soulbound_construct();
    new npc_sargerei_spiritbinder();
    new npc_sargerei_dominator();

    /// Spells
    // (No custom spell scripts needed – all handled in creature AI)
}
