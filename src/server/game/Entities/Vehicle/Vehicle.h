////////////////////////////////////////////////////////////////////////////////
//
// Project-Hellscream https://hellscream.org
// Copyright (C) 2018-2020 Project-Hellscream-6.2
// Discord https://discord.gg/CWCF3C9
//
////////////////////////////////////////////////////////////////////////////////

#ifndef __TRINITY_VEHICLE_H
#define __TRINITY_VEHICLE_H

#include "ObjectDefines.h"
#include "VehicleDefines.h"
#include "Unit.h"
#include <deque>

struct VehicleEntry;
class Unit;

typedef std::set<uint64> GuidSet;
class VehicleJoinEvent;

class Vehicle : public TransportBase
{
    public:
        explicit Vehicle(Unit* unit, VehicleEntry const* vehInfo, uint32 creatureEntry);
        virtual ~Vehicle();

        void Install();
        void Uninstall(bool dismount = false);
        void Reset(bool evading = false);
        void InstallAllAccessories(bool evading);
        void ApplyAllImmunities();
        void InstallAccessory(uint32 entry, int8 seatId, bool minion, uint8 type, uint32 summonTime);   //! May be called from scripts

        Unit* GetBase() const { return _me; }
        VehicleEntry const* GetVehicleInfo() const { return _vehicleInfo; }
        uint32 GetCreatureEntry() const { return _creatureEntry; }

        bool HasEmptySeat(int8 seatId) const;
        Unit* GetPassenger(int8 seatId) const;
        SeatMap::const_iterator GetNextEmptySeat(int8 seatId, bool next) const;
        uint8 GetAvailableSeatCount() const;
        uint32 GetRecAura() const { return _recAura; }
        bool CheckCustomCanEnter();
        bool AddPassenger(Unit* passenger, int8 seatId = -1);
        void RemovePassenger(Unit* passenger);
        void RelocatePassengers();
        void RemoveAllPassengers(bool dismount = false);
        void Dismiss();
        bool IsVehicleInUse() const;

        inline bool ArePassengersSpawnedByAI() const { return _passengersSpawnedByAI; }
        void SetPassengersSpawnedByAI(bool passengersSpawnedByAI) { _passengersSpawnedByAI = passengersSpawnedByAI; }

        inline bool CanBeCastedByPassengers() const { return _canBeCastedByPassengers; }
        void SetCanBeCastedByPassengers(bool canBeCastedByPassengers) { _canBeCastedByPassengers = canBeCastedByPassengers; }

        SeatMap Seats;  ///< The collection of all seats on the vehicle. Including vacant ones.

        VehicleSeatEntry const* GetSeatForPassenger(Unit const* passenger);

        protected:
            friend class VehicleJoinEvent;
            uint32 UsableSeatNum;         ///< Number of seats that match VehicleSeatEntry::UsableByPlayer, used for proper display flags

    private:
        enum Status
        {
            STATUS_NONE,
            STATUS_INSTALLED,
            STATUS_UNINSTALLING
        };

        SeatMap::iterator GetSeatIteratorForPassenger(Unit* passenger);
        void InitMovementInfoForBase();

        /// This method transforms supplied transport offsets into global coordinates
        void CalculatePassengerPosition(float& x, float& y, float& z, float& o);

        /// This method transforms supplied global coordinates into local offsets
        void CalculatePassengerOffset(float& x, float& y, float& z, float& o);

        Unit* _me;  ///< The underlying unit with the vehicle kit. Can be player or creature.
        VehicleEntry const* _vehicleInfo;   ///< DBC data for vehicle
        GuidSet vehiclePlayers;
        uint32 _creatureEntry;         ///< Can be different than the entry of _me in case of players
        Status _status;     ///< Internal variable for sanity checks
        uint32 _recAura;

        bool _passengersSpawnedByAI;
        bool _canBeCastedByPassengers;

        std::deque<VehicleJoinEvent*> _pendingJoinEvents;   ///< Collection of delayed join events for prospective passengers
        void CancelJoinEvent(VehicleJoinEvent* e);
};

class VehicleJoinEvent : public BasicEvent
{
    friend class Vehicle;
protected:
    VehicleJoinEvent(Vehicle* v, Unit* u) : Target(v), Passenger(u), Seat(Target->Seats.end()) {}
    bool Execute(uint64, uint32);
    void Abort(uint64);

    Vehicle* Target;
    Unit* Passenger;
    SeatMap::iterator Seat;
};
#endif
