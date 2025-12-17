//
// Created by Hyewon Song on 25. 7. 2.
//

#ifndef SALT_VEHICLEV4_H
#define SALT_VEHICLEV4_H


#include <Object/Agent/VehicleInterface.h>
#include <Object/Agent/Route.h>
#include <utils/config.h>
#include <string>

namespace SALT {
    class Route;

    class VehicleV4 : public VehicleInterface {
    public:
        VehicleV4();

        VehicleV4(Route* _route, string _departTime, int _routeIndex,  ID _ID="", VehicleClass _vehClass=PASSENGER, VehicleType _vehType=COMMON);
//        VehicleV4(map<RouteSelect,Route*> _routes, string _departTime, int _routeIndex,  ID _ID="", VehicleClass _vehClass=PASSENGER, VehicleType _vehType=COMMON);

        virtual ~VehicleV4();

        virtual void updateStatus(SALTTime nextTime);

    };
}


#endif //SALT_VEHICLEV4_H
