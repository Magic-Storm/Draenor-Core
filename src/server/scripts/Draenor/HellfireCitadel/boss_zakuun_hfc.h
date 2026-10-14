///////////////////////////////////////////////////////////////////////////////
///
///  MILLENIUM-STUDIO
///  Copyright 2016 Millenium-studio SARL
///  All Rights Reserved.
///
///////////////////////////////////////////////////////////////////////////////

#ifndef BOSS_ZAKUUN_HFC_HPP
#define BOSS_ZAKUUN_HFC_HPP

#include "hellfire_citadel.h"

/// Fel Lord Zakuun - 89890
/// Destructor's Rise - Hellfire Citadel

enum eZakuunSpells
{
    // Phase transitions
    SpellArmedPhase             = 181507,  ///< Armed phase visual / aura
    SpellDisarmedPhase          = 181509,  ///< Disarmed (weapon thrown) aura

    // Armed phase abilities
    SpellSoulCleave             = 179406,  ///< Cleave that sends tank to Shadow Realm
    SpellSoulCleaveDebuff       = 179408,  ///< Shadow Realm debuff placed on tank
    SpellWakeOfDestruction      = 181499,  ///< Ground-wave AoE behind boss
    SpellRumblingFissure        = 179391,  ///< Growing ground crack AoE
    SpellSeedOfDestruction      = 181508,  ///< Placed under a random player, explodes
    SpellSeedExplosion          = 181510,  ///< Explosion damage from Seed of Destruction

    // Disarmed phase abilities
    SpellCavitation             = 181515,  ///< Knockback shockwave when axe lands
    SpellLatentEnergy           = 182008,  ///< Periodic Fel energy pulsing from axe landing zone
    SpellFelCrystalBeam         = 179432,  ///< Beam from Fel Crystal to boss

    // Enrage / combined phase (30%)
    SpellEnrage                 = 179980,  ///< Berserk enrage at 30%
    SpellFelOutburst            = 181504,  ///< Massive AoE during enrage phase

    // Shadow Realm mechanics
    SpellShadowRealm            = 179409,  ///< Teleport tank to Shadow Realm
    SpellResidualEnergyAura     = 182107,  ///< Aura on Residual Energy add

    // Bonus loot
    SpellZakuunBonusLoot        = 188979,
};

enum eZakuunCreatures
{
    NpcResidualEnergy           = 90294,   ///< Shadow Realm add from Soul Cleave
    NpcFelCrystal               = 90305,   ///< Summoned crystal during Disarmed phase
    NpcSeedStalker              = 90296,   ///< Invisible trigger for Seed of Destruction zone
};

enum eZakuunEvents
{
    EventSoulCleave             = 1,
    EventWakeOfDestruction,
    EventRumblingFissure,
    EventSeedOfDestruction,
    EventCavitation,
    EventLatentEnergy,
    EventSummonFelCrystal,
    EventPhaseSwitch,
    EventEnragePhase,
    EventFelOutburst,

    // Residual Energy add
    EventResidualEnergyPulse,
};

enum eZakuunPhases
{
    PhaseArmed      = 1,   ///< Boss wielding his axe - melee + Wave of Destruction heavy
    PhaseDisarmed   = 2,   ///< Axe thrown - Cavitation + Latent Energy + Fel Crystal
    PhaseEnrage     = 3,   ///< Below 30% - both phases merged, enrage
};

enum eZakuunActions
{
    ActionSwitchPhase   = 1,
    ActionEnterEnrage,
};

enum eZakuunTimers
{
    TimerSoulCleave         = 20  * TimeConstants::IN_MILLISECONDS,
    TimerWakeOfDestruction  = 12  * TimeConstants::IN_MILLISECONDS,
    TimerRumblingFissure    = 18  * TimeConstants::IN_MILLISECONDS,
    TimerSeedOfDestruction  = 25  * TimeConstants::IN_MILLISECONDS,
    TimerCavitation         = 8   * TimeConstants::IN_MILLISECONDS,
    TimerLatentEnergy       = 6   * TimeConstants::IN_MILLISECONDS,
    TimerSummonFelCrystal   = 15  * TimeConstants::IN_MILLISECONDS,
    TimerPhaseSwitch        = 90  * TimeConstants::IN_MILLISECONDS,
    TimerFelOutburst        = 10  * TimeConstants::IN_MILLISECONDS,
};

#endif // BOSS_ZAKUUN_HFC_HPP
