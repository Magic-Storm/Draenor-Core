///////////////////////////////////////////////////////////////////////////////
///
///  MILLENIUM-STUDIO
///  Copyright 2016 Millenium-studio SARL
///  All Rights Reserved.
///
///////////////////////////////////////////////////////////////////////////////

#ifndef BOSS_VELHARI_HFC_HPP
#define BOSS_VELHARI_HFC_HPP

#include "hellfire_citadel.h"

/// Tyrant Velhari - 93439
/// Bastion of Shadows - Hellfire Citadel

enum eVelhariSpells
{
    // Phase 1 (100-70%) - Aura of Contempt
    SpellAuraOfContempt         = 179986,  ///< Aura of Contempt - stacking raid damage
    SpellSealOfDecay            = 180353,  ///< Tank swap stacking DoT
    SpellEdictOfCondemnation    = 182884,  ///< Marks players for mass execution
    SpellGavelOfTheTyrant       = 180258,  ///< Melee range shockwave slam

    // Phase 2 (70-40%) - Aura of Malice
    SpellAuraOfMalice           = 180018,  ///< Aura of Malice - increased damage + debuff
    SpellAnimaFont              = 180800,  ///< Spawns Anima Font adds / pools
    SpellFontOfCorruption       = 180802,  ///< Corruption burst from Font add
    SpellAnnihilationStrike     = 180810,  ///< Heavy cleave that resets threat

    // Phase 3 (40-0%) - Aura of Oppression
    SpellAuraOfOppression       = 179987,  ///< Aura of Oppression - lethal stacking
    SpellFinalVerdict           = 182880,  ///< Massive unavoidable raid damage
    SpellSanguineFont           = 182878,  ///< Blood pool add/zone

    // Shared / persistent
    SpellAncientCorruption      = 180351,  ///< Stacking corruption on all players
    SpellEmpoweredAura          = 182881,  ///< Aura intensifies between phases

    // Bonus loot
    SpellVelhariBonusLoot       = 188981,
};

enum eVelhariCreatures
{
    NpcAnimaFountain            = 93569,  ///< Channeling Anima Font add (Phase 2)
    NpcSanguineFontAdd          = 93570,  ///< Blood pool add (Phase 3)
    NpcCorruptionStalker        = 93572,  ///< Invisible zone stalker
};

enum eVelhariEvents
{
    // Phase 1
    EventSealOfDecay            = 1,
    EventEdictOfCondemnation,
    EventGavelOfTheTyrant,

    // Phase 2
    EventAnimaFont,
    EventAnnihilationStrike,

    // Phase 3
    EventFinalVerdict,
    EventSanguineFont,

    // Shared
    EventAncientCorruption,
    EventPhaseTransition,

    // Add events
    EventFontCorruptionPulse,
};

enum eVelhariPhases
{
    PhaseContempt   = 1,  ///< Phase 1: 100-70%, Aura of Contempt
    PhaseMalice     = 2,  ///< Phase 2: 70-40%, Aura of Malice
    PhaseOppression = 3,  ///< Phase 3: 40-0%, Aura of Oppression
};

enum eVelhariActions
{
    ActionPhaseTwo      = 1,
    ActionPhaseThree,
};

enum eVelhariTimers
{
    TimerSealOfDecay            = 15 * TimeConstants::IN_MILLISECONDS,
    TimerEdictOfCondemnation    = 25 * TimeConstants::IN_MILLISECONDS,
    TimerGavelOfTheTyrant       = 20 * TimeConstants::IN_MILLISECONDS,
    TimerAnimaFont              = 18 * TimeConstants::IN_MILLISECONDS,
    TimerAnnihilationStrike     = 12 * TimeConstants::IN_MILLISECONDS,
    TimerFinalVerdict           = 20 * TimeConstants::IN_MILLISECONDS,
    TimerSanguineFont           = 15 * TimeConstants::IN_MILLISECONDS,
    TimerAncientCorruption      = 5  * TimeConstants::IN_MILLISECONDS,
    TimerFontCorruptionPulse    = 4  * TimeConstants::IN_MILLISECONDS,
};

#endif // BOSS_VELHARI_HFC_HPP
