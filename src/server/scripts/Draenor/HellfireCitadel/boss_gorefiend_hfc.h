///////////////////////////////////////////////////////////////////////////////
///
///  MILLENIUM-STUDIO
///  Copyright 2016 Millenium-studio SARL
///  All Rights Reserved.
///
///////////////////////////////////////////////////////////////////////////////

#ifndef BOSS_GOREFIEND_HFC_HPP
#define BOSS_GOREFIEND_HFC_HPP

#include "hellfire_citadel.h"

/// ---- Spells ----------------------------------------------------------------

enum eGorefiendSpells
{
	/// Main rotation
	SpellFeastOfSouls		= 181295,	///< Channelled AoE drain that empowers Gorefiend
	SpellSharedFate			= 179864,	///< Tether two random players together - damages both if far apart
	SpellSharedFateVisual	= 179909,	///< Visual/aura component of Shared Fate
	SpellTouchOfDoom		= 179978,	///< Heavy DoT on current tank
	SpellShadowOfDeath		= 179909,	///< Sends target to Gorefiend's realm (reuse visual spell ID as placeholder)
	SpellHungerForLife		= 179883,	///< Heals Gorefiend when a player in the realm dies
	SpellConsumedSouls		= 181296,	///< Stacking power buff on Gorefiend for each soul consumed

	/// Adds spawned inside the realm
	SpellCrushingDarkness	= 180049,	///< Cast by Gorebound Spirit add
	SpellGoreboundCorrupt	= 180059,	///< DoT applied by Tortured Essence

	/// Bonus loot
	SpellGorefiendBonus		= 188976,
};

/// ---- Creatures -------------------------------------------------------------

enum eGorefiendCreatures
{
	NpcTorturedEssence		= 93145,	///< Shade add inside realm
	NpcGoreboundSpirit		= 91812,	///< Melee add inside realm
};

/// ---- Events ----------------------------------------------------------------

enum eGorefiendEvents
{
	EventFeastOfSouls		= 1,
	EventSharedFate,
	EventTouchOfDoom,
	EventShadowOfDeath,
	EventHungerForLife,
};

/// ---- Timers ----------------------------------------------------------------

enum eGorefiendTimers
{
	TimerFeastOfSouls		=  20000,	///< 20 sec initial
	TimerSharedFate			=  15000,
	TimerTouchOfDoom		=  10000,
	TimerShadowOfDeath		=  25000,
	TimerHungerForLife		=  40000,
};

/// ---- Actions ---------------------------------------------------------------

enum eGorefiendActions
{
	ActionSoulEscaped		= 1,		///< A player escaped the realm alive -> remove empowerment stack
};

#endif /// BOSS_GOREFIEND_HFC_HPP
