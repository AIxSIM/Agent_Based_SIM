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
        void preprocess_for_router();
        void build_init_routing_table();
        void gen_od_to_trip();
        void allocate_init_routes(const int _curstep);
        void allocate_inter_routes(const int _curstep, bool write_file = false);
        void rebuild_routing_table();
        void write_file_route_xml_completed();

        int make_routing_table(LibsaltRouteSelect rs = LibsaltRouteSelect::MINTRVTIME);
        void update_routing_table(LibsaltRouteSelect rs = LibsaltRouteSelect::MINTRVTIME);
        int recalculate_routing_table(std::string excepted_link, LibsaltRouteSelect rs = LibsaltRouteSelect::MINTRVTIME);
        std::string get_routing_path(std::string from, std::string to);
        int replace_route_by_vehicle_id(std::string vehID, std::string new_route);

        int get_dua_sec_period();
        int get_router_time_begin();
        int get_router_time_end();

    private:
        static SALT::VehicleManager* getVehicleManager();
        static SALT::NetworkManager* getNetworkManager();
//        static SALT::Router* RT;

        static SALT::VehicleInterface* getVehicle(const std::string& vehicleID);
        static SALT::RouteSelect _match_rs_to_salt_rs(LibsaltRouteSelect rs);

        SALT::config_info _get_config_info();
    };

} // End of namespace libsalt

#endif //SALT_ROUTING_H

/****************************************************************************/
