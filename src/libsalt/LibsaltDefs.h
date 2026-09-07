/****************************************************************************/
// C++ libsalt API implementation
/****************************************************************************/
#pragma once

#include <vector>
#include <limits>
#include <map>
#include <string>
#include <stdexcept>
#include <sstream>
#include <iostream>
#include <memory>

#include <utils/config.h>
#include <Object/Network/Node.h>
#include <Object/Network/Link.h>
#include <Object/Network/CellInterface.h>
#include <Object/Network/Connection.h>
#include <Object/Agent/VehicleInterface.h>
#include <Object/TrafficSignal/TrafficSignal.h>
#include <Object/TrafficSignal/TrafficSignalSchedule.h>
#include <Router/Router.h>
#include <Event/EventAttribute_LinkAccessControl.h>
#include <Event/EventInterface.h>

// #define enum_class_fix


// ===========================================================================
// class and type definitions
// ===========================================================================
namespace libsalt {

    const double INVALID_DOUBLE_VALUE = -1073741824.0;
    const int INVALID_INT_VALUE = -1073741824;

/**
 * @class LibsaltRouteSelect
 */

    enum class LibsaltRouteSelect {
        MINTRVTIME,
        NARROWROAD,
        WIDEROAD
    };

    enum class LibsaltEventState {
        PASS,					// Common Usable
        RESTRICT,				// Closed (Capacity == 0)
        EVACUATION,				// Evacuation, Road Usable for evacuation --> Controlled in CellInterface Class
        CONTROLLED_CAPACITY		// Passable with Controllable Capacity (0 < Capacity <= 2)
    };

    // enum LibsaltEventHandleMode {
    //     EVENT_START,
    //     EVENT_END,
    //     EVENT_INTERSECTION,
    //     EVENT_INTERSECTION_END
    // };



/**
 * @class LibsaltException
 */
class LibsaltException : public std::runtime_error {
public:
    /** constructor */
    LibsaltException(std::string what)
            : std::runtime_error(what) {}
};

/// @name Structures definitions
/// @{

struct LibsaltResult {
    virtual ~LibsaltResult() {}
    virtual std::string toString() {
        return "";
    }
};

/**
 * @struct LibsaltPosition
 * @brief A 3D-position
 */
struct LibsaltPosition : LibsaltResult {
    std::string toString() {
        std::ostringstream os;
        os << "LibsaltPosition(" << x << "," << y << "," << z << ")";
        return os.str();
    }
    float x, y, z;
};

/** @struct LibsaltPositionVector
  * @brief A list of positions
  */
typedef std::vector<LibsaltPosition> LibsaltPositionVector;

struct LibsaltInt : LibsaltResult {
    LibsaltInt() : value(0) {}
    LibsaltInt(int v) : value(v) {}
    std::string toString() {
        std::ostringstream os;
        os << value;
        return os.str();
    }
    int value;
};


struct LibsaltDouble : LibsaltResult {
    LibsaltDouble() : value(0.) {}
    LibsaltDouble(double v) : value(v) {}
    std::string toString() {
        std::ostringstream os;
        os << value;
        return os.str();
    }
    double value;
};


struct LibsaltString : LibsaltResult {
    LibsaltString() : value("") {}
    LibsaltString(std::string v) : value(v) {}
    std::string toString() {
        return value;
    }
    std::string value;
};


struct LibsaltStringList : LibsaltResult {
    std::string toString() {
        std::ostringstream os;
        os << "[";
        for (std::string v : value) {
            os << v << ",";
        }
        os << "]";
        return os.str();
    }
    std::vector<std::string> value;
};

/// @brief {variable->value}
typedef std::map<int, std::shared_ptr<LibsaltResult> > LibsaltResults;

}


#ifdef SWIG
%template(TLSPhaseVector) std::vector<std::pair<int,std::string>>;
%template(TLSPhasePair) std::pair<int,std::string>;
#endif

namespace libsalt{
    class TLSSchedule {
    public:
        TLSSchedule(std::string _scheduleID, int _offset, std::vector<std::pair<int,std::string>> _phasevector) {
            myID = _scheduleID;
            myOffset = _offset;
            for (auto pv : _phasevector) {
                myPhaseVector.push_back(pv);
            }
        }

        virtual ~TLSSchedule() {}

        std::string myID;
        int myOffset; // Offset time
        // TLSPhasePair := pair<duration,state>
        std::vector<std::pair<int,std::string>> myPhaseVector;

        std::vector<std::pair<int,std::string>> getPhaseVector() {  return myPhaseVector; }
        std::string getSchedulID() { return myID; }
        int getOffset() { return myOffset; }
    };
}


#ifdef SWIG
%{
    namespace swig {
        template <>  struct traits<libsalt::TLSSchedule> {
        typedef pointer_category category;
        static const char* type_name() { return "TLSSchedule"; }
     };
   }

%}
%template(TLSSchedulesMap) std::map<std::string, libsalt::TLSSchedule *>;
%template(TLSTODMap) std::map<int, std::string>;
%template(LibsaltNodeVector) std::vector<libsalt::LibsaltNode>;
%template(LibsaltLinkVector) std::vector<libsalt::LibsaltLink>;
%template(LibsaltLaneVector) std::vector<libsalt::LibsaltLane>;
%template(LibsaltCellVector) std::vector<libsalt::LibsaltCell>;
%template(LibsaltConnectionVector) std::vector<libsalt::LibsaltConnection>;
%template(LibsaltVehicleVector) std::vector<libsalt::LibsaltVehicle>;
%template(LibsaltEventVector) std::vector<libsalt::LibsaltEvent>;
%template(LibsaltEventMap) std::map<int, std::vector<libsalt::LibsaltEvent>>;
%template(LibsaltEventVectorV2) std::vector<libsalt::LibsaltEventV2>;
%template(LibsaltEventPair) std::pair<int, libsalt::LibsaltEventV2>;
%template(LibsaltEventDeque) std::deque<std::pair<int, libsalt::LibsaltEventV2>>;
%template(LibsaltEventMapV2) std::map<int,std::deque<std::pair<int, libsalt::LibsaltEventV2>>>;

#endif


namespace libsalt {

    class TLSLogic {
    public:
        TLSLogic(std::string _nodeID, std::map<std::string,TLSSchedule*> _schedules, std::map<int,std::string> _todplan)
        : myTLSID(_nodeID), myScheduleMap(_schedules), myTODPlan(_todplan){}
        virtual ~TLSLogic() {}

        std::string myTLSID;

        // Time of Day Plan
        std::map<std::string, TLSSchedule*> myScheduleMap; // (schedule name, schedule itself)
        std::map<int, std::string> myTODPlan; //ToD Plan (schedule starting time, schedule name)

        std::string getTLSID() {  return myTLSID;  }
        std::map<std::string, TLSSchedule*> getScheduleMap() {
            return myScheduleMap;
        }

        std::map<int, std::string> getTODPlan() {
            return myTODPlan;
        }

    };

    // Intersection, Junction
    class LibsaltNode {
    public:
        bool operator==(const std::string& e) const {
            return (id == e);
        }

        LibsaltNode() {}
        LibsaltNode(const std::string& _id, const float _x, const float _y, const std::string& _type)
                : id(_id), x(_x), y(_y), type(_type) {}
        LibsaltNode(SALT::Node* node) {
            id = node->getID();
            x = node->getMyLocation().x;
            y = node->getMyLocation().y;
            type = node->getMyIntersectionType();
        }
        ~LibsaltNode() {}

        std::string id; // intersection's name
        float x;
        float y;
        std::string type; // intersection's type: priority, ...

        std::string toString() {
            std::ostringstream os;
            os << "LibsaltNode(name=" << id << ", pos=(" << x << "," << y << "), type=" << type << ")";
            return os.str();
        }

    };

    class LibsaltLink {
    public:
        bool operator==(const std::string& e) const {
            return (id == e);
        }

        LibsaltLink() {}
        LibsaltLink(SALT::Link* link) {
            id = link->getID();
            fromNode = link->getFromNode()->getID();
            toNode = link->getToNode()->getID();
            numLanes = link->getNumLane();
            shape = link->getShape();
            speedLimit = link->getMySpeedLimit();
            spreadType = link->getSpreadType();
            info = link->myInfo;
            len = link->getLength();
            leftPocket = link->getMyLeftPocket();
            rightPocket = link->getMyRightPocket();
        }
        ~LibsaltLink() {}

        std::string id;
        std::string fromNode;
        std::string toNode;
        int numLanes;
        std::string shape;
        float speedLimit;
        SALT::SpreadType spreadType;
        std::string info;
        float len;
        int leftPocket;
        int rightPocket;

        std::string toString(SALT::SpreadType type) {
            if (type == SALT::SpreadType::CENTER) {
                return "center";
            } else {
                return "right";
            }
        }

        std::string toString() {
            std::ostringstream os;
            os << "LibsaltLink(id=" << id << ", fromNode=" << fromNode << ", toNode=" << toNode << ", numLanes=" << numLanes << ", shape=" << shape << ", speedLimit=" << speedLimit << ", spreadType="
            << toString(spreadType) << ", info=" << info << ", len=" << len << ", leftPocket=" << leftPocket << ", rightPocket=" << rightPocket << ")";
            return os.str();
        }
    };

    class LibsaltConnection {
    public:
        LibsaltConnection() {}
        LibsaltConnection(SALT::Connection* c) {
            fromLink = c->getFromLink();
            toLink = c->getToLink();
            fromLane = c->getFromLane();
            toLane = c->getToLane();
            linkIndex = c->getLinkIndex();
            direction.x = c->getDirection().x;
            direction.y = c->getDirection().y;
            rotationDir = c->getRotationDir();
            info = c->myInfo;
        }
        ~LibsaltConnection() {}

        LibsaltLink fromLink;
        LibsaltLink toLink;
        int fromLane;
        int toLane;
        int linkIndex;
        LibsaltPosition direction;
        std::string rotationDir;
        std::string info;

        std::string toString() {
            std::ostringstream os;
            os << "LibsaltConnection(from=(" << fromLink.id << ":" << fromLane << "), to=(" << toLink.id << ":" << toLane <<
                "), direction=(" << direction.x << ", " << direction.y << "), rotationDir=" << rotationDir << ", linkIndex=" << linkIndex << ", info=" << info << ")";
            return os.str();
        }
    };

    class LibsaltLane {
    public:
        bool operator<(const LibsaltLane& e) const {
            return (id < e.id);
        }
        LibsaltLane() {}
        LibsaltLane(const std::string& laneID) {
            istringstream f(laneID);
            std::string s;
            getline(f, s, '_');
            link = s;
            getline(f, s, '_');
            lane = std::stoi(s);
        }
        LibsaltLane(const std::string& linkID, int lane) {
            id = linkID + "_" + std::to_string(lane);
            link = linkID;
            lane = lane;
        }
        ~LibsaltLane() {}

        std::string id;
        std::string link;
        int lane;

        std::string toString() {
            return id;
        }
    };

    class LibsaltCell {
    public:
        bool operator<(const LibsaltCell& e) const {
            return (id < e.id);
        }
        LibsaltCell() {}
        LibsaltCell(const std::string& cellID) {
            id = cellID;
            istringstream f(cellID);
            std::string s;
            getline(f, s, '_');
            link = s;
            getline(f, s, '_');
            section = std::stoi(s);
            getline(f, s, '_');
            lane = std::stoi(s);
        }
        LibsaltCell(const std::string& linkID, int section, int lane) {
            id = linkID + "_" + std::to_string(section) + "_" + std::to_string(lane);
            link = linkID;
            section = section;
            lane = lane;
        }
        LibsaltCell(SALT::CellInterface *r) {
            id = r->getID();
            link = r->getMyLink()->getID();
            section = r->getSection();
            lane = r->getLane();
        }
        ~LibsaltCell() {}

        std::string id;
        std::string link;
        int section;
        int lane;

        std::string toString() {
            return id;
        }
    };

    class LibsaltRoute {
    public:
        LibsaltRoute() {}
        LibsaltRoute(std::string routeString) {
            istringstream f(routeString);
            std::string s;
            while (getline(f, s, ' ')) {
                route.push_back(s);
            }
        }
        ~LibsaltRoute() {}

        std::vector<std::string> route;

        std::string toString() {
            std::ostringstream os;

            std::ostringstream str;
            for (int i = 0; i < route.size(); i++) {
                if (i == 0) {
                    str << route[i];
                } else {
                    str << " " << route[i];
                }
            }

            os << "LibsaltRoute(#links=" << route.size() << ", links=" << str.str() << ")";
            return os.str();
        }
    };

    class LibsaltVehicle {
    public:
        LibsaltVehicle() {}
        LibsaltVehicle(SALT::VehicleInterface* v) {
            id = v->getMyName();
            shape = v->getMyShape();
            speed = v->getMySpeed();
            SALT::VehicleClass clazz = v->getMyClass();
            if (clazz == SALT::BUS) {
                vClass = "bus";
            } else {
                vClass = "passenger";
            }
            departTime = v->getMyDepartTime();
            cellIndex = v->getMyCellIndex();
            length = v->getMyLength();
            route = v->getRouteString();
            rsPref = _match_salt_rs_to_rs(v->getMyRSPreference());

        }
        ~LibsaltVehicle() {}

        std::string id;
        std::string shape;
        float speed;
        std::string vClass;
        int departTime;
        int cellIndex;
        float length;
        std::string route;
        LibsaltRouteSelect rsPref;

        int routeIndex;

        std::string toString() {
            std::ostringstream os;
            os << "LibsaltVehicle(id=" << id << ", vClass=" << vClass << ", cellIndex=" << cellIndex << ", length=" << length << ", departTime=" << departTime << ", speed=" << speed << ", shape=" << shape << ", route=" << route << ")";
            return os.str();
        }

        LibsaltRouteSelect _match_salt_rs_to_rs(SALT::RouteSelect rs) {
            if (rs == SALT::RouteSelect::MINTRVTIME) {
                return LibsaltRouteSelect::MINTRVTIME;
            } else if (rs == SALT::RouteSelect::MINTRVDIST) {
                return LibsaltRouteSelect::NARROWROAD;
            } else if (rs == SALT::RouteSelect::WIDEROAD) {
                return LibsaltRouteSelect::WIDEROAD;
            } else {
                cout << "Error: Invalid Route Select --> Thus, default to MINTRVTIME" << endl;
                return LibsaltRouteSelect::MINTRVTIME;
            }
        }
    };

    class LibsaltEvent {
    public:
        LibsaltEvent() {}
        // LibsaltEvent(SALT::EventAttribute_LinkAccessControl* attr) {
        //     myEventID = attr->getEventID();
        //     myLinkIDs = attr->getMyLinkIDs();
        //     myEnterLinkIDs = attr->getEnterLinkIDs();
        //     additional=attr->getAdditional();
        //     myUniDirLinkIDs = attr->getUniDirLinkIDs();
        //     myUniDirOppLinkIDs = attr->getUniDirOppLinkIDs();
        //     capacity_option = attr->getCapacityOption();
        //     myBegin = attr->getEventBegin();
        //     myEnd = attr->getEventEnd();
        // }
        LibsaltEvent(SALT::EventInterface* attr) {
            myEventID = attr->getEventID();
            myLinkIDs = attr->getMyLinkIDs();
            myEnterLinkIDs = attr->getEnterLinkIDs();
            additional=attr->getAdditional();
            myUniDirLinkIDs = attr->getUniDirLinkIDs();
            myUniDirOppLinkIDs = attr->getUniDirOppLinkIDs();
            capacity_option = attr->getCapacityOption();
            myBegin = attr->getEventBegin();
            myEnd = attr->getEventEnd();
        }

        ~LibsaltEvent() {}

        std::string myEventID;
        std::vector<std::string> myLinkIDs;
        std::vector<std::string> myEnterLinkIDs;

        // Additional Event Description --> Uni-Direction-Restrict
        bool additional=false;
        std::vector<std::string> myUniDirLinkIDs;
        std::vector<std::string> myUniDirOppLinkIDs;
        double capacity_option = 1.0;

        int myBegin=0;
        int myEnd=0;

        std::string toString() {
            std::ostringstream os;
            os << "LibsaltEvent(id=" << myEventID << ", myBegin=" << myBegin << ", myEnd=" << myEnd << ", additional=" << additional << ", # of myLinkIDs=" << myLinkIDs.size() << ", # of myEnterLinkIDs=" << myEnterLinkIDs.size() <<
                ", # of myUniDirLinkIDs=" << myUniDirLinkIDs.size() << ", # of myUniDirOppLinkIDs=" << myUniDirOppLinkIDs.size() << ", capacity_option=" << capacity_option <<  ")";
            return os.str();
        }
    };

    class LibsaltEventV2 {
    public:
        LibsaltEventV2() {}
        // LibsaltEvent(SALT::EventAttribute_LinkAccessControl* attr) {
        //     myEventID = attr->getEventID();
        //     myLinkIDs = attr->getMyLinkIDs();
        //     myEnterLinkIDs = attr->getEnterLinkIDs();
        //     additional=attr->getAdditional();
        //     myUniDirLinkIDs = attr->getUniDirLinkIDs();
        //     myUniDirOppLinkIDs = attr->getUniDirOppLinkIDs();
        //     capacity_option = attr->getCapacityOption();
        //     myBegin = attr->getEventBegin();
        //     myEnd = attr->getEventEnd();
        // }
        LibsaltEventV2(SALT::EventInterface* attr) {
            myEventID = attr->getEventID();
            // myLinkIDs = attr->getMyLinkIDs();
            // myEnterLinkIDs = attr->getEnterLinkIDs();
            // additional=attr->getAdditional();
            // myUniDirLinkIDs = attr->getUniDirLinkIDs();
            // myUniDirOppLinkIDs = attr->getUniDirOppLinkIDs();
            // capacity_option = attr->getCapacityOption();
            myBegin = attr->getEventBegin();
            myEnd = attr->getEventEnd();
            setMyTargets(attr->getTargets());
            setMyTargetLinks(attr->getTargetLinks());
        }

        ~LibsaltEventV2() {}

        std::string myEventID;
        // std::vector<std::string> myLinkIDs;
        // std::vector<std::string> myEnterLinkIDs;

        std::map<std::string, tuple<LibsaltEventState, float, std::vector<std::string>>> myTargets;
        std::map<std::string, std::pair<LibsaltEventState, float>> myTargetLinks;
        std::map<std::string, std::pair<LibsaltEventState, float>> myIntersectionTargetLinks;

        // Additional Event Description --> Uni-Direction-Restrict
        // bool additional=false;
        // std::vector<std::string> myUniDirLinkIDs;
        // std::vector<std::string> myUniDirOppLinkIDs;
        // double capacity_option = 1.0;

        int myBegin=0;
        int myEnd=0;

        std::string toString() {
            std::ostringstream os;
            os << "LibsaltEvent(id=" << myEventID << ", myBegin=" << myBegin << ", myEnd=" << myEnd <<
                ", # of myTargets==" << myTargets.size() <<", # of myTargetLinks==" << myTargetLinks.size() << ")";
            return os.str();
        }

        void setMyTargets(std::map<std::string, tuple<SALT::EventState, float, std::vector<std::string>>> _targets) {
            for (auto it = _targets.begin(); it != _targets.end(); ++it) {
                std::string targetID = it->first;
                tuple<SALT::EventState, float, std::vector<std::string>> target = it->second;
                LibsaltEventState state = _match_salt_es_to_es(std::get<0>(target));
                float capacity = std::get<1>(target);
                std::vector<std::string> targetLinkIDs = std::get<2>(target);
                myTargets[targetID] = std::make_tuple(state, capacity, targetLinkIDs);
            }
        }

        void setMyTargetLinks(std::map<std::string, std::pair<SALT::EventState, float>> _targets) {
            for (auto it = _targets.begin(); it != _targets.end(); ++it) {
                std::string linkID = it->first;
                std::pair<SALT::EventState, float> target = it->second;
                LibsaltEventState state = _match_salt_es_to_es(target.first);
                float capacity = target.second;
                myTargetLinks[linkID] = std::make_pair(state, capacity);
            }
        }

        void setIntersectionTargetLinks(std::map<std::string, std::pair<SALT::EventState, float>> _targets) {
            for (auto it = _targets.begin(); it != _targets.end(); ++it) {
                std::string linkID = it->first;
                std::pair<SALT::EventState, float> target = it->second;
                LibsaltEventState state = _match_salt_es_to_es(target.first);
                float capacity = target.second;
                myIntersectionTargetLinks[linkID] = std::make_pair(state, capacity);
            }
        }

        LibsaltEventState _match_salt_es_to_es(SALT::EventState es) {
            if (es == SALT::EventState::PASS) {
                return LibsaltEventState::PASS;
            } else if (es == SALT::EventState::EVACUATION) {
                return LibsaltEventState::EVACUATION;
            } else if (es == SALT::EventState::RESTRICT) {
                return LibsaltEventState::RESTRICT;
            } else if (es == SALT::EventState::CONTROLLED_CAPACITY) {
                return LibsaltEventState::CONTROLLED_CAPACITY;
            } else {
                cout << "Error: Invalid Route Select --> Thus, default to PASS" << endl;
                return LibsaltEventState::PASS;
            }
        }
    };
} // end of namespace libsalt
