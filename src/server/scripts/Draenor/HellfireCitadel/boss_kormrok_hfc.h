///////////////////////////////////////////////////////////////////////////////
///
///  MILLENIUM-STUDIO
///  Copyright 2016 Millenium-studio SARL
///  All Rights Reserved.
///
///////////////////////////////////////////////////////////////////////////////

#ifndef BOSS_KORMROK_HFC_HPP
#define BOSS_KORMROK_HFC_HPP

#include "hellfire_citadel.h"

/// ============================================================================
///  Pool leap destination positions (approx. — refine from sniff when available)
/// ============================================================================
static Position const g_KormrokFieryPoolPos  = { 4380.0f, 651.0f, 87.5f, 0.0f }; ///< Explosive Energy — empowers Explosive Runes
static Position const g_KormrokFoulPoolPos   = { 4430.0f, 651.0f, 87.5f, 0.0f }; ///< Foul Energy     — empowers Grasping Hands
static Position const g_KormrokShadowPoolPos = { 4405.0f, 593.0f, 87.5f, 0.0f }; ///< Shadow Energy   — empowers Fel Outpouring

/// ============================================================================
///  Pool type identifiers
/// ============================================================================
enum eKormrokPools : uint8
{
	PoolNone    = 0,
	PoolFiery   = 1,   ///< Explosive Energy → Empowered Explosive Runes
	PoolFoul    = 2,   ///< Foul Energy     → Empowered Grasping Hands
	PoolShadow  = 3,   ///< Shadow Energy   → Empowered Fel Outpouring
};

/// ============================================================================
///  Spells  (https://www.wowhead.com/npc=90776/kormrok)
/// ============================================================================
enum eKormrokSpells
{
	/// Core abilities — Pound is never pool-empowered
	SpellPound                   = 180244,  ///< Raid-wide shockwaves under every player
	SpellExplosiveRunes          = 180270,  ///< Places rune traps that detonate after a short delay
	SpellGraspingHands           = 181299,  ///< Hands erupt from the ground, gripping players
	SpellFelOutpouring           = 181292,  ///< Shadow pool spews globules across the room

	/// Empowered variants (leap into matching energy pool)
	SpellEmpoweredExplosiveRunes = 181297,  ///< Empowered Explosive Runes
	SpellEmpoweredGraspingHands  = 180297,  ///< Empowered Grasping Hands / Dragging Hands
	SpellEmpoweredFelOutpouring  = 185816,  ///< Empowered Fel Outpouring

	/// Bonus loot
	SpellKormrokLoot             = 188974,
};

/// ============================================================================
///  Event IDs
/// ============================================================================
enum eKormrokEvents
{
	EventPound           = 1,
	EventExplosiveRunes,
	EventGraspingHands,
	EventFelOutpouring,
	EventLeapToPool,
};

/// ============================================================================
///  Timers
/// ============================================================================
enum eKormrokTimers
{
	TimerPound            = 10  * TimeConstants::IN_MILLISECONDS,
	TimerPoundCD          = 18  * TimeConstants::IN_MILLISECONDS,
	TimerExplosiveRunes   = 20  * TimeConstants::IN_MILLISECONDS,
	TimerExplosiveRunesCD = 30  * TimeConstants::IN_MILLISECONDS,
	TimerGraspingHands    = 35  * TimeConstants::IN_MILLISECONDS,
	TimerGraspingHandsCD  = 40  * TimeConstants::IN_MILLISECONDS,
	TimerFelOutpouring    = 25  * TimeConstants::IN_MILLISECONDS,
	TimerFelOutpouringCD  = 35  * TimeConstants::IN_MILLISECONDS,
	TimerLeapToPool       = 100 * TimeConstants::IN_MILLISECONDS,
	TimerResumeAfterLeap  =   2 * TimeConstants::IN_MILLISECONDS,
};

/// ============================================================================
///  Motion points
/// ============================================================================
enum eKormrokPoints
{
	PointPoolLeap = 1,
};

#endif /// BOSS_KORMROK_HFC_HPP
