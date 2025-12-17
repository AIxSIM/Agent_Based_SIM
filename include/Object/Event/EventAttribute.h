/*
 * EventAttribute.h
 *
 *  Created on: 2018. 1. 26.
 *      Author: SDJ
 */

#ifndef OBJECTT_EVENT_H_
#define OBJECTT_EVENT_H_

#include <utils/config.h>


namespace SALT{
//class EventListener;

class EventAttribute {
public:
	EventAttribute();
	virtual ~EventAttribute();
//	void addListener(EventListener * listener);
//	std::vector<EventListener*> listners_;
    virtual Result enforce(SALTTime currentTime) = 0;
    virtual SALTTime adjustTime(SALTTime currentTime, SALTTime scheduledTime, SALTTime newTime){
        return -1;
    };

    int priority = 0;

private:
//	SALTTime myStartTime;
//	SALTTime myEndTime;
//	EventType myClass;
};
}
#endif /* OBJECTT_EVENT_H_ */
