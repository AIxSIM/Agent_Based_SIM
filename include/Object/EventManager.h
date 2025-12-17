/*
 * EventManager.h
 *
 *  Created on: 2018. 1. 30.
 *      Author: SDJ
 */

#ifndef EVENTMANAGER_H_
#define EVENTMANAGER_H_

#include <utils/config.h>

#include "include/SimScenarioInfo.h"
#include "include/Object/Event/EventAttribute.h"
#include <iostream>
#include <vector>
#include <queue>


namespace SALT{

class EventAttribute;

class EventManager {
private:
    static EventManager* instance;
    EventManager();

public:
    typedef std::pair<EventAttribute*, SALTTime> Event;

    static EventManager* getInstance(); // access to EventManager (singleton)

	virtual ~EventManager();

    Result build(SimScenarioInfo* scenario);

    // core function
    Result build(string const& _event_file, string const& _input_type="SALT");

    virtual void registerEvent(EventAttribute* action, SALTTime scheduledTime);

//    virtual void reRegisterEvent(EventAttribute* action, SALTTime scheduledTime);

    virtual Result handle(SALTTime currentTime);

    bool isEmpty();

    void clearState(SALTTime currentTime, SALTTime newTime);

    SALT::SALTTime getScheduledTime();

protected:
    static bool eventCompare(const Event& e1, const Event& e2);

private:
	std::vector<Event> myEvents;
    SALTTime currentTime;
};
}
#endif /* EVENTMANAGER_H_ */
