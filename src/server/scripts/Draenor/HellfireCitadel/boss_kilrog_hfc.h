///////////////////////////////////////////////////////////////////////////////
///
///  MILLENIUM-STUDIO
///  Copyright 2016 Millenium-studio SARL
///  All Rights Reserved.
///
///////////////////////////////////////////////////////////////////////////////

#ifndef BOSS_KILROG_HFC_HPP
#define BOSS_KILROG_HFC_HPP

#include "hellfire_citadel.h"

enum eKilroggSpells
{
    /// Boss. IDs from Wowhead / BigWigs KilroggDeadeye.lua.
    /// 90477 and 96077 are creature entries (Blood Globule), not spells.
    SPELL_FEL_CORRUPTION = 182159,                ///< Alternate power. The encounter applies it; do not stack it by hand.
    SPELL_SHRED_ARMOR = 180199,                   ///< Tank hit. Debuff if it lands is 180200.
    SPELL_SHREDDED_ARMOR = 180200,
    SPELL_HEARTSEEKER = 180372,                   ///< Spear. The player debuff is 188929.
    SPELL_HEARTSEEKER_DEBUFF = 188929,
    SPELL_BLOOD_SPLATTER = 188852,                ///< Ground damage left by a Blood Globule
    SPELL_DEATH_THROES = 180224,                  ///< Repeating raid cast, not a 30% phase
    SPELL_VISION_OF_DEATH = 182428,
    SPELL_VISION_OF_DEATH_AURA = 185458,          ///< Buff on players who are inside the vision
    SPELL_IMMORTAL_RESOLVE = 180718,
    SPELL_IMMORTAL_SALVATION = 185563,
    SPELL_CLEANSING_AURA = 187089,
    SPELL_BLOODTHIRSTER_TRANSFORM = 182012,       ///< Cast as a Bloodthirster reaches the pool

    /// Hulking Terror
    SPELL_SAVAGE_STRIKES = 180163,                ///< Cone. Was mislabeled Frenzied Strikes.
    SPELL_RENDING_HOWL = 183917,                  ///< Interruptible raid damage. Was mislabeled Shattering Scream.
    SPELL_FEL_RUPTURE = 180243,                   ///< Death burst. Wowhead spell=180243.

    /// Vision of Death demons
    SPELL_FEL_BLAZE = 180618,                     ///< Hellblaze Imp
    SPELL_FEL_FLAMES = 180570,                    ///< Hellblaze Fiend
    SPELL_CINDER_BREATH = 180033,                 ///< Hellblaze Mistress

    /// Old names kept so existing call sites stay valid.
    SPELL_ARMOR_RIP = SPELL_SHRED_ARMOR,
    SPELL_FRENZIED_STRIKES = SPELL_SAVAGE_STRIKES,
    SPELL_SHATTERING_SCREAM = SPELL_RENDING_HOWL,
    SPELL_FEL_RIFT = SPELL_FEL_RUPTURE,
    SPELL_FEL_FIRE = SPELL_FEL_BLAZE,
    SPELL_FEL_FLAME = SPELL_FEL_FLAMES,
    SPELL_SEARING_BREATH = SPELL_CINDER_BREATH,
    SPELL_CORRUPTION_SICKNESS = SPELL_FEL_CORRUPTION,
    SPELL_VISION_OF_DEATH_CHECK = SPELL_VISION_OF_DEATH,
};

enum eKilroggCreatures
{
    /// Live encounter-frame IDs (BigWigs INSTANCE_ENCOUNTER_ENGAGE_UNIT).
    NPC_SALIVATING_BLOODTHIRSTER = 90521,         ///< Walks to the fel pool. Kill it before it arrives.
    NPC_HULKING_TERROR = 93369,                   ///< What a Bloodthirster becomes in the pool.
    NPC_BLOOD_GLOBULE = 90477,                    ///< Spawned by Heart Seeker. Not a spell.
    NPC_FEL_BLOOD_GLOBULE = 90513,
    NPC_HELLFIRE_GRUNT = 90407,
    NPC_HELLFIRE_BRUTE = 90411,
    NPC_HELLFIRE_MISTRESS = 90414,

    NPC_DROOLING_BLOODSUCKER = NPC_SALIVATING_BLOODTHIRSTER,
    NPC_MASSIVE_HORROR = NPC_HULKING_TERROR,
};

enum eKilroggEvents
{
    EVENT_ARMOR_RIP = 1,
    EVENT_HEARTSEEKER,
    EVENT_DEATH_THROES,
    EVENT_VISION_OF_DEATH,
    EVENT_SPAWN_BLOODSUCKER,
    EVENT_UPDATE_CORRUPTION,
    EVENT_DEATH_GATES,
    
    // Add Events
    EVENT_FRENZIED_STRIKES,
    EVENT_SHATTERING_SCREAM,
    EVENT_FEL_RIFT,
    EVENT_MOVE_TO_BLOOD,
    EVENT_TRANSFORM_TO_HORROR,
    
    // Demon Events
    EVENT_FEL_FIRE,
    EVENT_FEL_FLAME,
    EVENT_SEARING_BREATH,
};

enum eKilroggPhases
{
    PHASE_NORMAL = 1,
    PHASE_VISION_OF_DEATH,
    PHASE_DEATH_THROES,
};

enum eKilroggActions
{
    ACTION_START_VISION_PHASE = 1,
    ACTION_END_VISION_PHASE,
    ACTION_BLOOD_CLOT_REACHED,
    ACTION_MOVE_TO_BLOOD,
    ACTION_TRANSFORM_TO_HORROR,
};

enum eKilroggData
{
    DATA_VISION_PLAYERS = 1,
    DATA_BLOOD_POOLS,
    DATA_ACTIVE_HORRORS,
    DATA_CORRUPTION_LEVEL,
};

enum eKilroggTimers
{
    /// BigWigs OnEngage: Shred Armor 10.8s then 17s, Heart Seeker 25s,
    /// Death Throes 40s, Vision of Death 60s then ~75s, Bloodthirsters 15s then ~70s.
    TIMER_ARMOR_RIP = 11000,
    TIMER_ARMOR_RIP_CD = 17000,
    TIMER_HEARTSEEKER = 25000,
    TIMER_HEARTSEEKER_CD = 26000,
    TIMER_DEATH_THROES = 40000,
    TIMER_VISION_OF_DEATH = 60000,
    TIMER_VISION_OF_DEATH_CD = 75000,
    TIMER_SPAWN_BLOODSUCKER = 15000,
    TIMER_SPAWN_BLOODSUCKER_CD = 70000,
    TIMER_FRENZIED_STRIKES = 6000,
    TIMER_SHATTERING_SCREAM = 10000,
    TIMER_FEL_FIRE = 10000,
    TIMER_FEL_FLAME = 5000,
    TIMER_SEARING_BREATH = 4500,
};

enum eKilroggPoints
{
    POINT_BLOOD_POOL = 1,
};



#endif // BOSS_KILROG_HFC_HPP
