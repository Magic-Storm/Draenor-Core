///////////////////////////////////////////////////////////////////////////////
///
///  MILLENIUM-STUDIO
///  Copyright 2016 Millenium-studio SARL
///  All Rights Reserved.
///
///////////////////////////////////////////////////////////////////////////////

#include "boss_iskar_hfc.h"
#include "hellfire_citadel.h"

/// Shadow-Lord Iskar - 90316
class boss_shadow_lord_iskar : public CreatureScript
{
public:
    boss_shadow_lord_iskar() : CreatureScript("boss_shadow_lord_iskar") { }

    struct boss_shadow_lord_iskarAI : public BossAI
    {
        boss_shadow_lord_iskarAI(Creature* creature) : BossAI(creature, eCitadelDatas::DataIskar)
        {
            m_Instance  = creature->GetInstanceScript();
            m_EyeOwner  = 0;
            m_RavenCount = 0;
        }

        InstanceScript* m_Instance;
        uint8           m_Phase;
        uint64          m_EyeOwner;     ///< GUID of current Eye of Anzu holder
        uint8           m_RavenCount;

        void Reset() override
        {
            _Reset();

            m_Phase      = eIskarPhases::PHASE_ISKAR;
            m_EyeOwner   = 0;
            m_RavenCount = 0;

            me->SetReactState(REACT_AGGRESSIVE);
            me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);

            // Clean up any leftover adds
            DespawnCreaturesInArea(NPC_FEL_RAVEN,       me);
            DespawnCreaturesInArea(NPC_CORRUPTED_FELBLADE, me);
            DespawnCreaturesInArea(NPC_EYE_STALKER,     me);
        }

        void EnterCombat(Unit* /*who*/) override
        {
            _EnterCombat();

            m_Phase = eIskarPhases::PHASE_ISKAR;

            events.ScheduleEvent(EVENT_EYE_OF_ANZU,        TIMER_EYE_OF_ANZU);
            events.ScheduleEvent(EVENT_PHANTASMAL_WINDS,   TIMER_PHANTASMAL_WINDS);
            events.ScheduleEvent(EVENT_PHANTASMAL_WOUND,   TIMER_PHANTASMAL_WOUND);
            events.ScheduleEvent(EVENT_PHANTASMAL_FEL_BOMB, TIMER_FEL_BOMB);
            events.ScheduleEvent(EVENT_FEL_INCINERATION,   TIMER_FEL_INCINERATION);
            events.ScheduleEvent(EVENT_CHORD_OF_SHADOWS,   TIMER_CHORD_OF_SHADOWS);
            events.ScheduleEvent(EVENT_FEL_CHAKRAM,        TIMER_FEL_CHAKRAM);
            events.ScheduleEvent(EVENT_FOCUSED_CHI,        TIMER_FOCUSED_CHI);
            events.ScheduleEvent(EVENT_SHADOWS_GAZE,       TIMER_SHADOWS_GAZE);
            events.ScheduleEvent(EVENT_SPAWN_FEL_RAVEN,    TIMER_SPAWN_FEL_RAVEN);
            events.ScheduleEvent(EVENT_SPAWN_FELBLADE,     TIMER_SPAWN_FELBLADE);
        }

        void JustDied(Unit* /*killer*/) override
        {
            _JustDied();

            // Clean up all encounter-related creatures
            DespawnCreaturesInArea(NPC_FEL_RAVEN,          me);
            DespawnCreaturesInArea(NPC_CORRUPTED_FELBLADE,  me);
            DespawnCreaturesInArea(NPC_EYE_STALKER,        me);

            // Remove Eye of Anzu from all players
            Map::PlayerList const& players = me->GetMap()->GetPlayers();
            for (Map::PlayerList::const_iterator itr = players.begin(); itr != players.end(); ++itr)
            {
                if (Player* player = itr->getSource())
                    player->RemoveAura(SPELL_EYE_OF_ANZU);
            }

            // Grant bonus loot
            me->CastSpell(me, SPELL_ISKAR_BONUS_LOOT, true);
        }

        void KilledUnit(Unit* /*victim*/) override { }

        void DoAction(int32 action) override
        {
            switch (action)
            {
                case ACTION_EYE_OF_ANZU_TRANSFER:
                    TransferEyeOfAnzu();
                    break;
                case ACTION_EYE_EXPIRED:
                    // Aura expired naturally - re-schedule the initial cast
                    m_EyeOwner = 0;
                    events.ScheduleEvent(EVENT_EYE_OF_ANZU, TIMER_EYE_OF_ANZU);
                    break;
                default:
                    break;
            }
        }

        /// Move the Eye of Anzu aura from its current holder to a new random player
        void TransferEyeOfAnzu()
        {
            // Remove from current holder
            if (m_EyeOwner)
            {
                if (Unit* old = ObjectAccessor::GetUnit(*me, m_EyeOwner))
                    old->RemoveAura(SPELL_EYE_OF_ANZU);
                m_EyeOwner = 0;
            }

            // Build candidate list (alive players not currently holding the Eye)
            Map::PlayerList const& players = me->GetMap()->GetPlayers();
            std::vector<Player*> candidates;

            for (Map::PlayerList::const_iterator itr = players.begin(); itr != players.end(); ++itr)
            {
                if (Player* player = itr->getSource())
                {
                    if (player->IsAlive() && !player->HasAura(SPELL_EYE_OF_ANZU))
                        candidates.push_back(player);
                }
            }

            if (candidates.empty())
                return;

            Player* target = candidates[urand(0, candidates.size() - 1)];
            m_EyeOwner = target->GetGUID();
            me->CastSpell(target, SPELL_EYE_OF_ANZU, true);

            // Schedule the next automatic transfer
            events.ScheduleEvent(EVENT_TRANSFER_EYE, TIMER_EYE_TRANSFER);
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
                    // ── Eye of Anzu ──────────────────────────────────────────────
                    case EVENT_EYE_OF_ANZU:
                    {
                        TransferEyeOfAnzu();
                        break;
                    }
                    case EVENT_TRANSFER_EYE:
                    {
                        TransferEyeOfAnzu();
                        break;
                    }

                    // ── Phantasmal Winds – wide AoE knockback ────────────────────
                    case EVENT_PHANTASMAL_WINDS:
                    {
                        me->CastSpell(me, SPELL_PHANTASMAL_WINDS, false);
                        events.ScheduleEvent(EVENT_PHANTASMAL_WINDS, TIMER_PHANTASMAL_WINDS);
                        break;
                    }

                    // ── Phantasmal Wound – stacking DoT on random target ─────────
                    case EVENT_PHANTASMAL_WOUND:
                    {
                        if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 50.0f))
                            me->CastSpell(target, SPELL_PHANTASMAL_WOUND, false);
                        events.ScheduleEvent(EVENT_PHANTASMAL_WOUND, TIMER_PHANTASMAL_WOUND);
                        break;
                    }

                    // ── Phantasmal Fel Bomb – targeted ground explosion ──────────
                    case EVENT_PHANTASMAL_FEL_BOMB:
                    {
                        if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 50.0f))
                            me->CastSpell(target, SPELL_PHANTASMAL_FEL_BOMB, false);
                        events.ScheduleEvent(EVENT_PHANTASMAL_FEL_BOMB, TIMER_FEL_BOMB);
                        break;
                    }

                    // ── Fel Incineration – heavy channelled fire on tank ─────────
                    case EVENT_FEL_INCINERATION:
                    {
                        if (Unit* target = SelectTarget(SELECT_TARGET_TOPAGGRO))
                            me->CastSpell(target, SPELL_FEL_INCINERATION, false);
                        events.ScheduleEvent(EVENT_FEL_INCINERATION, TIMER_FEL_INCINERATION);
                        break;
                    }

                    // ── Chord of Shadows – tether between two players ────────────
                    case EVENT_CHORD_OF_SHADOWS:
                    {
                        if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 50.0f))
                            me->CastSpell(target, SPELL_CHORD_OF_SHADOWS, false);
                        events.ScheduleEvent(EVENT_CHORD_OF_SHADOWS, TIMER_CHORD_OF_SHADOWS);
                        break;
                    }

                    // ── Fel Chakram – bouncing projectile ────────────────────────
                    case EVENT_FEL_CHAKRAM:
                    {
                        if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 40.0f))
                            me->CastSpell(target, SPELL_FEL_CHAKRAM, false);
                        events.ScheduleEvent(EVENT_FEL_CHAKRAM, TIMER_FEL_CHAKRAM);
                        break;
                    }

                    // ── Focused Chi – concentrated burst on tank ─────────────────
                    case EVENT_FOCUSED_CHI:
                    {
                        if (Unit* target = SelectTarget(SELECT_TARGET_TOPAGGRO))
                            me->CastSpell(target, SPELL_FOCUSED_CHI, false);
                        events.ScheduleEvent(EVENT_FOCUSED_CHI, TIMER_FOCUSED_CHI);
                        break;
                    }

                    // ── Shadow's Gaze – stare mechanic on random player ──────────
                    case EVENT_SHADOWS_GAZE:
                    {
                        if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 50.0f))
                            me->CastSpell(target, SPELL_SHADOWS_GAZE, false);
                        events.ScheduleEvent(EVENT_SHADOWS_GAZE, TIMER_SHADOWS_GAZE);
                        break;
                    }

                    // ── Periodic Fel Raven spawn ─────────────────────────────────
                    case EVENT_SPAWN_FEL_RAVEN:
                    {
                        Position spawnPos;
                        me->GetRandomNearPosition(spawnPos, 25.0f);
                        if (Creature* raven = me->SummonCreature(NPC_FEL_RAVEN, spawnPos))
                        {
                            raven->SetInCombatWithZone();
                            ++m_RavenCount;
                        }
                        events.ScheduleEvent(EVENT_SPAWN_FEL_RAVEN, TIMER_SPAWN_FEL_RAVEN);
                        break;
                    }

                    // ── Periodic Corrupted Felblade Raven spawn ──────────────────
                    case EVENT_SPAWN_FELBLADE:
                    {
                        Position spawnPos;
                        me->GetRandomNearPosition(spawnPos, 20.0f);
                        if (Creature* felblade = me->SummonCreature(NPC_CORRUPTED_FELBLADE, spawnPos))
                            felblade->SetInCombatWithZone();
                        events.ScheduleEvent(EVENT_SPAWN_FELBLADE, TIMER_SPAWN_FELBLADE);
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
        return new boss_shadow_lord_iskarAI(creature);
    }
};

/// Fel Raven - 91263
class npc_fel_raven : public CreatureScript
{
public:
    npc_fel_raven() : CreatureScript("npc_fel_raven") { }

    struct npc_fel_ravenAI : public ScriptedAI
    {
        npc_fel_ravenAI(Creature* creature) : ScriptedAI(creature) { }

        void Reset() override
        {
            me->SetReactState(REACT_AGGRESSIVE);
            events.ScheduleEvent(EVENT_ADD_FEL_SLASH, TIMER_ADD_FEL_SLASH);
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
                    case EVENT_ADD_FEL_SLASH:
                    {
                        if (Unit* target = me->GetVictim())
                            me->CastSpell(target, SPELL_FEL_INCINERATION, false);
                        events.ScheduleEvent(EVENT_ADD_FEL_SLASH, TIMER_ADD_FEL_SLASH);
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
        return new npc_fel_ravenAI(creature);
    }
};

/// Corrupted Felblade Raven - 91266
class npc_corrupted_felblade_raven : public CreatureScript
{
public:
    npc_corrupted_felblade_raven() : CreatureScript("npc_corrupted_felblade_raven") { }

    struct npc_corrupted_felblade_ravenAI : public ScriptedAI
    {
        npc_corrupted_felblade_ravenAI(Creature* creature) : ScriptedAI(creature) { }

        void Reset() override
        {
            me->SetReactState(REACT_AGGRESSIVE);
            events.ScheduleEvent(EVENT_ADD_VOID_STATIC, TIMER_ADD_VOID_STATIC);
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
                    case EVENT_ADD_VOID_STATIC:
                    {
                        me->CastSpell(me, SPELL_CHORD_OF_SHADOWS, false);
                        events.ScheduleEvent(EVENT_ADD_VOID_STATIC, TIMER_ADD_VOID_STATIC);
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
        return new npc_corrupted_felblade_ravenAI(creature);
    }
};

/// Eye of Anzu - 179202
/// On expiry, notifies Iskar to transfer the Eye to a new random player.
class spell_eye_of_anzu : public SpellScriptLoader
{
public:
    spell_eye_of_anzu() : SpellScriptLoader("spell_eye_of_anzu") { }

    class spell_eye_of_anzu_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_eye_of_anzu_AuraScript);

        void HandleRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes mode)
        {
            // Only fire on natural expiry, not on explicit removal (AURA_REMOVE_BY_EXPIRE)
            if (GetTargetApplication()->GetRemoveMode() != AURA_REMOVE_BY_EXPIRE)
                return;

            Unit* target = GetTarget();
            if (!target)
                return;

            // Locate Iskar and trigger transfer
            std::list<Creature*> bossList;
            GetCreatureListWithEntryInGrid(bossList, target, eCitadelCreatures::BossIskar, 300.0f);
            for (Creature* boss : bossList)
            {
                if (boss && boss->IsAlive())
                {
                    boss->AI()->DoAction(ACTION_EYE_EXPIRED);
                    break;
                }
            }
        }

        void Register() override
        {
            OnEffectRemove += AuraEffectRemoveFn(spell_eye_of_anzu_AuraScript::HandleRemove, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
        }
    };

    AuraScript* GetAuraScript() const override { return new spell_eye_of_anzu_AuraScript(); }
};

void AddSC_boss_iskar_hfc()
{
    /// Boss
    new boss_shadow_lord_iskar();

    /// Creatures
    new npc_fel_raven();
    new npc_corrupted_felblade_raven();

    /// Spells
    new spell_eye_of_anzu();
}
