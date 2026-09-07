//
// Created by Hyewon Song on 25. 6. 23.
//

#ifndef SALT_EVENTATTRIBUTE_LINKACCESSCONTROL_H
#define SALT_EVENTATTRIBUTE_LINKACCESSCONTROL_H

#include "include/Object/Event/EventAttribute.h"
#include "include/Object/NetworkManager.h"


namespace SALT{

class EventAttribute_LinkAccessControl : public EventAttribute {
public:
//    EventAttribute_LinkAccessControl(const std::vector<std::string>& linkIDs, const std::string& vType, bool restrict, NetworkManager* _nm);
    EventAttribute_LinkAccessControl(ID _id, string _areaLinkListString, string _acpLinkListString, NetworkManager* _nm);
    EventAttribute_LinkAccessControl(ID _id, string _areaLinkListString, string _acpLinkListString, string _uniDirLinkListString, string _uniDirOppLinkListString, string _uniDirOppCap, NetworkManager* _nm);
    ~EventAttribute_LinkAccessControl();

    Result enforce(SALTTime currentTime) override;

    void setRestrictLinks (string _targets);
    void setACPLinks (string _targets);
    void setUniDirectionRestrict (string _targets);
    void setUniDirectionOpposite (string _targets);

    //Temporary
    Result checkAndRemoveLinkList ();

    void setEventStart(SALTTime _begin){
        myBegin = _begin;
    }

    void setEventEnd(SALTTime _end){
        myEnd = _end;
    }

    SALTTime getEventBegin() {  return myBegin; }
    SALTTime getEventEnd() {  return myEnd; }

    ID getEventID() { return myEventID; }

    std::vector<std::string> getMyLinkIDs() { return myLinkIDs; }
    std::vector<std::string> getEnterLinkIDs() { return myEnterLinkIDs; }
    std::vector<std::string> getUniDirLinkIDs() { return myUniDirLinkIDs; }
    std::vector<std::string> getUniDirOppLinkIDs() { return myUniDirOppLinkIDs; }
    double getUniDirOppCap() { return capacity_option; }
    bool getAdditional() { return additional; }
    double getCapacityOption() { return capacity_option; }

private:
    ID myEventID;
    std::vector<std::string> myLinkIDs;
    std::vector<std::string> myEnterLinkIDs;

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
};

}
#endif //SALT_EVENTATTRIBUTE_LINKACCESSCONTROL_H
