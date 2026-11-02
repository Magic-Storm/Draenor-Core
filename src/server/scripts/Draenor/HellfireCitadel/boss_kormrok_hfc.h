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
	/// Core abilities. Pound is never pool-empowered.
	/// IDs: BigWigs Kormrok.lua + Wowhead (Explosive Runes 181296, not 180270).
	SpellPound                   = 180244,  ///< Raid-wide shockwaves under every player
	SpellExplosiveRunes          = 181296,  ///< Five runes; unsoaked runes hit the raid
	SpellGraspingHands           = 181299,  ///< Hands root players until killed
	SpellFelOutpouring           = 181292,  ///< Shadow globules travel from the purple pool
	SpellLeap                    = 180068,

	/// Empowered variants. Which one is live depends on the pool he just leapt into.
	SpellEmpoweredExplosiveRunes = 181297,  ///< Fiery pool
	SpellDraggingHands           = 181300,  ///< Foul pool (empowered Grasping Hands)
	SpellEmpoweredFelOutpouring  = 181293,  ///< Shadow pool; all three pools spew globules

	/// Tank strikes gained for the rest of that empowerment. Not cast on LFR.
	SpellExplosiveBurst          = 181306,  ///< Fiery pool
	SpellFoulCrush               = 181307,  ///< Foul pool
	SpellSwat                    = 181305,  ///< Shadow pool

	/// Energy auras applied on landing
	SpellExplosiveEnergy         = 180116,
	SpellFoulEnergy              = 180117,
	SpellShadowEnergy            = 180115,
	SpellEnrage                  = 186882,

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
	EventTankStrike,
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
	/// First leap is ~10s after pull (BigWigs). Later leaps are ~133s, 145s from the shadow pool.
	TimerLeapFirst        =  10 * TimeConstants::IN_MILLISECONDS,
	TimerLeapExplosive    = 133 * TimeConstants::IN_MILLISECONDS,
	TimerLeapShadow       = 145 * TimeConstants::IN_MILLISECONDS,
	TimerLeapLFR          =  92 * TimeConstants::IN_MILLISECONDS,
	TimerTankStrike       =  25 * TimeConstants::IN_MILLISECONDS,
	TimerTankStrikeCD     =  38 * TimeConstants::IN_MILLISECONDS,
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
