////////////////////////////////////////////////////////////////////////////////
//
// Project-Hellscream https://hellscream.org
// Copyright (C) 2018-2020 Project-Hellscream-6.2
// Discord https://discord.gg/CWCF3C9
//
////////////////////////////////////////////////////////////////////////////////

#include "Common.h"
#include "Log.h"
#include "ObjectMgr.h"
#include "Vehicle.h"
#include "Unit.h"
#include "Util.h"
#include "WorldPacket.h"
#include "ScriptMgr.h"
#include "CreatureAI.h"
#include "ZoneScript.h"
#include "SpellMgr.h"
#include "SpellInfo.h"
#include "MoveSplineInit.h"
#include "EventProcessor.h"
#include "Player.h"
#include "Battleground.h"

Vehicle::Vehicle(Unit* unit, VehicleEntry const* vehInfo, uint32 creatureEntry) : UsableSeatNum(0), _me(unit), _vehicleInfo(vehInfo), _creatureEntry(creatureEntry),
 _status(STATUS_NONE), _passengersSpawnedByAI(false), _canBeCastedByPassengers(false)
{
    for (uint32 i = 0; i < MAX_VEHICLE_SEATS; ++i)
    {
        if (uint32 seatId = _vehicleInfo->m_seatID[i])
            if (VehicleSeatEntry const* veSeat = sVehicleSeatStore.LookupEntry(seatId))
            {
                Seats.insert(std::make_pair(i, VehicleSeat(veSeat)));
                if (veSeat->CanEnterOrExit())
                    ++UsableSeatNum;
            }
    }

    // Set or remove correct flags based on available seats. Will overwrite db data (if wrong).
    if (UsableSeatNum)
        _me->SetFlag(UNIT_FIELD_NPC_FLAGS, (_me->GetTypeId() == TYPEID_PLAYER ? UNIT_NPC_FLAG_PLAYER_VEHICLE : UNIT_NPC_FLAG_SPELLCLICK));
    else
        _me->RemoveFlag(UNIT_FIELD_NPC_FLAGS, (_me->GetTypeId() == TYPEID_PLAYER ? UNIT_NPC_FLAG_PLAYER_VEHICLE : UNIT_NPC_FLAG_SPELLCLICK));

    InitMovementInfoForBase();
}

Vehicle::~Vehicle()
{
    /// @Uninstall must be called before this.
    ASSERT(_status == STATUS_UNINSTALLING);
    for (SeatMap::const_iterator itr = Seats.begin(); itr != Seats.end(); ++itr)
        ASSERT(!itr->second.Passenger);
}

void Vehicle::Install()
{
    if (Creature* creature = _me->ToCreature())
    {
        switch (_vehicleInfo->m_PowerDisplayID)
        {
            case POWER_STEAM:
            case POWER_HEAT:
            case POWER_BLOOD:
            case POWER_OOZE:
            case POWER_WRATH:
                _me->setPowerType(POWER_ENERGY);
                _me->SetMaxPower(POWER_ENERGY, 100);
                break;
            case POWER_PYRITE:
                _me->setPowerType(POWER_ENERGY);
                _me->SetMaxPower(POWER_ENERGY, 50);
                break;
            default:
                for (uint32 i = 0; i < MAX_SPELL_VEHICLE; ++i)
                {
                    if (!creature->m_spells[i])
                        continue;

                    SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(creature->m_spells[i]);
                    if (!spellInfo)
                        continue;

                    for (auto itr : spellInfo->SpellPowers)
                    {
                        if (itr->PowerType == POWER_ENERGY)
                        {
                            _me->setPowerType(POWER_ENERGY);
                            _me->SetMaxPower(POWER_ENERGY, 100);
                            break;
                        }
                    }
                }
                break;
        }
    }

    _status = STATUS_INSTALLED;
    if (GetBase()->GetTypeId() == TYPEID_UNIT)
        sScriptMgr->OnInstall(this);
}

void Vehicle::InstallAllAccessories(bool evading)
{
    if (ArePassengersSpawnedByAI())
        return;

    if (GetBase()->IsPlayer() || !evading)
        RemoveAllPassengers();   // We might have aura's saved in the DB with now invalid casters - remove

    VehicleAccessoryList const* accessories = sObjectMgr->GetVehicleAccessoryList(this);
    if (!accessories)
        return;

    for (VehicleAccessoryList::const_iterator itr = accessories->begin(); itr != accessories->end(); ++itr)
        if (!evading || itr->IsMinion)  // only install minions on evade mode
            InstallAccessory(itr->AccessoryEntry, itr->SeatId, itr->IsMinion, itr->SummonedType, itr->SummonTime);
}

void Vehicle::Uninstall(bool dismount/* = false*/)
{
    /// @Prevent recursive uninstall call. (Bad script in OnUninstall/OnRemovePassenger/PassengerBoarded hook.)
    if (_status == STATUS_UNINSTALLING && !GetBase()->HasUnitTypeMask(UNIT_MASK_MINION))
    {
        TC_LOG_DEBUG("entities.vehicle", "Vehicle GuidLow: %u, Entry: %u attempts to uninstall, but already has STATUS_UNINSTALLING! "
            "Check Uninstall/PassengerBoarded script hooks for errors.", _me->GetGUIDLow(), _me->GetEntry());
        return;
    }
    _status = STATUS_UNINSTALLING;
    TC_LOG_DEBUG("entities.vehicle", "Vehicle::Uninstall Entry: %u, GuidLow: %u", _creatureEntry, _me->GetGUIDLow());
    RemoveAllPassengers(dismount);

    if (GetBase() && GetBase()->GetTypeId() == TYPEID_UNIT)
        sScriptMgr->OnUninstall(this);
}

void Vehicle::Reset(bool evading /*= false*/)
{
    if (GetBase()->GetTypeId() == TYPEID_PLAYER)
    {
        InstallAllAccessories(evading);
        return;
    }

    if (GetBase()->GetTypeId() != TYPEID_UNIT)
        return;

    TC_LOG_DEBUG("entities.vehicle", "Vehicle::Reset (Entry: %u, GuidLow: %u, DBGuid: %u)", GetCreatureEntry(), _me->GetGUIDLow(), _me->ToCreature()->GetDBTableGUIDLow());

    ApplyAllImmunities();
    InstallAllAccessories(evading);

    sScriptMgr->OnReset(this);
}

void Vehicle::ApplyAllImmunities()
{
    // This couldn't be done in DB, because some spells have MECHANIC_NONE

    // Vehicles should be immune on Knockback ...
    _me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_KNOCK_BACK, true);
    _me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_KNOCK_BACK_DEST, true);

    // Mechanical units & vehicles ( which are not Bosses, they have own immunities in DB ) should be also immune on healing ( exceptions in switch below )
    if (_me->ToCreature() && _me->ToCreature()->GetCreatureTemplate()->type == CREATURE_TYPE_MECHANICAL && !_me->ToCreature()->isWorldBoss())
    {
        // Heal & dispel ...
        _me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_HEAL, true);
        _me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_HEAL_PCT, true);
        _me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_DISPEL, true);
        _me->ApplySpellImmune(0, IMMUNITY_STATE, SPELL_AURA_PERIODIC_HEAL, true);

        // ... Shield & Immunity grant spells ...
        _me->ApplySpellImmune(0, IMMUNITY_STATE, SPELL_AURA_SCHOOL_IMMUNITY, true);
        _me->ApplySpellImmune(0, IMMUNITY_STATE, SPELL_AURA_MOD_UNATTACKABLE, true);
        _me->ApplySpellImmune(0, IMMUNITY_STATE, SPELL_AURA_SCHOOL_ABSORB, true);
        _me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_SHIELD, true);
        _me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_IMMUNE_SHIELD, true);
        _me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_BANISH, true);

        // ... Resistance, Split damage, Change stats ...
        _me->ApplySpellImmune(0, IMMUNITY_STATE, SPELL_AURA_DAMAGE_SHIELD, true);
        _me->ApplySpellImmune(0, IMMUNITY_STATE, SPELL_AURA_SPLIT_DAMAGE_PCT, true);
        _me->ApplySpellImmune(0, IMMUNITY_STATE, SPELL_AURA_MOD_RESISTANCE, true);
        _me->ApplySpellImmune(0, IMMUNITY_STATE, SPELL_AURA_MOD_STAT, true);
        _me->ApplySpellImmune(0, IMMUNITY_STATE, SPELL_AURA_MOD_DAMAGE_PERCENT_TAKEN, true);
    }

    // Different immunities for vehicles goes below
    switch (GetVehicleInfo()->m_ID)
    {
        // code below prevents a bug with movable cannons
        case 160: // Strand of the Ancients
        case 244: // Wintergrasp
        case 321: // Pilgrim's Bounty chairs
        case 510: // Isle of Conquest
            _me->SetControlled(true, UNIT_STATE_ROOT);
            // why we need to apply this? we can simple add immunities to slow mechanic in DB
            _me->ApplySpellImmune(0, IMMUNITY_STATE, SPELL_AURA_MOD_DECREASE_SPEED, true);
            break;
        default:
            break;
    }
}

void Vehicle::RemoveAllPassengers(bool dismount/* = false*/)
{
    TC_LOG_DEBUG("entities.vehicle", "Vehicle::RemoveAllPassengers. Entry: %u, GuidLow: %u", _creatureEntry, _me->GetGUIDLow());

    /// Setting to_Abort to true will cause @VehicleJoinEvent::Abort to be executed on next @Unit::UpdateEvents call
    /// This will properly "reset" the pending join process for the passenger.
    while (!_pendingJoinEvents.empty())
    {
        VehicleJoinEvent* e = _pendingJoinEvents.front();
        e->to_Abort = true;
        _pendingJoinEvents.pop_front();
    }

    // Passengers always cast an aura with SPELL_AURA_CONTROL_VEHICLE on the vehicle
    // We just remove the aura and the unapply handler will make the target leave the vehicle.
    // We don't need to iterate over Seats
    _me->RemoveAurasByType(SPELL_AURA_CONTROL_VEHICLE);

    if (dismount)
    {
        for (SeatMap::iterator itr = Seats.begin(); itr != Seats.end(); ++itr)
        {
            if (itr->second.Passenger)
            {
                if (Unit* passenger = ObjectAccessor::GetUnit(*GetBase(), itr->second.Passenger))
                    passenger->ExitVehicle();
                else
                    itr->second.Passenger = 0;
            }
        }
    }
    // Following the above logic, this assertion should NEVER fail.
    // Even in 'hacky' cases, there should at least be VEHICLE_SPELL_RIDE_HARDCODED on us.
    // SeatMap::const_iterator itr;
    // for (itr = Seats.begin(); itr != Seats.end(); ++itr)
    //    ASSERT(!itr->second.passenger);
}

/**
 * @fn bool Vehicle::HasEmptySeat(int8 seatId) const
 *
 * @brief Checks if vehicle's seat specified by 'seatId' is empty.
 *
 * @author Machiavelli
 * @date 17-2-2013
 *
 * @param seatId Identifier for the seat.
 *
 * @return true if empty seat, false if not.
 */

bool Vehicle::HasEmptySeat(int8 seatId) const
{
    SeatMap::const_iterator seat = Seats.find(seatId);
    if (seat == Seats.end())
        return false;
    return !seat->second.Passenger;
}

Unit* Vehicle::GetPassenger(int8 seatId) const
{
    SeatMap::const_iterator seat = Seats.find(seatId);
    if (seat == Seats.end())
        return NULL;

    return ObjectAccessor::GetUnit(*GetBase(), seat->second.Passenger);
}

SeatMap::const_iterator Vehicle::GetNextEmptySeat(int8 seatId, bool next) const
{
    SeatMap::const_iterator seat = Seats.find(seatId);
    if (seat == Seats.end())
        return seat;

    while (seat->second.Passenger || (!seat->second.SeatInfo->CanEnterOrExit() && !seat->second.SeatInfo->IsUsableByOverride()))
    {
        if (next)
        {
            if (++seat == Seats.end())
                seat = Seats.begin();
        }
        else
        {
            if (seat-- == Seats.begin())
                seat = Seats.end();
        }

        // Make sure we don't loop indefinetly
        if (seat->first == seatId)
        return Seats.end();
    }

    return seat;
}

void Vehicle::InstallAccessory(uint32 entry, int8 seatId, bool minion, uint8 type, uint32 summonTime)
{
    /// @Prevent adding accessories when vehicle is uninstalling. (Bad script in OnUninstall/OnRemovePassenger/PassengerBoarded hook.)
    if (_status == STATUS_UNINSTALLING)
    {
        TC_LOG_DEBUG("entities.vehicle", "Vehicle (GuidLow: %u, DB GUID: %u, Entry: %u) attempts to install accessory (Entry: %u) on seat %i with STATUS_UNINSTALLING! "
            "Check Uninstall/PassengerBoarded script hooks for errors.", _me->GetGUIDLow(),
            (_me->GetTypeId() == TYPEID_UNIT ? _me->ToCreature()->GetDBTableGUIDLow() : _me->GetGUIDLow()), GetCreatureEntry(), entry, (int32)seatId);
        return;
    }

    TC_LOG_DEBUG("entities.vehicle", "Vehicle (GuidLow: %u, DB Guid: %u, Entry %u): installing accessory (Entry: %u) on seat: %i",
        _me->GetGUIDLow(), (_me->GetTypeId() == TYPEID_UNIT ? _me->ToCreature()->GetDBTableGUIDLow() : _me->GetGUIDLow()), GetCreatureEntry(),
        entry, (int32)seatId);

        TempSummon* accessory = _me->SummonCreature(entry, *_me, TempSummonType(type), summonTime);
        ASSERT(accessory);

        if (minion)
            accessory->AddUnitTypeMask(UNIT_MASK_ACCESSORY);

        // Force enter for force vehicle aura - 296
        if (GetRecAura())
            accessory->_EnterVehicle(this, -1);
        (void)_me->HandleSpellClick(accessory, seatId);

        /// If for some reason adding accessory to vehicle fails it will unsummon in
         /// @VehicleJoinEvent::Abort
}

bool Vehicle::CheckCustomCanEnter()
{
    switch (GetCreatureEntry())
    {
        case 56682: // Keg in Stormstout Brewery
            return true;
        case 46185: // Sanitron
            return true;
    }

    return false;
}

bool Vehicle::AddPassenger(Unit* unit, int8 seatId)
{
    /// @Prevent adding passengers when vehicle is uninstalling. (Bad script in OnUninstall/OnRemovePassenger/PassengerBoarded hook.)
    if (_status == STATUS_UNINSTALLING)
    {
        TC_LOG_DEBUG("entities.vehicle", "Passenger GuidLow: %u, Entry: %u, attempting to board vehicle GuidLow: %u, Entry: %u during uninstall! SeatId: %i",
            unit->GetGUIDLow(), unit->GetEntry(), _me->GetGUIDLow(), _me->GetEntry(), (int32)seatId);
        return false;
    }

    TC_LOG_DEBUG("entities.vehicle", "Unit %s scheduling enter vehicle entry %u id %u dbguid %u seat %d", unit->GetName(), _me->GetEntry(), _vehicleInfo->m_ID, _me->GetGUIDLow(), seatId);

    if (unit->IsPlayer() && unit->GetMap()->IsBattleArena())
        return false;

    // The seat selection code may kick other passengers off the vehicle.
    // While the validity of the following may be arguable, it is possible that when such a passenger
    // exits the vehicle will dismiss. That's why the actual adding the passenger to the vehicle is scheduled
    // asynchronously, so it can be cancelled easily in case the vehicle is uninstalled meanwhile.
    SeatMap::iterator seat;
    VehicleJoinEvent* e = new VehicleJoinEvent(this, unit);
    unit->m_Events.AddEvent(e, unit->m_Events.CalculateTime(0));
    _pendingJoinEvents.push_back(e);

    if (seatId < 0) // no specific seat requirement
    {
        for (seat = Seats.begin(); seat != Seats.end(); ++seat)
            if (!seat->second.Passenger && (seat->second.SeatInfo->CanEnterOrExit() || seat->second.SeatInfo->IsUsableByOverride() || CheckCustomCanEnter()))
                break;

        if (seat == Seats.end()) // no available seat
        {
            CancelJoinEvent(e);
            return false;
        }

        e->Seat = seat;
    }
    else
    {
        seat = Seats.find(seatId);
        if (seat == Seats.end())
        {
            CancelJoinEvent(e);
            return false;
        }

        e->Seat = seat;

        if (seat->second.Passenger)
        {
            Unit* passenger = ObjectAccessor::GetUnit(*GetBase(), seat->second.Passenger);
            ASSERT(passenger);
            passenger->ExitVehicle();
        }

        ASSERT(!seat->second.Passenger);
    }
}

void Vehicle::RemovePassenger(Unit* unit)
{
    if (unit->GetVehicle() != this)
        return;

    SeatMap::iterator seat = GetSeatIteratorForPassenger(unit);
    ASSERT(seat != Seats.end());

    TC_LOG_DEBUG("entities.vehicle", "Unit %s exit vehicle entry %u id %u dbguid %u seat %d", unit->GetName(), _me->GetEntry(), _vehicleInfo->m_ID, _me->GetGUIDLow(), (int32)seat->first);

    seat->second.Passenger = 0;
    if (seat->second.SeatInfo->CanEnterOrExit() && ++UsableSeatNum)
        _me->SetFlag(UNIT_FIELD_NPC_FLAGS, (_me->GetTypeId() == TYPEID_PLAYER ? UNIT_NPC_FLAG_PLAYER_VEHICLE : UNIT_NPC_FLAG_SPELLCLICK));

    unit->ClearUnitState(UNIT_STATE_ONVEHICLE);

    if (_me->GetTypeId() == TYPEID_UNIT && unit->IsPlayer() && seat->first == 0 && seat->second.SeatInfo->m_flags & VEHICLE_SEAT_FLAG_CAN_CONTROL)
        _me->RemoveCharmedBy(unit);

    if (_me->IsInWorld())
    {
        unit->m_movementInfo.t_pos.Relocate(0.0f, 0.0f, 0.0f, 0.0f);
        unit->m_movementInfo.t_time = 0;
        unit->m_movementInfo.t_seat = 0;
        unit->m_movementInfo.t_guid = 0;
    }

    // only for flyable vehicles
    if (unit->IsFlying())
        _me->CastSpell(unit, VEHICLE_SPELL_PARACHUTE, true);

    if (_me->GetTypeId() == TYPEID_UNIT && _me->ToCreature()->IsAIEnabled)
        _me->ToCreature()->AI()->PassengerBoarded(unit, seat->first, false);

    if (Creature* l_Passenger = unit->ToCreature())
    {
        if (l_Passenger->IsAIEnabled)
            l_Passenger->AI()->OnVehicleExited(_me);
    }

    if (GetBase()->GetTypeId() == TYPEID_UNIT)
        sScriptMgr->OnRemovePassenger(this, unit);
}

//! Must be called after m_base::Relocate
void Vehicle::RelocatePassengers()
{
    ASSERT(_me->GetMap());

    // not sure that absolute position calculation is correct, it must depend on vehicle pitch angle
    for (SeatMap::const_iterator itr = Seats.begin(); itr != Seats.end(); ++itr)
    {
        if (Unit* passenger = ObjectAccessor::GetUnit(*GetBase(), itr->second.Passenger))
        {
            ASSERT(passenger->IsInWorld());

            float px, py, pz, po;
            passenger->m_movementInfo.t_pos.GetPosition(px, py, pz, po);
            CalculatePassengerPosition(px, py, pz, po);
            passenger->UpdatePosition(px, py, pz, po);
        }
    }
}

void Vehicle::Dismiss()
{
    if (GetBase()->GetTypeId() != TYPEID_UNIT)
        return;

    TC_LOG_DEBUG("entities.vehicle", "Vehicle::Dismiss Entry: %u, GuidLow %u, DBGuid: %u", GetCreatureEntry(), _me->GetGUIDLow(), _me->ToCreature()->GetDBTableGUIDLow());
    Uninstall();
    GetBase()->ToCreature()->DespawnOrUnsummon();
}

bool Vehicle::IsVehicleInUse() const
{
    for (SeatMap::const_iterator itr = Seats.begin(); itr != Seats.end(); ++itr)
        if (itr->second.Passenger)
            return true;

    return false;
}

void Vehicle::InitMovementInfoForBase()
{
    uint32 vehicleFlags = GetVehicleInfo()->m_flags;

    if (vehicleFlags & VEHICLE_FLAG_NO_STRAFE)
        _me->AddExtraUnitMovementFlag(MOVEMENTFLAG2_NO_STRAFE);
    if (vehicleFlags & VEHICLE_FLAG_NO_JUMPING)
        _me->AddExtraUnitMovementFlag(MOVEMENTFLAG2_NO_JUMPING);
    if (vehicleFlags & VEHICLE_FLAG_FULLSPEEDTURNING)
        _me->AddExtraUnitMovementFlag(MOVEMENTFLAG2_FULL_SPEED_TURNING);
    if (vehicleFlags & VEHICLE_FLAG_ALLOW_PITCHING)
        _me->AddExtraUnitMovementFlag(MOVEMENTFLAG2_ALWAYS_ALLOW_PITCHING);
    if (vehicleFlags & VEHICLE_FLAG_FULLSPEEDPITCHING)
        _me->AddExtraUnitMovementFlag(MOVEMENTFLAG2_FULL_SPEED_PITCHING);
}

VehicleSeatEntry const* Vehicle::GetSeatForPassenger(Unit const* passenger)
{
    if (!passenger)
        return NULL;

    SeatMap::iterator itr;
    for (itr = Seats.begin(); itr != Seats.end(); ++itr)
        if (itr->second.Passenger == passenger->GetGUID())
            return itr->second.SeatInfo;

    return NULL;
}

/**
 * @fn SeatMap::iterator Vehicle::GetSeatIteratorForPassenger(Unit* passenger)
 *
 * @brief Gets seat iterator for specified passenger.
 *
 * @author Machiavelli
 * @date 17-2-2013
 *
 * @param [in,out] passenger Passenger to look up.
 *
 * @return The seat iterator for specified passenger if it's found on the vehicle. Otherwise Seats.end() (invalid iterator).
 */

SeatMap::iterator Vehicle::GetSeatIteratorForPassenger(Unit* passenger)
{
    SeatMap::iterator itr;
    for (itr = Seats.begin(); itr != Seats.end(); ++itr)
        if (itr->second.Passenger == passenger->GetGUID())
            return itr;

    return Seats.end();
}

/**
 * @fn uint8 Vehicle::GetAvailableSeatCount() const
 *
 * @brief Gets the available seat count.
 *
 * @author Machiavelli
 * @date 17-2-2013
 *
 * @return The available seat count.
 */

uint8 Vehicle::GetAvailableSeatCount() const
{
    uint8 ret = 0;
    SeatMap::const_iterator itr;
    for (itr = Seats.begin(); itr != Seats.end(); ++itr)
        if (!itr->second.Passenger && (itr->second.SeatInfo->CanEnterOrExit() || itr->second.SeatInfo->IsUsableByOverride()))
            ++ret;

    return ret;
}

void Vehicle::CalculatePassengerPosition(float& p_X, float& p_Y, float& p_Z, float& p_O)
{
    float l_InX     = p_X;
    float l_InY     = p_Y;
    float l_InZ     = p_Z;
    float l_InO     = p_O;

    Unit* l_Base    = GetBase();
    Position l_Pos  = *l_Base;

    if (Vehicle* l_ParentTransport = l_Base->GetVehicle())
    {
        l_Pos = l_Base->m_movementInfo.t_pos;
        l_ParentTransport->CalculatePassengerPosition(l_Pos.m_positionX, l_Pos.m_positionY, l_Pos.m_positionZ, l_Pos.m_orientation);
    }

    p_Z = l_Pos.m_orientation + l_InO;
    p_X = l_Pos.m_positionX + l_InX * std::cos(l_Pos.m_orientation) - l_InY * std::sin(l_Pos.m_orientation);
    p_Y = l_Pos.m_positionY + l_InY * std::cos(l_Pos.m_orientation) + l_InX * std::sin(l_Pos.m_orientation);
    p_Z = l_Pos.m_positionZ + l_InZ;
}

void Vehicle::CalculatePassengerOffset(float& p_X, float& p_Y, float& p_Z, float& p_O)
{
    Unit* l_Base = GetBase();

    p_O -= l_Base->GetOrientation();
    p_Z -= l_Base->GetPositionZ();
    p_Y -= l_Base->GetPositionY();
    p_X -= l_Base->GetPositionX();

    float l_InX = p_X;
    float l_InY = p_Y;

    p_Y = (l_InY - l_InX * tan(l_Base->m_orientation)) / (cos(l_Base->m_orientation) + std::sin(l_Base->m_orientation) * tan(l_Base->m_orientation));
    p_X = (l_InX + l_InY * tan(l_Base->m_orientation)) / (cos(l_Base->m_orientation) + std::sin(l_Base->m_orientation) * tan(l_Base->m_orientation));
}

/**
 * @fn void Vehicle::CancelJoinEvent(VehicleJoinEvent* e)
 *
 * @brief Aborts delayed @VehicleJoinEvent objects.
 * 		  Implies that the related unit will not be boarding the vehicle after all.
 *
 * @author Machiavelli
 * @date 17-2-2013
 *
 * @param [in,out] e The VehicleJoinEvent* to process.
 */

void Vehicle::CancelJoinEvent(VehicleJoinEvent* e)
{
    e->to_Abort = true;
    _pendingJoinEvents.pop_back();
}

/**
 * @fn void Vehicle::RemovePendingEvent(VehicleJoinEvent* e)
 *
 * @brief Removes @VehicleJoinEvent objects from pending join event store.
 *        This method only removes it after it's executed or aborted to prevent leaving
 *        pointers to deleted events.
 *
 * @author Shauren
 * @date 22-2-2013
 *
 * @param [in] e The VehicleJoinEvent* to remove from pending event store.
 */

void Vehicle::RemovePendingEvent(VehicleJoinEvent* e)
{
    for (std::deque<VehicleJoinEvent*>::iterator itr = _pendingJoinEvents.begin(); itr != _pendingJoinEvents.end(); ++itr)
    {
        if (*itr == e)
        {
            _pendingJoinEvents.erase(itr);
            break;
        }
    }
}

/**
 * @fn bool VehicleJoinEvent::Execute(uint64, uint32)
 *
 * @brief Actually adds the passenger @Passenger to vehicle @Target.
 *
 * @author Machiavelli
 * @date 17-2-2013
 *
 * @param parameter1 Unused
 * @param parameter2 Unused.
 *
 * @return true, cannot fail.
 *
 */

bool VehicleJoinEvent::Execute(uint64, uint32)
{
    ASSERT(Passenger->IsInWorld());
    ASSERT(Target->GetBase()->IsInWorld());

    Seat->second.Passenger = Passenger->GetGUID();
    if (Seat->second.SeatInfo->CanEnterOrExit())
    {
        ASSERT(Target->UsableSeatNum);
        --(Target->UsableSeatNum);
        if (!Target->UsableSeatNum)
        {
            if (Target->GetBase()->GetTypeId() == TYPEID_PLAYER)
                Target->GetBase()->RemoveFlag(UNIT_FIELD_NPC_FLAGS, UNIT_NPC_FLAG_PLAYER_VEHICLE);
            else
                Target->GetBase()->RemoveFlag(UNIT_FIELD_NPC_FLAGS, UNIT_NPC_FLAG_SPELLCLICK);
        }
    }

    Passenger->InterruptNonMeleeSpells(false);
    Passenger->RemoveAurasByType(SPELL_AURA_MOUNTED);

    Player* player = Passenger->ToPlayer();
    if (player)
    {
        // drop flag 
        if (Battleground* bg = player->GetBattleground())
            bg->EventPlayerDroppedFlag(player);

        player->StopCastingCharm();
        player->StopCastingBindSight();
        player->SendOnCancelExpectedVehicleRideAura();
        player->UnsummonPetTemporaryIfAny();
    }

    if (Seat->second.SeatInfo->m_flags && !(Seat->second.SeatInfo->m_flags & VEHICLE_SEAT_FLAG_ALLOW_TURNING))
        Passenger->AddUnitState(UNIT_STATE_ONVEHICLE);

    //Passenger->AddUnitMovementFlag(MOVEMENTFLAG_ONTRANSPORT);
    VehicleSeatEntry const* veSeat = Seat->second.SeatInfo;
    Passenger->m_movementInfo.t_pos.Relocate(veSeat->m_attachmentOffsetX, veSeat->m_attachmentOffsetY, veSeat->m_attachmentOffsetZ);
    Passenger->m_movementInfo.t_time = 0; // 1 for player
    Passenger->m_movementInfo.t_seat = Seat->first;
    Passenger->m_movementInfo.t_guid = Target->GetBase()->GetGUID();

    // Hackfix
    switch (veSeat->m_ID)
    {
    case 10882:
        Passenger->m_movementInfo.t_pos.m_positionX = 15.0f;
        Passenger->m_movementInfo.t_pos.m_positionY = 0.0f;
        Passenger->m_movementInfo.t_pos.m_positionZ = 30.0f;
        break;
    default:
        break;
    }

    //Old Trinity method
    //if (Target->GetBase()->GetTypeId() == TYPEID_UNIT && Passenger->GetTypeId() == TYPEID_PLAYER &&
    //    Seat->second.SeatInfo->m_flags & VEHICLE_SEAT_FLAG_CAN_CONTROL)
    //    ASSERT(Target->GetBase()->SetCharmedBy(Passenger, CHARM_TYPE_VEHICLE))

    if (Target->GetBase()->GetTypeId() == TYPEID_UNIT && Passenger->GetTypeId() == TYPEID_PLAYER)
    {
        if (Seat->second.SeatInfo->m_flags & VEHICLE_SEAT_FLAG_CAN_CONTROL
            && !Target->GetBase()->SetCharmedBy(Passenger, CHARM_TYPE_VEHICLE))
        {
            return false;
            //ASSERT(false);
        }
        else if (Seat->second.SeatInfo->m_flags & VEHICLE_SEAT_FLAG_UNK2)
        {
            Passenger->Dismount();
            Passenger->ToPlayer()->SetClientControl(Target->GetBase(), 1);
            Passenger->ToPlayer()->SetMover(Target->GetBase());
            Passenger->ToPlayer()->SetViewpoint(Target->GetBase(), true);
        }
    }

    Passenger->SendClearTarget();                            // SMSG_BREAK_TARGET
    Passenger->SetControlled(true, UNIT_STATE_ROOT);         // SMSG_FORCE_ROOT - In some cases we send SMSG_SPLINE_MOVE_ROOT here (for creatures)
    // also adds MOVEMENTFLAG_ROOT

    Movement::MoveSplineInit init(Passenger);
    init.DisableTransportPathTransformations();
    init.MoveTo(veSeat->m_attachmentOffsetX, veSeat->m_attachmentOffsetY, veSeat->m_attachmentOffsetZ, false, true);
    init.SetFacing(0.0f);
    init.SetTransportEnter();
    init.Launch();

    if (Target->GetBase()->GetTypeId() == TYPEID_UNIT)
    {
        if (Target->GetBase()->ToCreature()->IsAIEnabled)
            Target->GetBase()->ToCreature()->AI()->PassengerBoarded(Passenger, Seat->first, true);
        sScriptMgr->OnAddPassenger(Target, Passenger, Seat->first);

        // Actually quite a redundant hook. Could just use OnAddPassenger and check for unit typemask inside script.
        if (Passenger->HasUnitTypeMask(UNIT_MASK_ACCESSORY))
         sScriptMgr->OnInstallAccessory(Target, Passenger->ToCreature());

        // update all passenger's positions
        //Passenger's spline OR vehicle movement will update positions
        //RelocatePassengers(_me->GetPositionX(), _me->GetPositionY(), _me->GetPositionZ(), _me->GetOrientation());
    }
    return true;
}

/**
 * @fn void VehicleJoinEvent::Abort(uint64)
 *
 * @brief Aborts the event. Implies that unit @Passenger will not be boarding vehicle @Target after all.
 *
 * @author Machiavelli
 * @date 17-2-2013
 *
 * @param parameter1 Unused
 */

void VehicleJoinEvent::Abort(uint64)
{
    TC_LOG_DEBUG("entities.vehicle", "Passenger GuidLow: %u, Entry: %u, board on vehicle GuidLow: %u, Entry: %u SeatId: %i cancelled",
        Passenger->GetGUIDLow(), Passenger->GetEntry(), Target->GetBase()->GetGUIDLow(), Target->GetBase()->GetEntry(), (int32)Seat->first);

    /// @SPELL_AURA_CONTROL_VEHICLE auras can be applied even when the passenger is not (yet) on the vehicle. 
    /// When this code is triggered it means that something went wrong in @Vehicle::AddPassenger, and we should remove
    /// the aura manually.
    Target->GetBase()->RemoveAurasByType(SPELL_AURA_CONTROL_VEHICLE, Passenger->GetGUID());

    if (Passenger->HasUnitTypeMask(UNIT_MASK_ACCESSORY))
        Passenger->ToTempSummon()->UnSummon();
}
