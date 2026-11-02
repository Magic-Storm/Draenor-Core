///////////////////////////////////////////////////////////////////////////////
///
///  MILLENIUM-STUDIO
///  Copyright 2016 Millenium-studio SARL
///  All Rights Reserved.
///
///////////////////////////////////////////////////////////////////////////////

#include "boss_kilrog_hfc.h"
#include "hellfire_citadel.h"

/// Kilrogg Dead Eye - 90378
class boss_kilrogg_deadeye : public CreatureScript
{
public:
    boss_kilrogg_deadeye() : CreatureScript("boss_kilrogg_deadeye") { }

    enum eTalks
    {
        TalkAggro = 0,
        TalkSlay,
        TalkDeath,
        TalkAbility,
        TalkWipe
    };

    struct boss_kilrogg_deadeyeAI : public BossAI
    {
        boss_kilrogg_deadeyeAI(Creature* creature) : BossAI(creature, eCitadelDatas::DataKilrogg)
        {
            m_Instance = creature->GetInstanceScript();
        }

        InstanceScript* m_Instance;
        uint8 m_Phase;
        uint32 m_VisionTimer;
        uint32 m_BloodsuckerTimer;
        uint32 m_CorruptionTimer;
        std::list<Player*> m_VisionPlayers;
        std::list<Creature*> m_BloodPools;
        std::list<Creature*> m_ActiveHorrors;

        void Reset() override
        {
            _Reset();
            
            m_Phase = PHASE_NORMAL;
            m_VisionTimer = 0;
            m_BloodsuckerTimer = 0;
            m_CorruptionTimer = 0;
            m_VisionPlayers.clear();
            m_BloodPools.clear();
            m_ActiveHorrors.clear();
            
            me->SetReactState(REACT_AGGRESSIVE);
            me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
        }

        void EnterCombat(Unit* /*who*/) override
        {
            _EnterCombat();
            Talk(eTalks::TalkAggro);

            me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
            
            m_Phase = PHASE_NORMAL;

            /// Death Throes is a repeating cast from the start of the fight, not an execute phase.
            events.ScheduleEvent(EVENT_ARMOR_RIP, TIMER_ARMOR_RIP);
            events.ScheduleEvent(EVENT_HEARTSEEKER, TIMER_HEARTSEEKER);
            events.ScheduleEvent(EVENT_DEATH_THROES, TIMER_DEATH_THROES);
            events.ScheduleEvent(EVENT_VISION_OF_DEATH, TIMER_VISION_OF_DEATH);
            events.ScheduleEvent(EVENT_SPAWN_BLOODSUCKER, TIMER_SPAWN_BLOODSUCKER);
        }

        void KilledUnit(Unit* p_Who) override
        {
            if (p_Who && p_Who->IsPlayer())
                Talk(eTalks::TalkSlay);
        }

        void JustDied(Unit* /*killer*/) override
        {
            Talk(eTalks::TalkDeath);
            _JustDied();
            
            // Clean up all adds
            DespawnCreaturesInArea(NPC_DROOLING_BLOODSUCKER, me);
            DespawnCreaturesInArea(NPC_MASSIVE_HORROR, me);
            DespawnCreaturesInArea(NPC_HELLFIRE_GRUNT, me);
            DespawnCreaturesInArea(NPC_HELLFIRE_BRUTE, me);
            DespawnCreaturesInArea(NPC_HELLFIRE_MISTRESS, me);
            
            // Remove all corruption from players
            Map::PlayerList const& players = me->GetMap()->GetPlayers();
            for (Map::PlayerList::const_iterator itr = players.begin(); itr != players.end(); ++itr)
            {
                if (Player* player = itr->getSource())
                {
                    player->RemoveAura(SPELL_FEL_CORRUPTION);
                    player->RemoveAura(SPELL_VISION_OF_DEATH);
                    player->RemoveAura(SPELL_VISION_OF_DEATH_AURA);
                }
            }
        }

        void UpdateAI(uint32 diff) override
        {
            if (!UpdateVictim())
                return;
            
            events.Update(diff);

            // Handle events
            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_ARMOR_RIP:
                    {
                        if (Unit* target = SelectTarget(SELECT_TARGET_TOPAGGRO))
                        {
                            me->CastSpell(target, SPELL_SHRED_ARMOR, false);
                        }
                        events.ScheduleEvent(EVENT_ARMOR_RIP, TIMER_ARMOR_RIP_CD);
                        break;
                    }
                    case EVENT_HEARTSEEKER:
                    {
                        if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 50.0f, true))
                            me->CastSpell(target, SPELL_HEARTSEEKER, false);
                        events.ScheduleEvent(EVENT_HEARTSEEKER, TIMER_HEARTSEEKER_CD);
                        break;
                    }
                    case EVENT_DEATH_THROES:
                    {
                        me->CastSpell(me, SPELL_DEATH_THROES, false);
                        events.ScheduleEvent(EVENT_DEATH_THROES, TIMER_DEATH_THROES);
                        break;
                    }
                    case EVENT_VISION_OF_DEATH:
                    {
                        CastVisionOfDeath();
                        events.ScheduleEvent(EVENT_VISION_OF_DEATH, TIMER_VISION_OF_DEATH_CD);
                        break;
                    }
                    case EVENT_SPAWN_BLOODSUCKER:
                    {
                        SpawnBloodthirsters();
                        events.ScheduleEvent(EVENT_SPAWN_BLOODSUCKER, TIMER_SPAWN_BLOODSUCKER_CD);
                        break;
                    }
                }
            }
            
            DoMeleeAttackIfReady();
        }

        void DoAction(int32 action) override
        {
            switch (action)
            {
                case ACTION_START_VISION_PHASE:
                    HandleVisionPhase();
                    break;
                case ACTION_END_VISION_PHASE:
                    EndVisionPhase();
                    break;
                case ACTION_BLOOD_CLOT_REACHED:
                    HandleBloodClotReach();
                    break;
            }
        }

        uint32 GetData(uint32 type) const
        {
            switch (type)
            {
                case DATA_VISION_PLAYERS:
                    return m_VisionPlayers.size();
                case DATA_BLOOD_POOLS:
                    return m_BloodPools.size();
                case DATA_ACTIVE_HORRORS:
                    return m_ActiveHorrors.size();
                case DATA_CORRUPTION_LEVEL:
                    return m_Phase;
                default:
                    return 0;
            }
        }

        void SetData(uint32 type, uint32 data) override
        {
            switch (type)
            {
                case DATA_VISION_PLAYERS:
                    if (data == 0)
                        m_VisionPlayers.clear();
                    break;
                case DATA_BLOOD_POOLS:
                    if (data == 0)
                        m_BloodPools.clear();
                    break;
                case DATA_ACTIVE_HORRORS:
                    if (data == 0)
                        m_ActiveHorrors.clear();
                    break;
            }
        }

        void EnterPhase(uint8 phase)
        {
            m_Phase = phase;
            
            switch (phase)
            {
                case PHASE_NORMAL:
                    me->SetReactState(REACT_AGGRESSIVE);
                    break;
                case PHASE_VISION_OF_DEATH:
                    me->SetReactState(REACT_PASSIVE);
                    me->AttackStop();
                    break;
                case PHASE_DEATH_THROES:
                    me->SetReactState(REACT_AGGRESSIVE);
                    break;
            }
        }

        void CastVisionOfDeath()
        {
            /// Kilrogg keeps fighting. The spell itself sends a few players into the vision.
            /// Stopping him here used to freeze the encounter for everyone else.
            me->CastSpell(me, SPELL_VISION_OF_DEATH, false);
        }

        void SpawnBloodthirsters()
        {
            /// Three Salivating Bloodthirsters. One appears on the fel pool and transforms immediately.
            /// The other two walk in from the sides and must be killed before they arrive.
            /// Wowhead Kilrogg guide / BigWigs add warnings.
            Position l_Pool;
            me->GetNearPosition(l_Pool, 25.0f, 0.0f);

            if (Creature* l_Immediate = me->SummonCreature(NPC_HULKING_TERROR, l_Pool, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 30000))
                l_Immediate->SetInCombatWithZone();

            float l_Side[2] = { -2.2f, 2.2f };
            for (uint8 i = 0; i < 2; ++i)
            {
                Position l_Spawn;
                me->GetNearPosition(l_Spawn, 18.0f, l_Side[i]);
                if (Creature* l_Walker = me->SummonCreature(NPC_SALIVATING_BLOODTHIRSTER, l_Spawn, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 5000))
                    l_Walker->AI()->DoAction(ACTION_MOVE_TO_BLOOD);
            }
        }

        void HandleBloodClotReach()
        {
        }

        void HandleVisionPhase()
        {
            // Spawn demons for each player in vision
            for (std::list<Player*>::iterator itr = m_VisionPlayers.begin(); itr != m_VisionPlayers.end(); ++itr)
            {
                Player* player = *itr;
                if (!player || !player->IsAlive())
                    continue;
                
                // Spawn random demons
                uint32 demonEntry = 0;
                switch (urand(0, 2))
                {
                    case 0: demonEntry = NPC_HELLFIRE_GRUNT; break;
                    case 1: demonEntry = NPC_HELLFIRE_BRUTE; break;
                    case 2: demonEntry = NPC_HELLFIRE_MISTRESS; break;
                }
                
                Position demonPos = *player;
                demonPos.m_positionX += frand(-5.0f, 5.0f);
                demonPos.m_positionY += frand(-5.0f, 5.0f);
                
                if (Creature* demon = me->SummonCreature(demonEntry, demonPos))
                {
                    demon->SetInCombatWith(player);
                    demon->AddThreat(player, 1000.0f);
                }
            }
        }

        void EndVisionPhase()
        {
            // Give buffs to players who killed demons
            for (std::list<Player*>::iterator itr = m_VisionPlayers.begin(); itr != m_VisionPlayers.end(); ++itr)
            {
                Player* player = *itr;
                if (!player || !player->IsAlive())
                    continue;
                
                // Apply appropriate buffs based on role
                if (player->GetRoleForGroup() == ROLE_DAMAGE)
                    player->CastSpell(player, SPELL_IMMORTAL_RESOLVE, true);
                else if (player->GetRoleForGroup() == ROLE_HEALER)
                {
                    player->CastSpell(player, SPELL_IMMORTAL_SALVATION, true);
                    player->CastSpell(player, SPELL_CLEANSING_AURA, true);
                }
            }
            
            m_VisionPlayers.clear();
            EnterPhase(PHASE_NORMAL);
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new boss_kilrogg_deadeyeAI(creature);
    }
};

// Salivating Bloodthirster - 90521. Walks to the fel pool.
class npc_drooling_bloodsucker : public CreatureScript
{
public:
    npc_drooling_bloodsucker() : CreatureScript("npc_drooling_bloodsucker") { }

    enum eTalks
    {
        TalkAggro = 0,
        TalkSlay,
        TalkDeath,
        TalkAbility,
        TalkWipe
    };

    struct npc_drooling_bloodsuckerAI : public ScriptedAI
    {
        npc_drooling_bloodsuckerAI(Creature* creature) : ScriptedAI(creature) { }

        void Reset() override
        {
            /// Passive so it only walks, but damageable. Players are supposed to kill it
            /// before it reaches the fel pool. It cannot be stunned (Wowhead).
            me->SetReactState(REACT_PASSIVE);
            me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_STUN, true);
            me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_FREEZE, true);
            events.ScheduleEvent(EVENT_MOVE_TO_BLOOD, 1000);
        }

        void MovementInform(uint32 type, uint32 pointId) override
        {
            if (type == POINT_MOTION_TYPE && pointId == POINT_BLOOD_POOL)
            {
                TransformToHorror();
            }
        }

        void UpdateAI(uint32 diff) override
        {
            events.Update(diff);
            
            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_MOVE_TO_BLOOD:
                        MoveToBloodPool();
                        break;
                }
            }
        }

        void MoveToBloodPool()
        {
            /// The fel pool is in front of Kilrogg. There is no separate pool NPC on retail.
            Unit* l_Owner = me->GetOwner();
            if (!l_Owner)
            {
                if (TempSummon* l_Summon = me->ToTempSummon())
                    l_Owner = l_Summon->GetSummoner();
            }

            if (!l_Owner)
                return;

            Position l_Dest;
            l_Owner->GetNearPosition(l_Dest, 25.0f, 0.0f);
            me->GetMotionMaster()->MovePoint(POINT_BLOOD_POOL, l_Dest);
        }

        void TransformToHorror()
        {
            // Transform to massive horror
            me->CastSpell(me, SPELL_BLOODTHIRSTER_TRANSFORM, true);

            if (Creature* horror = me->SummonCreature(NPC_HULKING_TERROR, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), me->GetOrientation(), TEMPSUMMON_CORPSE_TIMED_DESPAWN, 30000))
            {
                horror->SetInCombatWithZone();
            }
            
            me->DespawnOrUnsummon();
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_drooling_bloodsuckerAI(creature);
    }
};

// Hulking Terror - 93369 (90523 on some difficulties)
class npc_massive_horror : public CreatureScript
{
public:
    npc_massive_horror() : CreatureScript("npc_massive_horror") { }

    enum eTalks
    {
        TalkAggro = 0,
        TalkSlay,
        TalkDeath,
        TalkAbility,
        TalkWipe
    };

    struct npc_massive_horrorAI : public ScriptedAI
    {
        npc_massive_horrorAI(Creature* creature) : ScriptedAI(creature) { }

        void Reset() override
        {
            me->SetReactState(REACT_AGGRESSIVE);
            me->SetInCombatWithZone();
            
            events.ScheduleEvent(EVENT_FRENZIED_STRIKES, TIMER_FRENZIED_STRIKES);
            events.ScheduleEvent(EVENT_SHATTERING_SCREAM, TIMER_SHATTERING_SCREAM);
        }

        void JustDied(Unit* /*killer*/) override
        {
            CastFelRift();
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
                    case EVENT_FRENZIED_STRIKES:
                        me->CastSpell(me->GetVictim(), SPELL_FRENZIED_STRIKES, false);
                        events.ScheduleEvent(EVENT_FRENZIED_STRIKES, TIMER_FRENZIED_STRIKES);
                        break;
                    case EVENT_SHATTERING_SCREAM:
                        me->CastSpell(me, SPELL_SHATTERING_SCREAM, false);
                        events.ScheduleEvent(EVENT_SHATTERING_SCREAM, TIMER_SHATTERING_SCREAM);
                        break;
                }
            }
            
            DoMeleeAttackIfReady();
        }

        void CastFelRift()
        {
            me->CastSpell(me, SPELL_FEL_RIFT, false);
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_massive_horrorAI(creature);
    }
};

// Vision of Death Demon - 90407, 90411, 90414
class npc_vision_of_death_demon : public CreatureScript
{
public:
    npc_vision_of_death_demon() : CreatureScript("npc_vision_of_death_demon") { }

    enum eTalks
    {
        TalkAggro = 0,
        TalkSlay,
        TalkDeath,
        TalkAbility,
        TalkWipe
    };

    struct npc_vision_of_death_demonAI : public ScriptedAI
    {
        npc_vision_of_death_demonAI(Creature* creature) : ScriptedAI(creature) { }

        void Reset() override
        {
            me->SetReactState(REACT_AGGRESSIVE);
            me->SetInCombatWithZone();
            
            // Schedule appropriate ability based on demon type
            switch (me->GetEntry())
            {
                case NPC_HELLFIRE_GRUNT:
                    events.ScheduleEvent(EVENT_FEL_FIRE, TIMER_FEL_FIRE);
                    break;
                case NPC_HELLFIRE_BRUTE:
                    events.ScheduleEvent(EVENT_FEL_FLAME, TIMER_FEL_FLAME);
                    break;
                case NPC_HELLFIRE_MISTRESS:
                    events.ScheduleEvent(EVENT_SEARING_BREATH, TIMER_SEARING_BREATH);
                    break;
            }
        }

        void JustDied(Unit* killer) override
        {
            // Give buff to killer if they're in vision phase
            if (killer)
            {
                if (Player* player = killer->ToPlayer())
                {
                    if (player->HasAura(SPELL_VISION_OF_DEATH) || player->HasAura(SPELL_VISION_OF_DEATH_AURA))
                    {
                        // Apply appropriate buff based on role
                        if (player->GetRoleForGroup() == ROLE_DAMAGE)
                            player->CastSpell(player, SPELL_IMMORTAL_RESOLVE, true);
                        else if (player->GetRoleForGroup() == ROLE_HEALER)
                        {
                            player->CastSpell(player, SPELL_IMMORTAL_SALVATION, true);
                            player->CastSpell(player, SPELL_CLEANSING_AURA, true);
                        }
                    }
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
                    case EVENT_FEL_FIRE:
                        me->CastSpell(me->GetVictim(), SPELL_FEL_FIRE, false);
                        events.ScheduleEvent(EVENT_FEL_FIRE, TIMER_FEL_FIRE);
                        break;
                    case EVENT_FEL_FLAME:
                        me->CastSpell(me->GetVictim(), SPELL_FEL_FLAME, false);
                        events.ScheduleEvent(EVENT_FEL_FLAME, TIMER_FEL_FLAME);
                        break;
                    case EVENT_SEARING_BREATH:
                        me->CastSpell(me->GetVictim(), SPELL_SEARING_BREATH, false);
                        events.ScheduleEvent(EVENT_SEARING_BREATH, TIMER_SEARING_BREATH);
                        break;
                }
            }
            
            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_vision_of_death_demonAI(creature);
    }
};

// Spell Scripts

// 182428 - Vision of Death
class spell_kilrogg_vision_of_death : public SpellScriptLoader
{
public:
    spell_kilrogg_vision_of_death() : SpellScriptLoader("spell_kilrogg_vision_of_death") { }

    class spell_kilrogg_vision_of_death_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_kilrogg_vision_of_death_SpellScript);

        void HandleTargets(std::list<WorldObject*>& targets)
        {
            if (targets.empty())
                return;

            Unit* caster = GetCaster();
            if (!caster)
                return;

            // Remove non-players and dead players
            targets.remove_if([](WorldObject* target) -> bool
            {
                if (!target->IsPlayer())
                    return true;
                
                Player* player = target->ToPlayer();
                return !player || !player->IsAlive() || player->HasAura(SPELL_VISION_OF_DEATH);
            });

            // Limit to 3-5 players
            if (targets.size() > 5)
            {
                targets.resize(5);
            }
        }

        void Register() override
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_kilrogg_vision_of_death_SpellScript::HandleTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENTRY);
        }
    };

    SpellScript* GetSpellScript() const override { return new spell_kilrogg_vision_of_death_SpellScript(); }
};

// 180372 - Heartseeker
class spell_kilrogg_heartseeker : public SpellScriptLoader
{
public:
    spell_kilrogg_heartseeker() : SpellScriptLoader("spell_kilrogg_heartseeker") { }

    class spell_kilrogg_heartseeker_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_kilrogg_heartseeker_SpellScript);

        void HandleAfterCast()
        {
            Unit* caster = GetCaster();
            Unit* target = GetExplTargetUnit();
            
            if (!caster || !target)
                return;

            /// Heart Seeker leaves a Blood Globule, which splatters fel blood on the ground.
            if (Creature* l_Globule = caster->SummonCreature(NPC_BLOOD_GLOBULE, target->GetPositionX(), target->GetPositionY(), target->GetPositionZ(), 0.0f, TEMPSUMMON_TIMED_DESPAWN, 30000))
                l_Globule->CastSpell(l_Globule, SPELL_BLOOD_SPLATTER, true);
        }

        void Register() override
        {
            AfterCast += SpellCastFn(spell_kilrogg_heartseeker_SpellScript::HandleAfterCast);
        }
    };

    SpellScript* GetSpellScript() const override { return new spell_kilrogg_heartseeker_SpellScript(); }
};

// 182159 - Corruption Sickness
class spell_kilrogg_corruption_sickness : public SpellScriptLoader
{
public:
    spell_kilrogg_corruption_sickness() : SpellScriptLoader("spell_kilrogg_corruption_sickness") { }

    class spell_kilrogg_corruption_sickness_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_kilrogg_corruption_sickness_AuraScript);

        void HandlePeriodic(AuraEffect const* /*aurEff*/)
        {
            Unit* target = GetTarget();
            if (!target)
                return;

            /// Fel Corruption is an alternate-power bar (182159). Reaching 100 mind-controls
            /// the player through the spell itself. Do not strip it and apply a stand-in aura.
        }

        void Register() override
        {
            OnEffectPeriodic += AuraEffectPeriodicFn(spell_kilrogg_corruption_sickness_AuraScript::HandlePeriodic, EFFECT_0, SPELL_AURA_PERIODIC_DAMAGE);
        }
    };

    AuraScript* GetAuraScript() const override { return new spell_kilrogg_corruption_sickness_AuraScript(); }
};

// 180718 - Immortal Resolve
class spell_kilrogg_immortal_resolve : public SpellScriptLoader
{
public:
    spell_kilrogg_immortal_resolve() : SpellScriptLoader("spell_kilrogg_immortal_resolve") { }

    class spell_kilrogg_immortal_resolve_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_kilrogg_immortal_resolve_AuraScript);

        void HandleApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
        {
            Unit* target = GetTarget();
            if (!target)
                return;

            // Apply damage increase
            // target->ApplyStatPctModifier(UNIT_MOD_DAMAGE_MAINHAND, TOTAL_PCT, 10.0f);
        }

        void HandleRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
        {
            Unit* target = GetTarget();
            if (!target)
                return;

            // Remove damage increase
            // target->ApplyStatPctModifier(UNIT_MOD_DAMAGE_MAINHAND, TOTAL_PCT, -10.0f);
        }

        void Register() override
        {
            OnEffectApply += AuraEffectApplyFn(spell_kilrogg_immortal_resolve_AuraScript::HandleApply, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
            OnEffectRemove += AuraEffectRemoveFn(spell_kilrogg_immortal_resolve_AuraScript::HandleRemove, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
        }
    };

    AuraScript* GetAuraScript() const override { return new spell_kilrogg_immortal_resolve_AuraScript(); }
};

// 185563 - Immortal Salvation
class spell_kilrogg_immortal_salvation : public SpellScriptLoader
{
public:
    spell_kilrogg_immortal_salvation() : SpellScriptLoader("spell_kilrogg_immortal_salvation") { }

    class spell_kilrogg_immortal_salvation_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_kilrogg_immortal_salvation_AuraScript);

        void HandleApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
        {
            Unit* target = GetTarget();
            if (!target)
                return;

            // Apply healing increase
            // target->ApplyStatPctModifier(UNIT_MOD_HEALING_DONE_POSITIVE, TOTAL_PCT, 10.0f);
        }

        void HandleRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
        {
            Unit* target = GetTarget();
            if (!target)
                return;

            // Remove healing increase
            // target->ApplyStatPctModifier(UNIT_MOD_HEALING_DONE_POSITIVE, TOTAL_PCT, -10.0f);
        }

        void Register() override
        {
            OnEffectApply += AuraEffectApplyFn(spell_kilrogg_immortal_salvation_AuraScript::HandleApply, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
            OnEffectRemove += AuraEffectRemoveFn(spell_kilrogg_immortal_salvation_AuraScript::HandleRemove, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
        }
    };

    AuraScript* GetAuraScript() const override { return new spell_kilrogg_immortal_salvation_AuraScript(); }
};

// 187089 - Cleansing Aura
class spell_kilrogg_cleansing_aura : public SpellScriptLoader
{
public:
    spell_kilrogg_cleansing_aura() : SpellScriptLoader("spell_kilrogg_cleansing_aura") { }

    class spell_kilrogg_cleansing_aura_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_kilrogg_cleansing_aura_AuraScript);

        void HandlePeriodic(AuraEffect const* /*aurEff*/)
        {
            Unit* target = GetTarget();
            if (!target)
                return;

            // Reduce corruption on nearby allies
            std::list<Unit*> allies;
            Trinity::AnyFriendlyUnitInObjectRangeCheck u_check(target, target, 10.0f);
            Trinity::UnitListSearcher<Trinity::AnyFriendlyUnitInObjectRangeCheck> searcher(target, allies, u_check);
            target->VisitNearbyObject(10.0f, searcher);

            for (Unit* ally : allies)
            {
                if (Aura* corruption = ally->GetAura(SPELL_CORRUPTION_SICKNESS))
                {
                    if (corruption->GetStackAmount() > 1)
                    {
                        corruption->ModStackAmount(-1);
                    }
                }
            }
        }

        void Register() override
        {
            OnEffectPeriodic += AuraEffectPeriodicFn(spell_kilrogg_cleansing_aura_AuraScript::HandlePeriodic, EFFECT_0, SPELL_AURA_DUMMY);
        }
    };

    AuraScript* GetAuraScript() const override { return new spell_kilrogg_cleansing_aura_AuraScript(); }
};

void AddSC_boss_kilrog_hfc()
{
    /// Boss
    new boss_kilrogg_deadeye();

    /// Creatures
    new npc_drooling_bloodsucker();
    new npc_massive_horror();
    new npc_vision_of_death_demon();

    /// Spells
    new spell_kilrogg_vision_of_death();
    new spell_kilrogg_heartseeker();
    new spell_kilrogg_corruption_sickness();
    new spell_kilrogg_immortal_resolve();
    new spell_kilrogg_immortal_salvation();
    new spell_kilrogg_cleansing_aura();
}