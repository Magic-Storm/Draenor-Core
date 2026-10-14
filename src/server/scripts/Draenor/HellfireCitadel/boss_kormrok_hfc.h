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
///  Pool leap destination positions
///  TODO: Verify exact coordinates against in-game data
/// ============================================================================
static Position const g_KormrokFieryPoolPos  = { 4380.0f, 651.0f, 87.5f, 0.0f }; ///< Northwest — empowers Pound
static Position const g_KormrokFoulPoolPos   = { 4430.0f, 651.0f, 87.5f, 0.0f }; ///< Northeast — empowers Fel Outpouring
static Position const g_KormrokShadowPoolPos = { 4405.0f, 593.0f, 87.5f, 0.0f }; ///< South      — empowers Explosive Runes & Grasping Hands

/// ============================================================================
///  Pool type identifiers
/// ============================================================================
enum eKormrokPools : uint8
{
	PoolNone    = 0,
	PoolFiery   = 1,   ///< Empowers Pound
	PoolFoul    = 2,   ///< Empowers Fel Outpouring
	PoolShadow  = 3,   ///< Empowers Explosive Runes + Grasping Hands
};

/// ============================================================================
///  Spells
/// ============================================================================
enum eKormrokSpells
{
	/// Core abilities
	SpellPound                   = 185427,  ///< Main tank hit — heavy physical damage + debuff
	SpellExplosiveRunes          = 180270,  ///< Places rune traps that detonate after a short delay
	SpellGraspingHands           = 180246,  ///< Hands erupt from the ground, gripping players
	SpellFelOutpouring           = 181992,  ///< Fel energy wave that strikes the entire raid

	/// Empowered variants (activated by leaping into the matching pool)
	/// SpellEmpoweredPound uses the same spell ID as SpellPound;
	///   actual damage increase should come from a separate DB aura or script hook.
	SpellEmpoweredPound          = 185427,  ///< Empowered Pound         (placeholder — same ID)
	SpellEmpoweredExplosiveRunes = 180286,  ///< Empowered Explosive Runes
	SpellEmpoweredGraspingHands  = 180297,  ///< Empowered Grasping Hands
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
