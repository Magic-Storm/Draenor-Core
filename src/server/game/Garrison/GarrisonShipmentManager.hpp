////////////////////////////////////////////////////////////////////////////////
//
// Project-Hellscream https://hellscream.org
// Copyright (C) 2018-2020 Project-Hellscream-6.2
// Discord https://discord.gg/CWCF3C9
//
////////////////////////////////////////////////////////////////////////////////

#ifndef CROSS
#ifndef GARRISON_SHIPMENT_MANAGER_HPP_GARRISON
# define GARRISON_SHIPMENT_MANAGER_HPP_GARRISON

#include "Common.h"

namespace MS { namespace Garrison
{
    /// Shipment manager class
    class ShipmentManager
    {
        private:
            /// Constructor
            ShipmentManager();
            /// Destructor
            ~ShipmentManager();

        public:
            static ShipmentManager* instance()
            {
                static ShipmentManager* instance = new ShipmentManager();
                return instance;
            }
            /// Init shipment manager
            void Init();

            /// Get shipment ID for specific building & player
            uint32 GetShipmentIDForBuilding(uint32 p_BuildingID, Player* p_Target, bool p_ForStartWorkOrder = false);

        private:
            std::map<uint32, uint32> m_ShipmentPerBuildingType;         ///< Shipment ID per building
            std::map<uint32, uint32> m_QuestShipmentPerBuildingType;    ///< Quest shipment ID per building

    };

}   ///< namespace Garrison
}   ///< namespace MS

#define sGarrisonShipmentManager MS::Garrison::ShipmentManager::instance()

#endif  ///< GARRISON_SHIPMENT_MANAGER_HPP_GARRISON
#endif
