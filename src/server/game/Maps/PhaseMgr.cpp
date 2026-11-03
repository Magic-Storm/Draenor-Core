////////////////////////////////////////////////////////////////////////////////
//
// Project-Hellscream https://hellscream.org
// Copyright (C) 2018-2020 Project-Hellscream-6.2
// Discord https://discord.gg/CWCF3C9
//
////////////////////////////////////////////////////////////////////////////////

#include "PhaseMgr.h"
#include "Chat.h"
#include "ObjectMgr.h"
#include "ConditionMgr.h"
#include "DBCStores.h"
#ifndef CROSS
#include "GarrisonMgr.hpp"
#include "Group.h"
#include "ObjectAccessor.h"
#endif /* not CROSS */

//////////////////////////////////////////////////////////////////
// Updating

PhaseMgr::PhaseMgr(Player* _player) : player(_player), phaseData(_player), _UpdateFlags(0)
{
    _PhaseDefinitionStore = sObjectMgr->GetPhaseDefinitionStore();
    _SpellPhaseStore = sObjectMgr->GetSpellPhaseStore();
}

void PhaseMgr::Update()
{
    if (IsUpdateInProgress())
        return;

    if (_UpdateFlags & PHASE_UPDATE_FLAG_CLIENTSIDE_CHANGED)
        phaseData.SendPhaseshiftToPlayer();

    if (_UpdateFlags & PHASE_UPDATE_FLAG_SERVERSIDE_CHANGED)
        phaseData.SendPhaseMaskToPlayer();

    _UpdateFlags = 0;
}
void PhaseMgr::ForceMapShiftUpdate()
{
    phaseData.SendPhaseshiftToPlayer();
}
void PhaseMgr::RemoveUpdateFlag(PhaseUpdateFlag updateFlag)
{
    _UpdateFlags &= ~updateFlag;

    if (updateFlag == PHASE_UPDATE_FLAG_ZONE_UPDATE)
    {
        // Update zone changes
        if (phaseData.HasActiveDefinitions())
        {
            phaseData.ResetDefinitions();
            _UpdateFlags |= (PHASE_UPDATE_FLAG_CLIENTSIDE_CHANGED | PHASE_UPDATE_FLAG_SERVERSIDE_CHANGED);
        }

        if (_PhaseDefinitionStore->find(player->GetZoneId()) != _PhaseDefinitionStore->end())
            Recalculate();
    }

    Update();
}

/////////////////////////////////////////////////////////////////
// Notifier

void PhaseMgr::NotifyConditionChanged(PhaseUpdateData const& updateData)
{
    if (NeedsPhaseUpdateWithData(updateData))
    {
        Recalculate();
        Update();
    }
}

//////////////////////////////////////////////////////////////////
// Phasing Definitions

void PhaseMgr::Recalculate()
{
    if (phaseData.HasActiveDefinitions())
    {
        phaseData.ResetDefinitions();
        _UpdateFlags |= (PHASE_UPDATE_FLAG_CLIENTSIDE_CHANGED | PHASE_UPDATE_FLAG_SERVERSIDE_CHANGED);
    }

    PhaseDefinitionStore::const_iterator itr = _PhaseDefinitionStore->find(player->GetZoneId());
    if (itr != _PhaseDefinitionStore->end())
    {
        for (PhaseDefinitionContainer::const_iterator phase = itr->second.begin(); phase != itr->second.end(); ++phase)
        {
            if (CheckDefinition(&(*phase)))
            {
                phaseData.AddPhaseDefinition(&(*phase));

                if (phase->phasemask)
                    _UpdateFlags |= PHASE_UPDATE_FLAG_SERVERSIDE_CHANGED;

                if (phase->phaseId || phase->terrainswapmap)
                    _UpdateFlags |= PHASE_UPDATE_FLAG_CLIENTSIDE_CHANGED;

                if (phase->IsLastDefinition())
                    break;
            }
        }
    }

#ifndef CROSS
    if (player->GetGarrison() && (player->GetMapId() == MS::Garrison::Globals::BaseMap || player->IsInGarrison()))
        _UpdateFlags |= PHASE_UPDATE_FLAG_CLIENTSIDE_CHANGED;
#endif /* not CROSS */
}

inline bool PhaseMgr::CheckDefinition(PhaseDefinition const* phaseDefinition)
{
    return sConditionMgr->IsObjectMeetPhaseCondition(phaseDefinition->zoneId, phaseDefinition->entry, player);
}

bool PhaseMgr::NeedsPhaseUpdateWithData(PhaseUpdateData const updateData) const
{
    PhaseDefinitionStore::const_iterator itr = _PhaseDefinitionStore->find(player->GetZoneId());
    if (itr != _PhaseDefinitionStore->end())
    {
        for (PhaseDefinitionContainer::const_iterator phase = itr->second.begin(); phase != itr->second.end(); ++phase)
        {
            if (ConditionContainer const* conditionList = sConditionMgr->GetConditionsForPhaseDefinition(phase->zoneId, phase->entry))
            {
                for (ConditionContainer::const_iterator condition = conditionList->begin(); condition != conditionList->end(); ++condition)
                    if (updateData.IsConditionRelated(*condition))
                        return true;
            }
        }
    }

    return false;
}

//////////////////////////////////////////////////////////////////
// Auras

void PhaseMgr::RegisterPhasingAuraEffect(AuraEffect const* auraEffect)
{
    std::list<PhaseInfo> phases;

    if (auraEffect->GetAuraType() == SPELL_AURA_PHASE)
    {
        PhaseInfo phaseInfo;

        if (auraEffect->GetMiscValue())
        {
            _UpdateFlags |= PHASE_UPDATE_FLAG_SERVERSIDE_CHANGED;
            phaseInfo.phasemask = auraEffect->GetMiscValue();
        }
        else
        {
            SpellPhaseStore::const_iterator itr = _SpellPhaseStore->find(auraEffect->GetId());
            if (itr != _SpellPhaseStore->end())
            {
                if (itr->second.phasemask)
                {
                    _UpdateFlags |= PHASE_UPDATE_FLAG_SERVERSIDE_CHANGED;
                    phaseInfo.phasemask = itr->second.phasemask;
                }

				if (itr->second.terrainswapmap)
					phaseInfo.terrainswapmap = itr->second.terrainswapmap;

				if (itr->second.worldmaparea)
					phaseInfo.worldmaparea = itr->second.worldmaparea;
            }
        }


		phaseInfo.phaseId = auraEffect->GetMiscValueB();

		if (phaseInfo.NeedsClientSideUpdate())
			_UpdateFlags |= PHASE_UPDATE_FLAG_CLIENTSIDE_CHANGED;

        phases.push_back(phaseInfo);
    }
    else if (auraEffect->GetAuraType() == SPELL_AURA_PHASE_GROUP)
    {
        // This core stores the phase id of SPELL_AURA_PHASE in MiscValueB.
        // Phase groups follow the same slot; MiscValue is the fallback.
        uint32 group = auraEffect->GetMiscValueB();
        std::vector<uint32> const* groupPhases = &GetPhasesForGroup(group);
        if (groupPhases->empty())
        {
            group = auraEffect->GetMiscValue();
            groupPhases = &GetPhasesForGroup(group);
        }

        for (uint32 phaseId : *groupPhases)
        {
            PhaseInfo phaseInfo;
            phaseInfo.phaseId = phaseId;
            if (phaseInfo.NeedsClientSideUpdate())
                _UpdateFlags |= PHASE_UPDATE_FLAG_CLIENTSIDE_CHANGED;
            phases.push_back(phaseInfo);
        }
    }

    for (auto itr = phases.begin(); itr != phases.end(); ++itr)
        phaseData.AddAuraInfo(auraEffect->GetId(), *itr);

    Update();
}

void PhaseMgr::UnRegisterPhasingAuraEffect(AuraEffect const* auraEffect)
{
    _UpdateFlags |= phaseData.RemoveAuraInfo(auraEffect->GetId());

    Update();
}

//////////////////////////////////////////////////////////////////
// Commands

void PhaseMgr::SendDebugReportToPlayer(Player* const debugger)
{
    ChatHandler(debugger).PSendSysMessage(LANG_PHASING_REPORT_STATUS, player->GetName(), player->GetZoneId(), player->getLevel(), player->GetTeamId(), _UpdateFlags);

    PhaseDefinitionStore::const_iterator itr = _PhaseDefinitionStore->find(player->GetZoneId());
    if (itr == _PhaseDefinitionStore->end())
        ChatHandler(debugger).PSendSysMessage(LANG_PHASING_NO_DEFINITIONS, player->GetZoneId());
    else
    {
        for (PhaseDefinitionContainer::const_iterator phase = itr->second.begin(); phase != itr->second.end(); ++phase)
        {
            if (CheckDefinition(&(*phase)))
                ChatHandler(debugger).PSendSysMessage(LANG_PHASING_SUCCESS, phase->entry, phase->IsNegatingPhasemask() ? "negated Phase" : "Phase", phase->phasemask);
            else
                ChatHandler(debugger).PSendSysMessage(LANG_PHASING_FAILED, phase->phasemask, phase->entry, phase->zoneId);

            if (phase->IsLastDefinition())
            {
                ChatHandler(debugger).PSendSysMessage(LANG_PHASING_LAST_PHASE, phase->phasemask, phase->entry, phase->zoneId);
                break;
            }
        }
    }

    ChatHandler(debugger).PSendSysMessage(LANG_PHASING_LIST, phaseData._PhasemaskThroughDefinitions, phaseData._PhasemaskThroughAuras, phaseData._CustomPhasemask);

    ChatHandler(debugger).PSendSysMessage(LANG_PHASING_PHASEMASK, phaseData.GetPhaseMaskForSpawn(), player->GetPhaseMask());
}

void PhaseMgr::SetCustomPhase(uint32 const phaseMask)
{
    phaseData._CustomPhasemask = phaseMask;

    _UpdateFlags |= PHASE_UPDATE_FLAG_SERVERSIDE_CHANGED;

    Update();
}

//////////////////////////////////////////////////////////////////
// Phase Data

void PhaseData::GetActivePhases(std::set<uint32>& phases) const
{
	// Phases from Auras.
	if (!spellPhaseInfo.empty())
        for (PhaseInfoContainer::const_iterator phaseInfoContainer = spellPhaseInfo.begin(); phaseInfoContainer != spellPhaseInfo.end(); ++phaseInfoContainer)
        {
            for (auto itr = phaseInfoContainer->second.begin(); itr != phaseInfoContainer->second.end(); ++itr)
            {
                if (itr->phaseId)
                    phases.insert(itr->phaseId);
            }
        }

	// Phase definitions from DB.
	if (!activePhaseDefinitions.empty())
		for (std::list<PhaseDefinition const*>::const_iterator l_IT = activePhaseDefinitions.begin(); l_IT != activePhaseDefinitions.end(); ++l_IT)
			if ((*l_IT)->phaseId)
				phases.insert((*l_IT)->phaseId);
}

uint32 PhaseData::GetCurrentPhasemask() const
{
    if (player->isGameMaster())
        return PHASEMASK_ANYWHERE;

    if (_CustomPhasemask)
        return _CustomPhasemask;

    return GetPhaseMaskForSpawn();
}

inline uint32 PhaseData::GetPhaseMaskForSpawn() const
{
    uint32 const phase = (_PhasemaskThroughDefinitions | _PhasemaskThroughAuras);
    return (phase ? phase : PHASEMASK_NORMAL);
}

void PhaseData::SendPhaseMaskToPlayer()
{
    // Server side update
    uint32 const phasemask = GetCurrentPhasemask();
    if (player->GetPhaseMask() == phasemask)
        return;

    player->SetPhaseMask(phasemask, false);

    if (player->IsVisible())
        player->UpdateObjectVisibility();
}

void PhaseData::SendPhaseshiftToPlayer()
{
	// Client side update
	std::set<uint32> l_PhaseIDs;
	std::set<uint32> l_TerrainSwaps;
	std::set<uint32> l_InactiveTerrainSwap;

	for (PhaseInfoContainer::const_iterator phaseInfoContainer = spellPhaseInfo.begin(); phaseInfoContainer != spellPhaseInfo.end(); ++phaseInfoContainer)
	{
        for (auto itr = phaseInfoContainer->second.begin(); itr != phaseInfoContainer->second.end(); ++itr)
        {
            if (itr->terrainswapmap)
                l_TerrainSwaps.insert(itr->terrainswapmap);

            if (itr->phaseId)
                l_PhaseIDs.insert(itr->phaseId);
        }
	}
	
	// Phase Definitions
	for (std::list<PhaseDefinition const*>::const_iterator l_IT = activePhaseDefinitions.begin(); l_IT != activePhaseDefinitions.end(); ++l_IT)
	{
		if ((*l_IT)->phaseId)
			l_PhaseIDs.insert((*l_IT)->phaseId);

		if ((*l_IT)->terrainswapmap)
			l_TerrainSwaps.insert((*l_IT)->terrainswapmap);
	}

#ifndef CROSS
	MS::Garrison::Manager* l_Garrison = player->GetGarrison();
	if (player->IsInPartyGarrison())
	{
		if (Group* l_Group = player->GetGroup())
		{
			if (Player* l_Leader = ObjectAccessor::FindPlayer(l_Group->GetLeaderGUID()))
			{
				if (l_Leader != player && l_Leader->GetGarrison() && l_Leader->GetGarrison()->GetGarrisonSiteLevelEntry()
					&& l_Leader->GetMapId() == player->GetMapId())
					l_Garrison = l_Leader->GetGarrison();
			}
		}
	}

	if (l_Garrison && l_Garrison->GetGarrisonSiteLevelEntry())
	{
		bool l_InThisGarrison = player->GetMapId() == uint32(l_Garrison->GetGarrisonSiteLevelEntry()->MapID);
		if (player->GetMapId() == MS::Garrison::Globals::BaseMap || l_InThisGarrison)
		{
			l_InactiveTerrainSwap.insert(l_Garrison->GetGarrisonSiteLevelEntry()->MapID);
			l_Garrison->GetTerrainSwaps(l_TerrainSwaps);
		}

		bool l_InThisShipyard = player->GetMapId() == uint32(l_Garrison->GetShipyardMapId());
		if (l_Garrison->HasShipyard() && (player->GetMapId() == MS::Garrison::Globals::BaseMap || l_InThisShipyard || l_InThisGarrison))
		{
			/// Blizz are weird people
			l_InactiveTerrainSwap.insert(MS::Garrison::ShipyardMapId::Alliance);
			l_InactiveTerrainSwap.insert(MS::Garrison::ShipyardMapId::Horde);
			l_Garrison->GetShipyardTerainSwaps(l_TerrainSwaps);
		}
	}
#endif

	player->SetPhaseIds(l_PhaseIDs);
	player->GetSession()->SendSetPhaseShift(l_PhaseIDs, l_TerrainSwaps, l_InactiveTerrainSwap);
}

void PhaseData::AddPhaseDefinition(PhaseDefinition const* phaseDefinition)
{
    if (phaseDefinition->IsOverwritingExistingPhases())
    {
        activePhaseDefinitions.clear();
        _PhasemaskThroughDefinitions = phaseDefinition->phasemask;
    }
    else
    {
        if (phaseDefinition->IsNegatingPhasemask())
            _PhasemaskThroughDefinitions &= ~phaseDefinition->phasemask;
        else
            _PhasemaskThroughDefinitions |= phaseDefinition->phasemask;
    }

    activePhaseDefinitions.push_back(phaseDefinition);
}

void PhaseData::AddAuraInfo(uint32 const spellId, PhaseInfo phaseInfo)
{
    if (phaseInfo.phasemask)
        _PhasemaskThroughAuras |= phaseInfo.phasemask;

    spellPhaseInfo[spellId].push_back(phaseInfo);
}

uint32 PhaseData::RemoveAuraInfo(uint32 const spellId)
{
    PhaseInfoContainer::iterator rAura = spellPhaseInfo.find(spellId);
    if (rAura == spellPhaseInfo.end())
        return 0;

    bool serverUpdated = false;
    bool clientUpdated = false;

    for (auto phase = rAura->second.begin(); phase != rAura->second.end(); ++phase)
    {
        if (phase->NeedsClientSideUpdate())
            clientUpdated = true;
        if (phase->NeedsServerSideUpdate())
            serverUpdated = true;
    }

    // Drop the aura before rebuilding. A phase-id or phase-group aura has no phasemask,
    // and leaving it here kept the client phase after the spell expired.
    spellPhaseInfo.erase(rAura);

    uint32 updateflag = 0;

    if (serverUpdated)
    {
        _PhasemaskThroughAuras = 0;
        updateflag |= PHASE_UPDATE_FLAG_SERVERSIDE_CHANGED;

        for (PhaseInfoContainer::const_iterator itr = spellPhaseInfo.begin(); itr != spellPhaseInfo.end(); ++itr)
            for (auto ph = itr->second.begin(); ph != itr->second.end(); ++ph)
                _PhasemaskThroughAuras |= ph->phasemask;
    }

    if (clientUpdated)
        updateflag |= PHASE_UPDATE_FLAG_CLIENTSIDE_CHANGED;

    return updateflag;
}

//////////////////////////////////////////////////////////////////
// Phase Update Data

void PhaseUpdateData::AddQuestUpdate(uint32 const questId)
{
    AddConditionType(CONDITION_QUESTREWARDED);
    AddConditionType(CONDITION_QUESTTAKEN);
    AddConditionType(CONDITION_QUEST_COMPLETE);
    AddConditionType(CONDITION_QUEST_NONE);

    _questId = questId;
}

bool PhaseUpdateData::IsConditionRelated(Condition const* condition) const
{
    switch (condition->ConditionType)
    {
        case CONDITION_QUESTREWARDED:
        case CONDITION_QUESTTAKEN:
        case CONDITION_QUEST_COMPLETE:
        case CONDITION_QUEST_NONE:
            return condition->ConditionValue1 == _questId && ((1 << condition->ConditionType) & _conditionTypeFlags);
        default:
            return (1 << condition->ConditionType) & _conditionTypeFlags;
    }
}

bool PhaseMgr::IsConditionTypeSupported(ConditionTypes const conditionType)
{
    switch (conditionType)
    {
        case CONDITION_QUESTREWARDED:
        case CONDITION_QUESTTAKEN:
        case CONDITION_QUEST_COMPLETE:
        case CONDITION_QUEST_NONE:
        case CONDITION_TEAM:
        case CONDITION_CLASS:
        case CONDITION_RACE:
        case CONDITION_INSTANCE_INFO:
        case CONDITION_LEVEL:
            return true;
        default:
            return false;
    }
}

void PhaseMgr::GetActivePhases(std::set<uint32>& phases) const
{
	phaseData.GetActivePhases(phases);
}
