///////////////////////////////////////////////////////////////////////////////
///
///  MILLENIUM-STUDIO
///  Copyright 2016 Millenium-studio SARL
///  All Rights Reserved.
///
///////////////////////////////////////////////////////////////////////////////

#ifndef BOSS_ISKAR_HFC_HPP
#define BOSS_ISKAR_HFC_HPP

#include "hellfire_citadel.h"

enum eIskarSpells
{
    // Boss core abilities
    SPELL_EYE_OF_ANZU           = 179202,   ///< Eye of Anzu - passes between players
    SPELL_PHANTASMAL_WINDS      = 181956,   ///< Phantasmal Winds - AoE knockback
    SPELL_PHANTASMAL_WOUND      = 182178,   ///< Phantasmal Wound - stacking DoT
    SPELL_PHANTASMAL_FEL_BOMB   = 182200,   ///< Phantasmal Fel Bomb - targeted ground effect
    SPELL_FEL_INCINERATION      = 182323,   ///< Fel Incineration - channelled fire damage
    SPELL_CHORD_OF_SHADOWS      = 185239,   ///< Chord of Shadows - tether + damage
    SPELL_FEL_CHAKRAM           = 182367,   ///< Fel Chakram - bouncing projectile
    SPELL_FOCUSED_CHI           = 182136,   ///< Focused Chi - concentrated burst
    SPELL_SHADOWS_GAZE          = 183023,   ///< Shadow's Gaze - stare debuff

    // Bonus roll
    SPELL_ISKAR_BONUS_LOOT      = 188977,
};

enum eIskarCreatures
{
    NPC_FEL_RAVEN               = 91263,    ///< Periodic add - Phase 1
    NPC_CORRUPTED_FELBLADE      = 91266,    ///< Periodic add - Phase 1
    NPC_EYE_STALKER             = 91569,    ///< Cosmetic stalker for Eye of Anzu
};

enum eIskarEvents
{
    // Boss events
    EVENT_EYE_OF_ANZU = 1,
    EVENT_TRANSFER_EYE,
    EVENT_PHANTASMAL_WINDS,
    EVENT_PHANTASMAL_WOUND,
    EVENT_PHANTASMAL_FEL_BOMB,
    EVENT_FEL_INCINERATION,
    EVENT_CHORD_OF_SHADOWS,
    EVENT_FEL_CHAKRAM,
    EVENT_FOCUSED_CHI,
    EVENT_SHADOWS_GAZE,
    EVENT_SPAWN_FEL_RAVEN,
    EVENT_SPAWN_FELBLADE,

    // Add events
    EVENT_ADD_FEL_SLASH,
    EVENT_ADD_VOID_STATIC,
};

enum eIskarPhases
{
    PHASE_ISKAR         = 1,
    PHASE_ENRAGE        = 2,
};

enum eIskarActions
{
    ACTION_EYE_OF_ANZU_TRANSFER = 1,
    ACTION_EYE_EXPIRED,
};

enum eIskarData
{
    DATA_EYE_OWNER      = 1,
    DATA_RAVEN_COUNT,
};

enum eIskarTimers
{
    TIMER_EYE_OF_ANZU           = 15000,
    TIMER_EYE_TRANSFER          = 20000,    ///< Auto-transfer interval
    TIMER_PHANTASMAL_WINDS      = 25000,
    TIMER_PHANTASMAL_WOUND      = 18000,
    TIMER_FEL_BOMB              = 22000,
    TIMER_FEL_INCINERATION      = 30000,
    TIMER_CHORD_OF_SHADOWS      = 35000,
    TIMER_FEL_CHAKRAM           = 20000,
    TIMER_FOCUSED_CHI           = 40000,
    TIMER_SHADOWS_GAZE          = 28000,
    TIMER_SPAWN_FEL_RAVEN       = 30000,
    TIMER_SPAWN_FELBLADE        = 45000,
    TIMER_ADD_FEL_SLASH         = 8000,
    TIMER_ADD_VOID_STATIC       = 12000,
};

#endif // BOSS_ISKAR_HFC_HPP
