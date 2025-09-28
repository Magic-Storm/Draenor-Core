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
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
        }

        void EnterCombat(Unit* /*who*/) override
        {
            _EnterCombat();
            
            m_Phase = PHASE_NORMAL;
            m_VisionTimer = 60000; // 1 minute
            m_BloodsuckerTimer = 30000; // 30 seconds
            m_CorruptionTimer = 1000; // 1 second
            
            events.ScheduleEvent(EVENT_ARMOR_RIP, TIMER_ARMOR_RIP);
            events.ScheduleEvent(EVENT_HEARTSEEKER, TIMER_HEARTSEEKER);
            events.ScheduleEvent(EVENT_DEATH_THROES, TIMER_DEATH_THROES);
            events.ScheduleEvent(EVENT_VISION_OF_DEATH, TIMER_VISION_OF_DEATH);
            events.ScheduleEvent(EVENT_SPAWN_BLOODSUCKER, TIMER_SPAWN_BLOODSUCKER);
            events.ScheduleEvent(EVENT_UPDATE_CORRUPTION, TIMER_UPDATE_CORRUPTION);
        }

        void JustDied(Unit* /*killer*/) override
        {
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
                    player->RemoveAura(SPELL_CORRUPTION_SICKNESS);
                    player->RemoveAura(SPELL_DEMONIC_POSSESSION);
                }
            }
        }

        void KilledUnit(Unit* /*victim*/) override
        {
            // Optional: Add kill quotes or special handling
        }

        void UpdateAI(uint32 diff) override
        {
            if (!UpdateVictim())
                return;
            
            events.Update(diff);
            
            // Update timers
            if (m_VisionTimer <= diff)
            {
                CastVisionOfDeath();
                m_VisionTimer = 120000; // 2 minutes
            }
            else
                m_VisionTimer -= diff;
            
            if (m_BloodsuckerTimer <= diff)
            {
                SpawnBloodsucker();
                m_BloodsuckerTimer = 45000; // 45 seconds
            }
            else
                m_BloodsuckerTimer -= diff;
            
            if (m_CorruptionTimer <= diff)
            {
                UpdateCorruption();
                m_CorruptionTimer = 1000; // 1 second
            }
            else
                m_CorruptionTimer -= diff;
            
            // Handle events
            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_ARMOR_RIP:
                    {
                        if (Unit* target = SelectTarget(SELECT_TARGET_TOPAGGRO))
                        {
                            me->CastSpell(target, SPELL_ARMOR_RIP, false);
                        }
                        events.ScheduleEvent(EVENT_ARMOR_RIP, TIMER_ARMOR_RIP);
                        break;
                    }
                    case EVENT_HEARTSEEKER:
                    {
                        if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 50.0f))
                        {
                            me->CastSpell(target, SPELL_HEARTSEEKER, false);
                            
                            // Create blood pool at target location
                            if (Creature* bloodPool = me->SummonCreature(NPC_BLOOD_POOL, target->GetPositionX(), target->GetPositionY(), target->GetPositionZ(), 0, TEMPSUMMON_TIMED_DESPAWN, 30000))
                            {
                                m_BloodPools.push_back(bloodPool);
                                bloodPool->CastSpell(bloodPool, SPELL_BLOOD_POOL, true);
                            }
                        }
                        events.ScheduleEvent(EVENT_HEARTSEEKER, TIMER_HEARTSEEKER);
                        break;
                    }
                    case EVENT_DEATH_THROES:
                    {
                        if (m_Phase == PHASE_DEATH_THROES)
                        {
                            me->CastSpell(me, SPELL_DEATH_THROES, false);
                            events.ScheduleEvent(EVENT_DEATH_GATES, 2000);
                        }
                        events.ScheduleEvent(EVENT_DEATH_THROES, TIMER_DEATH_THROES);
                        break;
                    }
                    case EVENT_DEATH_GATES:
                    {
                        me->CastSpell(me, SPELL_DEATH_GATES, false);
                        break;
                    }
                    case EVENT_VISION_OF_DEATH:
                    {
                        CastVisionOfDeath();
                        events.ScheduleEvent(EVENT_VISION_OF_DEATH, TIMER_VISION_OF_DEATH);
                        break;
                    }
                    case EVENT_SPAWN_BLOODSUCKER:
                    {
                        SpawnBloodsucker();
                        events.ScheduleEvent(EVENT_SPAWN_BLOODSUCKER, TIMER_SPAWN_BLOODSUCKER);
                        break;
                    }
                    case EVENT_UPDATE_CORRUPTION:
                    {
                        UpdateCorruption();
                        events.ScheduleEvent(EVENT_UPDATE_CORRUPTION, TIMER_UPDATE_CORRUPTION);
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

        uint32 GetData(uint32 type) const override
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
            // Select random players for vision phase
            Map::PlayerList const& players = me->GetMap()->GetPlayers();
            std::vector<Player*> availablePlayers;
            
            for (Map::PlayerList::const_iterator itr = players.begin(); itr != players.end(); ++itr)
            {
                if (Player* player = itr->getSource())
                {
                    if (player->IsAlive() && !player->HasAura(SPELL_VISION_OF_DEATH))
                        availablePlayers.push_back(player);
                }
            }
            
            // Select 3-5 players for vision
            uint8 visionCount = std::min(5, (int)availablePlayers.size());
            if (visionCount == 0)
            {
                // No players available, deal damage to all
                me->CastSpell(me, SPELL_VISION_OF_DEATH, false);
                return;
            }
            
            // Randomly select players
            for (uint8 i = 0; i < visionCount; ++i)
            {
                uint32 randomIndex = urand(0, availablePlayers.size() - 1);
                Player* selectedPlayer = availablePlayers[randomIndex];
                availablePlayers.erase(availablePlayers.begin() + randomIndex);
                
                selectedPlayer->CastSpell(selectedPlayer, SPELL_VISION_OF_DEATH, true);
                m_VisionPlayers.push_back(selectedPlayer);
            }
            
            EnterPhase(PHASE_VISION_OF_DEATH);
        }

        void SpawnBloodsucker()
        {
            // Spawn bloodsucker near a random blood pool
            if (m_BloodPools.empty())
                return;
            
            Creature* targetPool = Trinity::Containers::SelectRandomContainerElement(m_BloodPools);
            if (!targetPool)
                return;
            
            Position spawnPos = targetPool->GetPosition();
            spawnPos.m_positionX += frand(-10.0f, 10.0f);
            spawnPos.m_positionY += frand(-10.0f, 10.0f);
            
            if (Creature* bloodsucker = me->SummonCreature(NPC_DROOLING_BLOODSUCKER, spawnPos))
            {
                bloodsucker->AI()->DoAction(ACTION_MOVE_TO_BLOOD);
            }
        }

        void UpdateCorruption()
        {
            Map::PlayerList const& players = me->GetMap()->GetPlayers();
            for (Map::PlayerList::const_iterator itr = players.begin(); itr != players.end(); ++itr)
            {
                if (Player* player = itr->getSource())
                {
                    if (player->IsAlive())
                    {
                        // Apply corruption sickness
                        if (Aura* corruption = player->GetAura(SPELL_CORRUPTION_SICKNESS))
                        {
                            if (corruption->GetStackAmount() >= 100)
                            {
                                player->RemoveAura(SPELL_CORRUPTION_SICKNESS);
                                player->CastSpell(player, SPELL_DEMONIC_POSSESSION, true);
                            }
                            else
                            {
                                player->CastSpell(player, SPELL_CORRUPTION_SICKNESS, true);
                            }
                        }
                        else
                        {
                            player->CastSpell(player, SPELL_CORRUPTION_SICKNESS, true);
                        }
                    }
                }
            }
        }

        void HandleBloodClotReach()
        {
            // Blood clot reached Kilrogg - deal damage and heal
            me->CastSpell(me, SPELL_CORRUPTED_BLOOD_FRENZY, false);
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
                
                Position demonPos = player->GetPosition();
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

// Drooling Bloodsucker - 93369
class npc_drooling_bloodsucker : public CreatureScript
{
public:
    npc_drooling_bloodsucker() : CreatureScript("npc_drooling_bloodsucker") { }

    struct npc_drooling_bloodsuckerAI : public ScriptedAI
    {
        npc_drooling_bloodsuckerAI(Creature* creature) : ScriptedAI(creature) { }

        void Reset() override
        {
            me->SetReactState(REACT_PASSIVE);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
            
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
            // Find nearest blood pool
            Creature* nearestPool = nullptr;
            float nearestDist = 999.0f;
            
            std::list<Creature*> bloodPools;
            GetCreatureListWithEntryInGrid(bloodPools, me, NPC_BLOOD_POOL, 100.0f);
            
            for (Creature* pool : bloodPools)
            {
                float dist = me->GetDistance(pool);
                if (dist < nearestDist)
                {
                    nearestDist = dist;
                    nearestPool = pool;
                }
            }
            
            if (nearestPool)
            {
                me->GetMotionMaster()->MovePoint(POINT_BLOOD_POOL, nearestPool->GetPositionX(), nearestPool->GetPositionY(), nearestPool->GetPositionZ());
            }
        }

        void TransformToHorror()
        {
            // Transform to massive horror
            if (Creature* horror = me->SummonCreature(NPC_MASSIVE_HORROR, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), me->GetOrientation()))
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

// Massive Horror - 90523
class npc_massive_horror : public CreatureScript
{
public:
    npc_massive_horror() : CreatureScript("npc_massive_horror") { }

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

        void JustDied(Unit* /*killer*/) override
        {
            // Give buff to killer if they're in vision phase
            if (Unit* killer = me->GetKiller())
            {
                if (Player* player = killer->ToPlayer())
                {
                    if (player->HasAura(SPELL_VISION_OF_DEATH))
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
class spell_kilrogg_vision_of_death : public SpellScript
{
    PrepareSpellScript(spell_kilrogg_vision_of_death);

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
        OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_kilrogg_vision_of_death::HandleTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENTRY);
    }
};

// 180372 - Heartseeker
class spell_kilrogg_heartseeker : public SpellScript
{
    PrepareSpellScript(spell_kilrogg_heartseeker);

    void HandleAfterCast()
    {
        Unit* caster = GetCaster();
        Unit* target = GetExplTargetUnit();
        
        if (!caster || !target)
            return;

        // Create blood pool at target location
        if (Creature* bloodPool = caster->SummonCreature(NPC_BLOOD_POOL, target->GetPositionX(), target->GetPositionY(), target->GetPositionZ(), 0, TEMPSUMMON_TIMED_DESPAWN, 30000))
        {
            bloodPool->CastSpell(bloodPool, SPELL_BLOOD_POOL, true);
        }
    }

    void Register() override
    {
        AfterCast += SpellCastFn(spell_kilrogg_heartseeker::HandleAfterCast);
    }
};

// 182159 - Corruption Sickness
class spell_kilrogg_corruption_sickness : public AuraScript
{
    PrepareAuraScript(spell_kilrogg_corruption_sickness);

    void HandlePeriodic(AuraEffect const* /*aurEff*/)
    {
        Unit* target = GetTarget();
        if (!target)
            return;

        // Add corruption stack
        if (Aura* corruption = target->GetAura(SPELL_CORRUPTION_SICKNESS))
        {
            if (corruption->GetStackAmount() >= 100)
            {
                target->RemoveAura(SPELL_CORRUPTION_SICKNESS);
                target->CastSpell(target, SPELL_DEMONIC_POSSESSION, true);
            }
        }
    }

    void Register() override
    {
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_kilrogg_corruption_sickness::HandlePeriodic, EFFECT_0, SPELL_AURA_PERIODIC_DAMAGE);
    }
};

// 180718 - Immortal Resolve
class spell_kilrogg_immortal_resolve : public AuraScript
{
    PrepareAuraScript(spell_kilrogg_immortal_resolve);

    void HandleApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        Unit* target = GetTarget();
        if (!target)
            return;

        // Apply damage increase
        target->ApplyStatPctModifier(UNIT_MOD_DAMAGE_MAINHAND, TOTAL_PCT, 10.0f);
    }

    void HandleRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        Unit* target = GetTarget();
        if (!target)
            return;

        // Remove damage increase
        target->ApplyStatPctModifier(UNIT_MOD_DAMAGE_MAINHAND, TOTAL_PCT, -10.0f);
    }

    void Register() override
    {
        OnEffectApply += AuraEffectApplyFn(spell_kilrogg_immortal_resolve::HandleApply, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
        OnEffectRemove += AuraEffectRemoveFn(spell_kilrogg_immortal_resolve::HandleRemove, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
    }
};

// 185563 - Immortal Salvation
class spell_kilrogg_immortal_salvation : public AuraScript
{
    PrepareAuraScript(spell_kilrogg_immortal_salvation);

    void HandleApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        Unit* target = GetTarget();
        if (!target)
            return;

        // Apply healing increase
        target->ApplyStatPctModifier(UNIT_MOD_HEALING_DONE_POSITIVE, TOTAL_PCT, 10.0f);
    }

    void HandleRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        Unit* target = GetTarget();
        if (!target)
            return;

        // Remove healing increase
        target->ApplyStatPctModifier(UNIT_MOD_HEALING_DONE_POSITIVE, TOTAL_PCT, -10.0f);
    }

    void Register() override
    {
        OnEffectApply += AuraEffectApplyFn(spell_kilrogg_immortal_salvation::HandleApply, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
        OnEffectRemove += AuraEffectRemoveFn(spell_kilrogg_immortal_salvation::HandleRemove, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
    }
};

// 187089 - Cleansing Aura
class spell_kilrogg_cleansing_aura : public AuraScript
{
    PrepareAuraScript(spell_kilrogg_cleansing_aura);

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
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_kilrogg_cleansing_aura::HandlePeriodic, EFFECT_0, SPELL_AURA_DUMMY);
    }
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