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

    int libsalt::Routing::recalculate_routing_table(std::string excepted) {
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
            RT->make_routing_table(links);
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