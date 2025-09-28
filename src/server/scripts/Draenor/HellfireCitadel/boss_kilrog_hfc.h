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
    // Boss Abilities
    SPELL_CORRUPTION_SICKNESS = 182159,           // Corruption Sickness - Stacking debuff
    SPELL_DEMONIC_POSSESSION = 180313,            // Demonic Possession - Mind control at 100 stacks
    SPELL_ARMOR_RIP = 180199,                     // Armor Rip - Physical damage + vulnerability
    SPELL_HEARTSEEKER = 180372,                   // Heartseeker - Line damage + DoT
    SPELL_BLOOD_POOL = 188852,                    // Blood Pool - Ground effect damage
    SPELL_BLOOD_CLOT = 90477,                     // Blood Clot - Moving add
    SPELL_BLOOD_FRENZY = 180127,                  // Blood Frenzy - Raid damage
    SPELL_CORRUPTED_BLOOD_CLOT = 96077,           // Corrupted Blood Clot - Moving add
    SPELL_CORRUPTED_BLOOD_FRENZY = 180143,        // Corrupted Blood Frenzy - Raid damage + heal
    SPELL_DEATH_THROES = 180224,                  // Death Throes - Raid damage + projectiles
    SPELL_DEATH_GATES = 184551,                   // Death Gates - Damage increase
    SPELL_VISION_OF_DEATH = 182428,               // Vision of Death - Phase transition
    SPELL_IMMORTAL_RESOLVE = 180718,              // Immortal Resolve - DPS buff
    SPELL_IMMORTAL_SALVATION = 185563,            // Immortal Salvation - Healer buff
    SPELL_CLEANSING_AURA = 187089,                // Cleansing Aura - Corruption reduction
    
    // Add Abilities
    SPELL_FRENZIED_STRIKES = 180163,              // Frenzied Strikes - Horror cleave
    SPELL_FEL_CLAWS = 182153,                     // Fel Claws - Corruption application
    SPELL_SHATTERING_SCREAM = 183917,             // Shattering Scream - Interruptible AoE
    SPELL_FEL_RIFT = 180243,                      // Fel Rift - Death explosion
    
    // Demon Abilities (Vision of Death)
    SPELL_FEL_FIRE = 180618,                      // Fel Fire - Hellfire Grunt
    SPELL_FEL_FLAME = 180570,                     // Fel Flame - Hellfire Brute
    SPELL_SEARING_BREATH = 180033,                // Searing Breath - Hellfire Mistress
};

enum eKilroggCreatures
{
    NPC_DROOLING_BLOODSUCKER = 93369,             // Moving to blood pool
    NPC_MASSIVE_HORROR = 90523,                   // Transformed bloodsucker
    NPC_HELLFIRE_GRUNT = 90407,                   // Vision of Death demon
    NPC_HELLFIRE_BRUTE = 90411,                   // Vision of Death demon
    NPC_HELLFIRE_MISTRESS = 90414,                // Vision of Death demon
    NPC_BLOOD_POOL = 188852,                      // Blood pool creature
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
    TIMER_ARMOR_RIP = 15000,
    TIMER_HEARTSEEKER = 25000,
    TIMER_DEATH_THROES = 45000,
    TIMER_VISION_OF_DEATH = 60000,
    TIMER_SPAWN_BLOODSUCKER = 30000,
    TIMER_UPDATE_CORRUPTION = 1000,
    TIMER_FRENZIED_STRIKES = 5000,
    TIMER_SHATTERING_SCREAM = 10000,
    TIMER_FEL_FIRE = 3000,
    TIMER_FEL_FLAME = 5000,
    TIMER_SEARING_BREATH = 4000,
};

enum eKilroggPoints
{
    POINT_BLOOD_POOL = 1,
};



#endif // BOSS_KILROG_HFC_HPP
