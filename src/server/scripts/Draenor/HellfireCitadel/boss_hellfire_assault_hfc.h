///////////////////////////////////////////////////////////////////////////////
///
///  MILLENIUM-STUDIO
///  Copyright 2016 Millenium-studio SARL
///  All Rights Reserved.
///
///////////////////////////////////////////////////////////////////////////////

#ifndef BOSS_HELLFIRE_ASSAULT_HFC_HPP
#define BOSS_HELLFIRE_ASSAULT_HFC_HPP

#include "hellfire_citadel.h"

/// ============================================================================
///  Spells
/// ============================================================================
enum eHellfireAssaultSpells
{
	/// Siegemaster Mar'tak
	SpellHowlingAxe              = 184243,  ///< Thrown axe — damages target and nearby players
	SpellShockwave               = 184394,  ///< Frontal cone knockback + physical damage
	SpellTraitorsVow             = 185237,  ///< Raid-wide damage and movement speed reduction

	/// Felfire Munitions
	SpellFelfireMunitions        = 180926,  ///< Carrier aura — applied when player picks up crate

	/// Bonus Loot
	SpellHellfireAssaultLoot     = 188972,
};

/// ============================================================================
///  Creatures
/// ============================================================================
enum eHellfireAssaultCreatures
{
	NpcHellfireGrunt             = 93858,   ///< Hulking Berserker — assault wave melee add
	NpcFelfireSiegeVehicle       = 91103,   ///< Felfire Demolisher — drops Felfire Munitions on death
	NpcFelfireMunitionsCrate     = 90610,   ///< Felfire Munitions — interactable pickup
	NpcHellfireCannon            = 90018,   ///< Alliance cannon — players deliver munitions here to breach the door
};

/// ============================================================================
///  Event IDs
/// ============================================================================
enum eHellfireAssaultEvents
{
	EventHowlingAxe              = 1,
	EventShockwave,
	EventTraitorsVow,
	EventSpawnWave,
};

/// ============================================================================
///  Timers
/// ============================================================================
enum eHellfireAssaultTimers
{
	TimerHowlingAxe              = 12 * TimeConstants::IN_MILLISECONDS,
	TimerHowlingAxeCD            = 20 * TimeConstants::IN_MILLISECONDS,
	TimerShockwave               = 25 * TimeConstants::IN_MILLISECONDS,
	TimerShockwaveCD             = 35 * TimeConstants::IN_MILLISECONDS,
	TimerTraitorsVow             = 40 * TimeConstants::IN_MILLISECONDS,
	TimerTraitorsVowCD           = 60 * TimeConstants::IN_MILLISECONDS,
	TimerFirstWave               = 15 * TimeConstants::IN_MILLISECONDS,
	TimerWaveCD                  = 45 * TimeConstants::IN_MILLISECONDS,
};

/// ============================================================================
///  Actions (cross-NPC communication)
/// ============================================================================
enum eHellfireAssaultActions
{
	ActionMunitionDelivered      = 1,   ///< Cannon → Mar'tak: player delivered munitions
	ActionCannonDestroyed,              ///< Cannon → Mar'tak: cannon was killed
};

/// ============================================================================
///  Constants
/// ============================================================================
static uint8 const g_MunitionsToBreachDoor = 10; ///< Deliveries needed to end the encounter
static uint8 const g_WaveGruntCount        =  4; ///< Grunts per assault wave

#endif /// BOSS_HELLFIRE_ASSAULT_HFC_HPP
