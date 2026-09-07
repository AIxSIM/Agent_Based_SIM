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
#include "include/Object/Event/EventInterface.h"
#include <iostream>
#include <vector>
#include <queue>


namespace SALT{

class EventInterface;

class EventManager {
private:
    static EventManager* instance;
    EventManager();

public:
    // typedef std::pair<EventAttribute_LinkAccessControl*, SALTTime> Event;
    typedef std::pair<EventInterface*, SALTTime> Event;

    static EventManager* getInstance(); // access to EventManager (singleton)

	virtual ~EventManager();

    Result build(SimScenarioInfo* scenario);

    // virtual void registerEvent(EventAttribute_LinkAccessControl* action, SALTTime scheduledTime);
    Result registerEvent(EventInterface* action, SALTTime scheduledTime, EventHandleMode _mode);
    Result registerEventDuration(std::string _eventID, SALTTime _startTime, SALTTime _endTime);

    Result changeEventHandleMode(std::string _eventID, SALTTime scheduledTime, EventHandleMode _mode);

//    virtual void reRegisterEvent(EventAttribute* action, SALTTime scheduledTime);

    // Result checkScheduledTime(SALTTime scheduledTime);

    Result handle(SALTTime currentTime);

    bool isEmpty();

    // std::vector<Event> getEvents(SALTTime currentTime);

    // After currentTime, return the scheduled events
    // std::map<SALTTime,std::vector<EventInterface *>> getEvents(SALTTime currentTime);
    std::map<SALTTime, std::deque<std::pair<EventHandleMode, EventInterface *>>> getEvents(SALTTime currentTime);
    EventInterface * getEvent(std::string _id);

    // void clearState(SALTTime currentTime, SALTTime newTime);

    // SALTTime getScheduledTime();
    
    bool isEndEvent(EventInterface * _event);

    bool isEventIntersectionWith(EventInterface * _event);

protected:
    static bool eventCompare(const Event& e1, const Event& e2);

private:
	// std::vector<Event> myEvents;
    // std::map<SALTTime, std::vector<EventInterface *> > myEvents;
    std::map<SALTTime, std::deque<std::pair<EventHandleMode, EventInterface *>>> myEvents;
    std::map<std::string, std::pair<SALTTime, SALTTime>> myEventDurations;
    std::map<std::string, std::vector<std::tuple<std::string, SALTTime, SALTTime>>> myEventIntersections;
    SALTTime currentTime;
    SALTTime simBeginTime;
    SALTTime simEndTime;

    std::map<std::string, std::vector<std::string>> _findEventIntersection();
    Result _registerEventIntersection();

    // core function
    Result _build(string const& _event_file, string const& _input_type="SALT");
    Result _postRegisterProcess();
};
}
#endif /* EVENTMANAGER_H_ */
