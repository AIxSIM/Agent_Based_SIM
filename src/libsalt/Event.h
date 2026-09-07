/**
 * @class Event
 * @brief C++ libsalt API implementation
 */


#ifndef SALT_EVENT_H
#define SALT_EVENT_H


#include <vector>
#include <libsalt/LibsaltDefs.h>

#include "Simulation.h"


namespace libsalt {
    class Event {
    public:
        // typedef std::pair<LibsaltEventAttribute*, SALT::SALTTime> LibsaltEvent;
        // std::map<int,std::vector<LibsaltEvent>> getPlannedEvent(const int _curstep);
        std::map<int, std::deque<std::pair<int, LibsaltEventV2>>>  getPlannedEvents (const int _curstep);      // After _curstep, return the planned event list
        int getScheduledTimeAfter(const int _curstep);
        std::deque<std::pair<int, LibsaltEventV2>> getScheduledEventAfter(const int _curstep);


    private:
        static SALT::EventManager* getEventManager();
        static SALT::NetworkManager* getNetworkManager();

        int _match_salt_ehm_to_ehm(SALT::EventHandleMode ehm);

    };

}


#endif //SALT_EVENT_H