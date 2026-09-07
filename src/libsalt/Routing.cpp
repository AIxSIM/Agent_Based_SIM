/****************************************************************************/
// C++ libsalt API implementation
/****************************************************************************/


#include <vector>
#include <set>
#include <algorithm>
#include "Routing.h"
#include "Simulation.h"
#include "Vehicle.h"
#include "Object/Network/Link.h"
#include "Router/Router.h"
#include "utils/config.h"

using namespace SALT;


namespace libsalt {
    SALT::VehicleManager *Routing::getVehicleManager() {
        return Simulation::getVehicleManager();
    }

    SALT::NetworkManager *Routing::getNetworkManager() {
        return Simulation::getNetworkManager();
    }

    SALT::VehicleInterface * Routing::getVehicle(const string &vehicleID) {
        set<SALT::VehicleInterface *> runnings = getVehicleManager()->getRunningSet();
        for (SALT::VehicleInterface* r : runnings) {
            if (r && r->getMyName() == vehicleID) {
                return r;
            }
        }

        list<SALT::VehicleInterface *> standbys = getVehicleManager()->getStandbyList();
        for (SALT::VehicleInterface* s : standbys) {
            if (s && s->getMyName() == vehicleID) {
                return s;
            }
        }

        return nullptr;
    }


    int Routing::get_dua_sec_period() {
        return _get_config_info().dua_sec_period;
    }

    int Routing::get_router_time_begin() {
        return _get_config_info().time_begin;
    }

    int Routing::get_router_time_end() {
        return _get_config_info().time_end;
    }

    SALT::RouteSelect Routing::_match_rs_to_salt_rs(LibsaltRouteSelect rs) {
        if (rs == LibsaltRouteSelect::MINTRVTIME) {
            return SALT::RouteSelect::MINTRVTIME;
        } else if (rs == LibsaltRouteSelect::NARROWROAD) {
            return SALT::RouteSelect::MINTRVDIST;
        } else if (rs == LibsaltRouteSelect::WIDEROAD) {
            return SALT::RouteSelect::WIDEROAD;
        } else {
            cout << "Error: Invalid Route Select --> Thus, default to MINTRVTIME" << endl;
            return SALT::RouteSelect::MINTRVTIME;
        }
    }

    SALT::config_info Routing::_get_config_info() {
        SALT::Router * RT = Simulation::getRouter();
        return RT->get_routing_config();
    }

    void Routing::preprocess_for_router() {
        SALT::SimulationController * SC = Simulation::getSimulationController();
        SALT::Router * RT = Simulation::getRouter();
        RT->runData(SC);
    }

    void Routing::build_init_routing_table() {
        SALT::SimulationController * SC = Simulation::getSimulationController();
        SALT::Router * RT = Simulation::getRouter();
        RT->runDijkstra(SC);
    }

    void Routing::gen_od_to_trip() {
        SALT::Router * RT = Simulation::getRouter();
        RT->runOD2Trips();
    }

    void Routing::allocate_init_routes(const int _curstep) {
        SALT::Router * RT = Simulation::getRouter();
        SALT::SALTTime curstep = (SALT::SALTTime)_curstep;
        RT->runRouter(curstep);
    }

    void Routing::allocate_inter_routes(const int _curstep, bool write_file) {
        SALT::SimulationController * SC = Simulation::getSimulationController();
        SALT::Router * RT = Simulation::getRouter();
        SALT::SALTTime curstep = (SALT::SALTTime)_curstep;
        RT->MPduaRouter(curstep, SC, write_file);
    }

    void Routing::rebuild_routing_table() {
        SALT::SimulationController * SC = Simulation::getSimulationController();
        SALT::Router * RT = Simulation::getRouter();
        RT->runDijkstra(SC);
    }

    void Routing::write_file_route_xml_completed() {
        SALT::SimulationController * SC = Simulation::getSimulationController();
        SALT::Router * RT = Simulation::getRouter();
        RT->write_file_route_xml_completed(SC);
    }

    int Routing::make_routing_table(LibsaltRouteSelect rs) {
        int ret = 1;
        SALT::Router * RT = Simulation::getRouter();
        auto links = Simulation::getSimulationController()->getLinkList();
        SALT::RouteSelect RS = _match_rs_to_salt_rs(rs);
        ret = RT->make_routing_table(links, RS);
        return ret;
    }

    void Routing::update_routing_table(LibsaltRouteSelect rs) {
        SALT::SimulationController * SC = Simulation::getSimulationController();
        SALT::Router * RT = Simulation::getRouter();
        SALT::RouteSelect RS = _match_rs_to_salt_rs(rs);
        RT->update_routing_table(SC, RS);
    }

    int libsalt::Routing::recalculate_routing_table(std::string excepted, LibsaltRouteSelect rs) {
        int ret = 1;
        SALT::Router * RT = Simulation::getRouter();
        std::vector<std::string> excepted_links;
        excepted_links.push_back(excepted);
        vector<SALT::Link*> links;
        auto _links = Simulation::getSimulationController()->getLinkList();
        cout<<"Before Exception: " <<_links.size()<<endl;
        for (SALT::Link* r : _links) {
            if (find(excepted_links.begin(), excepted_links.end(),r->getID()) == excepted_links.end()){
                links.push_back(r);
            }
        }
        cout<<"After Exception: " <<links.size()<<endl;
        if (_links.size() - links.size() == excepted_links.size()){
            //map<SALT::Link*, int> mapGraph;
            for (auto k = 0; k < links.size(); ++k) {
                RT->set_mapGraph(links[k], k);
            }
            //map<int, SALT::Link*> mapGraphToLink;
            for (auto k = 0; k < links.size(); ++k) {
                RT->set_mapGraphToLink(k, links[k]);
            }
            //map<string, int> mapGraphID2IDX;
            for (auto k = 0; k < links.size(); ++k) {
                RT->set_mapGraphID2IDX(links[k]->getID(), k);
            }
            cout << "[Recalculate Routing Table]" << endl;
            //RT->make_routing_table(links);
            SALT::RouteSelect RS = _match_rs_to_salt_rs(rs);;
            RT->make_routing_table(links, RS);
        } else {
            ret = 0;
        }

        return ret;
    }

    std::string Routing::get_routing_path(std::string from, std::string to) {
        return Simulation::getRouter()->get_routing_path(from, to);
    }

    int Routing::replace_route_by_vehicle_id(std::string vehID, std::string new_route) {
        // TODO - In this case, must check the current located link of the vehicle is same to starting link of new routes for replacing routes
        SALT::VehicleInterface * vehicle = getVehicle(vehID);
        if (vehicle->replaceCurrentRoutes(new_route, getNetworkManager()) == SALT::SUCCESS){
            return 1;
        } else {
            return 0;
        }
    }

}
