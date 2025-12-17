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
    EventAttribute_LinkAccessControl(string _areaLinkListString, string _acpLinkListString, NetworkManager* _nm);
    ~EventAttribute_LinkAccessControl();

    Result enforce(SALTTime currentTime) override;

    void setRestrictLinks (string _targets);
    void setACPLinks (string _targets);

    //Temporary
    Result checkAndRemoveLinkList ();

    void setEventStart(SALTTime _begin){
        myBegin = _begin;
    }

    void setEventEnd(SALTTime _end){
        myEnd = _end;
    }

private:
    std::vector<std::string> myLinkIDs;
    std::vector<std::string> myEnterLinkIDs;
//    std::string myVType;
//    bool restrictMode;
    NetworkManager* myNetworkManager;
    SALTTime myBegin=0;
    SALTTime myEnd=0;
};

}
#endif //SALT_EVENTATTRIBUTE_LINKACCESSCONTROL_H
