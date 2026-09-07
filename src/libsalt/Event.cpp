//
// Created by Hyewon Song on 26. 5. 22..
//

#include "Event.h"
#include <libsalt/LibsaltDefs.h>


// std::map<int,std::vector<libsalt::LibsaltEvent>> libsalt::Event::getPlannedEvent(const int _curstep) {
//     std::map<int,std::vector<LibsaltEvent>> plannedEvents;
//     SALT::SALTTime curstep = (SALT::SALTTime)_curstep;
//     for (auto ev : getEventManager()->getEvents(curstep)) {
//         std::vector<LibsaltEvent> events;
//         for (auto evm : ev.second) {
//             events.push_back(LibsaltEvent(evm));
//         }
//         plannedEvents[ev.first] = events;
//     }
//     return plannedEvents;
// }

std::map<int, std::deque<std::pair<int, libsalt::LibsaltEventV2>>> libsalt::Event::getPlannedEvents(const int _curstep) {
    std::map<int, std::deque<std::pair<int, LibsaltEventV2>>> plannedEvents;
    SALT::SALTTime curstep = (SALT::SALTTime)_curstep;
    if (getEventManager()->getEvents(curstep).empty()) {
        cout << "No planned event after current time (=" << curstep << ")" << endl;
        return plannedEvents;
    }
    for (auto ev : getEventManager()->getEvents(curstep)) {
        for (auto evm : ev.second) {
            plannedEvents[ev.first].emplace_back(_match_salt_ehm_to_ehm(evm.first), LibsaltEventV2(evm.second));
        }
    }
    return plannedEvents;
}

int libsalt::Event::getScheduledTimeAfter(const int _curstep) {
    std::vector<int> stimes;
    SALT::SALTTime curstep = (SALT::SALTTime)_curstep;
    if (getEventManager()->getEvents(curstep).empty()) {
        cout << "No planned event after current time (=" << curstep << ")" << endl;
        return -1;
    }
    for (auto ev : getEventManager()->getEvents(curstep)) {
        stimes.push_back(ev.first);
    }
    return *std::min_element(stimes.begin(), stimes.end());
}

std::deque<std::pair<int, libsalt::LibsaltEventV2>> libsalt::Event::
getScheduledEventAfter(const int _curstep) {
    std::deque<std::pair<int, LibsaltEventV2>> scheduledEvent;
    SALT::SALTTime curstep = (SALT::SALTTime)_curstep;
    if (getEventManager()->getEvents(curstep).empty()) {
        cout << "No planned event after current time (=" << curstep << ")" << endl;
        return scheduledEvent;
    }
    auto stime = getScheduledTimeAfter(_curstep);
    for (auto ev : getEventManager()->getEvents(curstep)) {
        if (ev.first == stime) {
            for (auto evm : ev.second) {
                scheduledEvent.emplace_back(_match_salt_ehm_to_ehm(evm.first), LibsaltEventV2(evm.second));
            }
        }
    }
    return scheduledEvent;
}

SALT::EventManager * libsalt::Event::getEventManager() {
    return Simulation::getEventManager();
}

SALT::NetworkManager * libsalt::Event::getNetworkManager() {
    return Simulation::getNetworkManager();
}

int libsalt::Event::_match_salt_ehm_to_ehm(SALT::EventHandleMode ehm) {
    if (ehm == SALT::EventHandleMode::EVENT_START) {
        return 0;
    } else if (ehm == SALT::EventHandleMode::EVENT_END) {
        return 1;
    } else if (ehm == SALT::EventHandleMode::EVENT_INTERSECTION) {
        return 2;
    } else if (ehm == SALT::EventHandleMode::EVENT_INTERSECTION_END) {
        return 3;
    } else {
        cout << "Error: Invalid Route Select --> Thus, default to EVENT_START" << endl;
        return 0;
    }
}
