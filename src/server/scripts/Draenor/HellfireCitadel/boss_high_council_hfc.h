///////////////////////////////////////////////////////////////////////////////
///
///  MILLENIUM-STUDIO
///  Copyright 2016 Millenium-studio SARL
///  All Rights Reserved.
///
///////////////////////////////////////////////////////////////////////////////

#ifndef BOSS_HIGH_COUNCIL_HFC_HPP
#define BOSS_HIGH_COUNCIL_HFC_HPP

#include "hellfire_citadel.h"

/// ---- Spells ----------------------------------------------------------------

enum eCouncilSpells
{
	/// Dia Darkwhisper (92144)
	SpellVoidBolt			= 184365,	///< Shadow bolt on current target
	SpellNightmareVisage	= 184449,	///< Fear on random player
	SpellVoidHaze			= 184357,	///< Ground AoE pool
	SpellVoidShroud			= 184362,	///< Damage reduction aura applied to nearby council members

	/// Gurtogg Bloodboil (92146)
	SpellAcidicWound		= 184827,	///< Stacking armor reduction DoT on current target
	SpellFelRage			= 184358,	///< Enrage - attack speed + damage buff
	SpellBloodyCleave		= 184366,	///< Frontal cleave

	/// Jubei'thos (92142)
	SpellFelstormBlade		= 184355,	///< Whirlwind-style AoE around self
	SpellMirrorImage		= 184749,	///< Summons copies of Jubei'thos

	/// Shared / Bonus
	SpellCouncilBonus		= 188975,	///< Bonus loot spell
};

/// ---- Creatures -------------------------------------------------------------

enum eCouncilCreatures
{
	NpcJubeithosImage		= 92143,	///< Mirror image clone of Jubei'thos
};

/// ---- Events ----------------------------------------------------------------

enum eCouncilEvents
{
	/// Dia Darkwhisper
	EventVoidBolt			= 1,
	EventNightmareVisage,
	EventVoidHaze,

	/// Gurtogg Bloodboil
	EventAcidicWound,
	EventFelRage,
	EventBloodyCleave,

	/// Jubei'thos
	EventFelstormBlade,
	EventMirrorImage,
};

/// ---- Timers ----------------------------------------------------------------

enum eCouncilTimers
{
	TimerVoidBolt			= 8000,
	TimerNightmareVisage	= 22000,
	TimerVoidHaze			= 18000,

	TimerAcidicWound		= 7000,
	TimerFelRage			= 30000,
	TimerBloodyCleave		= 12000,

	TimerFelstormBlade		= 10000,
	TimerMirrorImage		= 35000,
};

#endif /// BOSS_HIGH_COUNCIL_HFC_HPP
