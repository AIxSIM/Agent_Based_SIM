//
// Created by Hyewon Song on 26. 6. 17..
//

#ifndef SALT_EVENTINTERFACE_H
#define SALT_EVENTINTERFACE_H


#include <utils/config.h>
#include "include/Object/NetworkManager.h"


namespace SALT {
    class EventInterface {
    public:
        EventInterface();
        // V1   -----------------------------------------------
        EventInterface(ID _id, string _areaLinkListString, string _acpLinkListString, NetworkManager* _nm);
        EventInterface(ID _id, string _areaLinkListString, string _acpLinkListString, string _uniDirLinkListString, string _uniDirOppLinkListString, string _uniDirOppCap, NetworkManager* _nm);
        //-----------------------------------------------------

        // V2   -------------------------------------------------------
        EventInterface(ID _id, NetworkManager* _nm);
        //-------------------------------------------------------------


        virtual ~EventInterface();

        // V1   -----------------------------------------------
        Result enforce(SALTTime currentTime);
        //-----------------------------------------------------

        // V2   -------------------------------------------------------
        Result enforce(SALTTime currentTime, EventHandleMode mode);
        //-------------------------------------------------------------

        // V1   -----------------------------------------------
        void setRestrictLinks (string _targets);
        void setACPLinks (string _targets);
        void setUniDirectionRestrict (string _targets);
        void setUniDirectionOpposite (string _targets);

        //-----------------------------------------------------

        // V2  --------------------------------------------------------------------------------
        Result _setTargets(string _id, EventState _state, float _capacity, string _targetlinks);
        Result setTargets(string _id, string _state, string _capacity, string _targetlinks);
        Result setMyIntersections(SALTTime _intersectionST, EventInterface * _intersectionEvent);
        void _checkAndFillIntersections(SALTTime _time);
        void _buildupTargetLinks();
        // ------------------------------------------------------------------------------------

        //Temporary
        Result checkAndRemoveLinkList ();
        Result checkAndRemoveLinkListV2 ();

        void setEventStart(SALTTime _begin){
            myBegin = _begin;
        }
        void setEventEnd(SALTTime _end){
            myEnd = _end;
        }

        SALTTime getEventBegin() {  return myBegin; }
        SALTTime getEventEnd() {  return myEnd; }

        ID getEventID() { return myEventID; }

        int getMyPriority() { return priority; }
        void setMyPriority(int _priority) { priority = _priority; }


        // V1  --------------------------------------------------------------------------------
        std::vector<std::string> getMyLinkIDs() { return myLinkIDs; }
        std::vector<std::string> getEnterLinkIDs() { return myEnterLinkIDs; }
        std::vector<std::string> getUniDirLinkIDs() { return myUniDirLinkIDs; }
        std::vector<std::string> getUniDirOppLinkIDs() { return myUniDirOppLinkIDs; }
        double getUniDirOppCap() { return capacity_option; }
        bool getAdditional() { return additional; }
        double getCapacityOption() { return capacity_option; }
        //-------------------------------------------------------------------------------------

        // V2  ---------------------------------------------------------------------------------------------------------
        std::map<std::string, tuple<EventState, float, std::vector<std::string>>> getTargets() { return myTargets; }
        tuple<EventState, float, std::vector<std::string>> getTarget(string _id) { return myTargets[_id]; }
        std::map<std::string, std::pair<EventState, float>> getTargetLinks() { return myTargetLinks; }
        EventState getTargetState(string _id) { return std::get<0>(myTargets[_id]); }
        std::vector<std::string> getTargetIDs();

        // EventHandleMode getMode() { return myMode; }
        // void setMode(EventHandleMode _mode) { myMode = _mode; }


        // -------------------------------------------------------------------------------------------------------------

    private:
        ID myEventID;
        std::vector<std::string> myLinkIDs;
        std::vector<std::string> myEnterLinkIDs;

        int priority = 0;

        // Additional Event Description --> Uni-Direction-Restrict
        bool additional=false;
        std::vector<std::string> myUniDirLinkIDs;
        std::vector<std::string> myUniDirOppLinkIDs;
        double capacity_option = 1.0;
        //    std::string myVType;
        //    bool restrictMode;
        NetworkManager* myNetworkManager;
        SALTTime myBegin=0;
        SALTTime myEnd=0;
        std::map<std::string, tuple<EventState, float, std::vector<std::string>>> myTargets;     // map<TargetID, tuple<EventState (PASS/RESTRICT/EVACUATE/EVACUATE_RESTRICT), Capacity (0~2), LinkID List in Target Area>>
        // std::vector<std::pair<SALTTime, EventInterface *>> myIntersections;
        std::map<SALTTime, std::vector<EventInterface *>> myIntersections;        // map<Intersection Start Time, Heading Event> --> This Event == Following Event (by Time)
        std::map<std::string, std::pair<EventState, float>> myTargetLinks;
        std::map<std::string, std::pair<EventState, float>> myIntersectionTargetLinks;

        EventState _matchingEventState(string _state);

        // EventHandleMode myMode;

    };
};
#endif //SALT_EVENTINTERFACE_H