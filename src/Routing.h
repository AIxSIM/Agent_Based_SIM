/****************************************************************************/
// C++ libsalt API implementation
/****************************************************************************/

#ifndef SALT_ROUTING_H
#define SALT_ROUTING_H

#pragma once

#include <vector>
#include <libsalt/LibsaltDefs.h>
#include <Controller/SimulationController.h>
#include "Router/Router.h"


/**
 * @class Routing
 * @brief C++ libsalt API implementation
 */
namespace libsalt {

    class Routing {
    public:
//        bool setConfigForRouter(std::string path);
//        SALT::Result recalculate_routing_table(std::vector<LibsaltLink *> excepted_links);
        int recalculate_routing_table(std::string excepted_link);
        std::string get_routing_path(std::string from, std::string to);
        int replace_route_by_vehicle_id(std::string vehID, std::string new_route);

    private:
        static SALT::VehicleManager* getVehicleManager();
        static SALT::NetworkManager* getNetworkManager();
//        static SALT::Router* RT;

        static SALT::VehicleInterface* getVehicle(const std::string& vehicleID);
    };

} // End of namespace libsalt

#endif //SALT_ROUTING_H

/****************************************************************************/
