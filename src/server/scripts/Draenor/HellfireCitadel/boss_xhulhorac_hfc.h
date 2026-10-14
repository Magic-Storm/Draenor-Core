///////////////////////////////////////////////////////////////////////////////
///
///  MILLENIUM-STUDIO
///  Copyright 2016 Millenium-studio SARL
///  All Rights Reserved.
///
///////////////////////////////////////////////////////////////////////////////

#ifndef BOSS_XHULHORAC_HFC_HPP
#define BOSS_XHULHORAC_HFC_HPP

#include "hellfire_citadel.h"

/// Xhul'horac - 93068
/// Destructor's Rise - Hellfire Citadel

enum eXhulSpells
{
    // Fel abilities
    SpellFelblazeFlurry         = 186073,  ///< Rapid fel fire strikes on current target
    SpellChainsOfFel            = 186500,  ///< Chains a random player to the ground
    SpellFelstar                = 186071,  ///< Fel projectile seeding fire patches

    // Void abilities
    SpellCreepingVoid           = 186063,  ///< Spreading void zone under random player
    SpellBlackHole              = 186546,  ///< Massive void suck AoE
    SpellVoidStep               = 186061,  ///< Boss blinks to a random player leaving void pool
    SpellVoidstar               = 186059,  ///< Void projectile seeding void patches

    // Merged / combined
    SpellVoidFelExplosion       = 186543,  ///< Collision explosion when Fel and Void meet

    // Portal / summon auras
    SpellOpenFelPortal          = 186074,  ///< Opens a Fel portal
    SpellOpenVoidPortal         = 186060,  ///< Opens a Void portal

    // Phase auras (cosmetic/mechanical)
    SpellFelAffinity            = 186076,  ///< Boss is in Fel-dominant phase
    SpellVoidAffinity           = 186062,  ///< Boss is in Void-dominant phase

    // Bonus loot
    SpellXhulBonusLoot          = 188980,
};

enum eXhulCreatures
{
    NpcOmnus                    = 93344,   ///< Large Fel add from Fel portal
    NpcUnstableVoidfiend        = 93071,   ///< Large Void add from Void portal
    NpcFelImpXhul               = 93073,   ///< Small Fel imp add
    NpcFelPortalStalker         = 93072,   ///< Invisible trigger for Fel portal
    NpcVoidPortalStalker        = 93074,   ///< Invisible trigger for Void portal
};

enum eXhulEvents
{
    EventFelblazeFlurry         = 1,
    EventChainsOfFel,
    EventFelstar,
    EventCreepingVoid,
    EventBlackHole,
    EventVoidStep,
    EventVoidstar,
    EventPhaseSwitch,
    EventOpenFelPortal,
    EventOpenVoidPortal,
    EventVoidFelExplosion,

    // Add events
    EventOmnusFelBolt,
    EventUnstableVoidPulse,
};

enum eXhulPhases
{
    PhaseFel    = 1,   ///< Fel-dominant: Felblaze Flurry, Chains of Fel, Fel portals active
    PhaseVoid   = 2,   ///< Void-dominant: Creeping Void, Black Hole, Void portals active
    PhaseCombined = 3, ///< Final merge: all abilities, void/fel collision damage
};

enum eXhulActions
{
    ActionSwitchPhase   = 1,
    ActionEnterCombined,
};

enum eXhulTimers
{
    TimerFelblazeFlurry     = 15 * TimeConstants::IN_MILLISECONDS,
    TimerChainsOfFel        = 22 * TimeConstants::IN_MILLISECONDS,
    TimerFelstar            = 18 * TimeConstants::IN_MILLISECONDS,
    TimerCreepingVoid       = 12 * TimeConstants::IN_MILLISECONDS,
    TimerBlackHole          = 30 * TimeConstants::IN_MILLISECONDS,
    TimerVoidStep           = 20 * TimeConstants::IN_MILLISECONDS,
    TimerVoidstar           = 16 * TimeConstants::IN_MILLISECONDS,
    TimerPhaseSwitch        = 80 * TimeConstants::IN_MILLISECONDS,
    TimerOpenFelPortal      = 25 * TimeConstants::IN_MILLISECONDS,
    TimerOpenVoidPortal     = 25 * TimeConstants::IN_MILLISECONDS,
    TimerVoidFelExplosion   = 10 * TimeConstants::IN_MILLISECONDS,
    TimerOmnusFelBolt       = 6  * TimeConstants::IN_MILLISECONDS,
    TimerUnstableVoidPulse  = 5  * TimeConstants::IN_MILLISECONDS,
};

#endif // BOSS_XHULHORAC_HFC_HPP
