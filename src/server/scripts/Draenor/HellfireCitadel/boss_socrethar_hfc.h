///////////////////////////////////////////////////////////////////////////////
///
///  MILLENIUM-STUDIO
///  Copyright 2016 Millenium-studio SARL
///  All Rights Reserved.
///
///////////////////////////////////////////////////////////////////////////////

#ifndef BOSS_SOCRETHAR_HFC_HPP
#define BOSS_SOCRETHAR_HFC_HPP

#include "hellfire_citadel.h"

enum eSocretharSpells
{
    // Phase 1 – Soulbound Construct
    SPELL_REVERBERATING_BLOW        = 182020,   ///< Massive cleave on tank
    SPELL_FEL_PRISON                = 183017,   ///< Traps a player in place
    SPELL_GIFT_OF_MANARI            = 184124,   ///< AoE heal for Construct
    SPELL_CONSTRUCT_APOCALYPSE      = 184053,   ///< Channelled devastation (Phase 1)

    // Phase 2 – Socrethar
    SPELL_DOMINATING_POWER          = 184394,   ///< Mind control / domination
    SPELL_SHADOW_WORD_AGONY         = 184081,   ///< Shadow DoT on random target
    SPELL_EXERTAR_OF_SOULS          = 182769,   ///< Soul rip – AoE soul explosion
    SPELL_APOCALYPSE_FINAL          = 184053,   ///< Apocalypse (shared ID, Phase 2 cast)

    // Bonus roll
    SPELL_SOCRETHAR_BONUS_LOOT      = 188978,
};

enum eSocretharCreatures
{
    NPC_SOULBOUND_CONSTRUCT         = 90296,    ///< Phase 1 construct vehicle
    NPC_SARGEREI_SPIRITBINDER       = 92510,    ///< Phase 2 healer add
    NPC_SARGEREI_DOMINATOR          = 92507,    ///< Phase 2 DPS add
};

enum eSocretharEvents
{
    // Phase 1 – Construct events
    EVENT_REVERBERATING_BLOW = 1,
    EVENT_FEL_PRISON,
    EVENT_GIFT_OF_MANARI,
    EVENT_CONSTRUCT_APOCALYPSE,

    // Phase 2 – Socrethar events
    EVENT_DOMINATING_POWER,
    EVENT_SHADOW_WORD_AGONY,
    EVENT_EXERTAR_OF_SOULS,
    EVENT_SPAWN_SARGEREI,
    EVENT_APOCALYPSE,

    // Construct add events
    EVENT_CONSTRUCT_REVERB,
    EVENT_CONSTRUCT_FEL_PRISON,
};

enum eSocretharPhases
{
    PHASE_CONSTRUCT     = 1,
    PHASE_SOCRETHAR     = 2,
};

enum eSocretharActions
{
    ACTION_CONSTRUCT_WEAK           = 1,    ///< Construct hit 30% – trigger Phase 2
    ACTION_CONSTRUCT_SLAIN,                 ///< Construct killed
    ACTION_ENTER_PHASE2,
};

enum eSocretharData
{
    DATA_CONSTRUCT_GUID     = 1,
    DATA_PHASE,
};

enum eSocretharTimers
{
    // Phase 1
    TIMER_REVERBERATING_BLOW        = 12000,
    TIMER_FEL_PRISON                = 22000,
    TIMER_GIFT_OF_MANARI            = 35000,
    TIMER_CONSTRUCT_APOCALYPSE      = 45000,

    // Phase 2
    TIMER_DOMINATING_POWER          = 20000,
    TIMER_SHADOW_WORD_AGONY         = 15000,
    TIMER_EXERTAR_OF_SOULS          = 30000,
    TIMER_SPAWN_SARGEREI            = 25000,
    TIMER_APOCALYPSE                = 40000,
};

#endif // BOSS_SOCRETHAR_HFC_HPP
