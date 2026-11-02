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
    /// IDs from BigWigs Socrethar.lua and Wowhead.
    /// 182020 is Iron Reaver's Pounding, 184394 is Hellfire Assault's Shockwave.
    /// Neither belongs on this fight.

    /// Phase 1 – Soulbound Construct (90296)
    SPELL_REVERBERATING_BLOW        = 180008,   ///< Tank cleave, first at 7s
    SPELL_FEL_PRISON                = 181288,   ///< Cast. The root aura is 183017.
    SPELL_VOLATILE_FEL_ORB          = 180221,   ///< Bouncing orb, first at 13s
    SPELL_FELBLAZE_CHARGE           = 182051,   ///< Charge, first at 29s

    /// Phase 2 – Socrethar (92330) and the Sargerei
    SPELL_EXERT_DOMINANCE           = 183331,   ///< Interruptible cast
    SPELL_APOCALYPSE                = 183329,   ///< Channel. 184053 is Fel Barrier, not this.
    SPELL_GIFT_OF_MANARI            = 184124,   ///< Dominator heal, not a Construct spell
    SPELL_SHADOW_BOLT_VOLLEY        = 182392,   ///< Sargerei Shadowcaller
    SPELL_GHASTLY_FIXATION          = 182769,   ///< Haunting Soul

    /// Bonus roll
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
    /// Phase 1, from BigWigs OnEngage
    TIMER_REVERBERATING_BLOW        = 7000,
    TIMER_REVERBERATING_BLOW_CD     = 17000,
    TIMER_FEL_PRISON                = 48000,
    TIMER_FEL_PRISON_CD             = 45000,
    TIMER_VOLATILE_FEL_ORB          = 13000,
    TIMER_VOLATILE_FEL_ORB_CD       = 30000,
    TIMER_FELBLAZE_CHARGE           = 29000,
    TIMER_FELBLAZE_CHARGE_CD        = 30000,

    /// Phase 2
    TIMER_EXERT_DOMINANCE           = 20000,
    TIMER_SPAWN_SARGEREI            = 24000,
    TIMER_APOCALYPSE                = 51000,
    TIMER_APOCALYPSE_CD             = 36000,
};

#endif // BOSS_SOCRETHAR_HFC_HPP
