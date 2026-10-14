////////////////////////////////////////////////////////////////////////////////
//
//  MILLENIUM-STUDIO
//  Copyright 2016 Millenium-studio SARL
//  All Rights Reserved.
//
////////////////////////////////////////////////////////////////////////////////

#ifndef HELLFIRE_CITADEL_HPP
#define HELLFIRE_CITADEL_HPP

# include "InstanceScript.h"
# include "ScriptedCosmeticAI.hpp"
# include "ScriptedCreature.h"
# include "ScriptMgr.h"
# include "ScriptUtils.h"
# include "Cell.h"
# include "CellImpl.h"
# include "GridNotifiers.h"
# include "GridNotifiersImpl.h"
# include "Vehicle.h"
# include "GameObjectAI.h"
# include "Group.h"
# include "MoveSplineInit.h"
# include "CreatureTextMgr.h"

// About HP scaling, in DB, Health_mod for Exp 5 (1 = 109k HP)
// Encounter list: https://www.wowhead.com/zone=7545/hellfire-citadel

static void CastSpellToPlayers(Map* p_Map, Unit* p_Caster, uint32 p_SpellID, bool p_Triggered)
{
	if (p_Map == nullptr)
		return;

	Map::PlayerList const& l_Players = p_Map->GetPlayers();
	for (Map::PlayerList::const_iterator l_Iter = l_Players.begin(); l_Iter != l_Players.end(); ++l_Iter)
	{
		if (Player* l_Player = l_Iter->getSource())
		{
			if (p_Caster != nullptr)
				p_Caster->CastSpell(l_Player, p_SpellID, p_Triggered);
			else
				l_Player->CastSpell(l_Player, p_SpellID, p_Triggered);
		}
	}
}

static void DespawnCreaturesInArea(uint32 p_Entry, WorldObject* p_WorldObject)
{
	std::list<Creature*> l_Creatures;
	GetCreatureListWithEntryInGrid(l_Creatures, p_WorldObject, p_Entry, p_WorldObject->GetMap()->GetVisibilityRange());

	for (Creature* l_Iter : l_Creatures)
		l_Iter->DespawnOrUnsummon();
}

enum eCitadelZones
{
	MapHellfireCitadel = 1448,
};

enum eCitadelCreatures
{
	/// Hellbreach - Part 1
	BossMartak					= 95068,
	BossReaver					= 90284,
	BossKormrok					= 90776,

	/// Hellfire Antechamber / Passage trash
	/// https://www.wowhead.com/guide/raiding/hellfire-citadel/kormrok-strategy-guide
	NpcShamblingHulk			= 94804,
	NpcFelHellweaver			= 94806,
	NpcHellfireGuardian		= 94808,	///< Antechamber trash
	NpcHellfireGuardianWall	= 94985,	///< Iron Reaver wall (Turning the Tide)
	NpcGoreboundAssassin		= 94832,
	NpcFelTouchedSeer			= 94916,
	NpcIronPeon				= 94924,
	NpcFelStarvedTrainee		= 94947,
	NpcGraggra				= 94995,	///< Mini-boss before Court of Blood

	/// Pits of Mannoroth trash
	NpcMorkronn				= 94777,
	NpcSovokk					= 94779,
	NpcFelExtractor			= 94285,
	NpcShadowInfuser			= 94283,
	NpcFieryEnkindler			= 94284,
	NpcTogdrov				= 94816,
	NpcArmoredSkullsmasher	= 94859,
	NpcGrimAmbusher			= 94861,
	NpcKeenEyedGronnstalker	= 94894,

	/// Halls of Blood - Part 2
	BossGurtogg					= 92146,
	BossDia						= 92144,
	BossJubeithos				= 92142,
	BossKilrogg					= 90378,
	BossGorefiend				= 91809,

	NpcSalivatingBloodthirster = 90521,
	NpcHulkingTerror			= 90523,
	NpcHellfireDeathcaller	= 92740,

	/// Bastion of Shadows - Part 3
	BossIskar					= 90316,
	BossSocrethar				= 92330,
	BossVelhari					= 93439,

	NpcSoulboundConstruct		= 90296,
	NpcKorvos					= 95642,
	NpcAdjunctKuroh			= 91520,

	/// Destructor's Rise - Part 4
	BossZakuun					= 89890,
	BossXhulhorac				= 93068,
	BossMannoroth				= 91349,

	NpcDagGorath				= 92527,
	NpcMekBarash				= 92554,
	NpcZerikShekor			= 92526,
	NpcVazeelFazag			= 92523,
	NpcShaoGhun				= 92514,
	NpcGlynevere				= 94617,
	NpcCattwen				= 94618,
	NpcBryanda				= 94724,
	NpcFelScorcher			= 93913,

	FelSummoner					= 91305,
	MassiveBlastWave			= 454533,
	FelRedSpire					= 95851,
	FelPurpleSpire				= 464544,
	FelGreenSpire				= 464545,
	GuldanMann					= 91326,
	ChannelTarget				= 94928,
	DoomLord					= 91241,
	FelImp						= 91259,
	DreadInfernal				= 91270,
	KhadgarMann					= 95659,
	PortalMann					= 95699,

	/// The Black Gate - Part 5
	Anetheron					= 95408,
	Kazrogal					= 95280,
	Azgalor						= 95282,
	AchievementStalker			= 80831,
	BossArchimonde				= 91331,
};

enum eCitadelGameObjects
{
	/// Hellbreach - Part 1
	ReinforcedHellfireDoor		= 242464,

	/// Destructor's Rise - Part 4
	MannorothPlatformFix		= 700000,

	/// The Black Gate - Part 5
	DarkPortalFireWall			= 241485,
};

enum eCitadelDatas
{
	/// Bosses
	DataMartak,
	DataReaver,
	DataKormrok,
	DataCouncil,
	DataKilrogg,
	DataGorefiend,
	DataIskar,
	DataSocrethar,
	DataVelhari,
	DataZakuun,
	DataXhulhorac,
	DataMannoroth,
	DataArchimonde,
	MaxBossData,

	DataFelIronSummoner			= eCitadelDatas::MaxBossData,

	FriendlyFaction				= 35,

	CouncilCount				= 3,
	MaxSummoners				= 3,
};

enum eCitadelSpells
{
	Berserker					= 26662,

	/// Bonus roll (boss death → CastSpellToPlayers)
	/// https://www.wowhead.com/zone=7545/hellfire-citadel
	BonusAssault				= 188972,
	BonusReaver					= 188973,
	BonusKormrok				= 188974,
	BonusCouncil				= 188975,
	BonusKilrogg				= 188976,	///< shared Gorefiend ID in Millenium data
	BonusGorefiend				= 188976,
	BonusIskar					= 188977,
	BonusSocrethar				= 188978,
	BonusZakuun					= 188979,
	BonusXhulhorac				= 188980,
	BonusVelhari				= 188981,
	BonusMannoroth				= 188983,
	BonusArchimonde				= 188984,

	/// Antechamber trash
	/// https://www.wowhead.com/guide/raiding/hellfire-citadel/kormrok-strategy-guide
	SpellFelStrike				= 188092,	///< Empowered Fel Strike also 188092; tank hit after leap
	SpellShieldLeap				= 188098,
	SpellShieldLeapImpact		= 188093,
	SpellHellweaving			= 188085,
	SpellHellweavingAura		= 188087,
	SpellFelFireball			= 187221,
	SpellOrbOfDestruction		= 188061,
	SpellOrbOfDestructionMissile = 188057,
	SpellFelCauterize			= 188213,
	SpellFelLavaBeam			= 188225,
	SpellFelSlam				= 187994,
	SpellBrutalShout			= 188012,
	SpellThrowExplosiveCrate	= 188221,
	SpellExplosiveCrateImpact	= 188223,
	SpellHammerThrow			= 189917,
	SpellFelPoison				= 188189,
	SpellCheapShot				= 1833,

	/// Pits of Mannoroth trash
	SpellTrashGraspingHand		= 188082,
	SpellTrashGraspingHands		= 181299,
	SpellSwat					= 181305,
	SpellLocalizedExplosion		= 188106,
	SpellPrimalEnergies			= 187130,
	SpellResidualShadows		= 187100,
	SpellFocusedFire			= 187110,
	SpellMuleKick				= 188165,
	SpellChargingStrike			= 188269,
	SpellMightySmash			= 188147,
	SpellSlagTrap				= 188376,
	SpellAgitate				= 184793,

	/// Graggra (Court of Blood approach)
	/// https://www.wowhead.com/guide/raiding/hellfire-citadel/hellfire-high-council-strategy-guide
	SpellBlazingFelTouch		= 188450,
	SpellBlazingFelTouchAura	= 188448,
	SpellGraggraSmash			= 188516,
	SpellGraggraSmashLeap		= 188510,
};

enum eCitadelAchievements
{
	AchievementArchimondeEdge	= 10045,
	AchievementArchimondeMythic	= 10043,
	AchievementArchimondeCurve	= 10044,
	AchievementArchimondeNormal	= 10019,
	AchievementTurningTheTide	= 10057,	///< Kill 10 Hellfire Guardians then Iron Reaver
};

enum eCitadelCriterias
{
};

enum eCitadelWorldState
{
};

enum eCitadelVisuals
{
};

enum eCitadelDungeons
{
	Hellbreach					= 982,
	HallsofBlood				= 983,
	BastionofShadows			= 984,
	DestructorRise				= 985,
	BlackGate					= 986
};

static std::array<uint32, eCitadelDatas::CouncilCount> g_CouncilEntries =
{
	eCitadelCreatures::BossGurtogg,
	eCitadelCreatures::BossDia,
	eCitadelCreatures::BossJubeithos
};

#endif
