#include <iostream>
#include <algorithm>
#include <chrono>
#include <cstring>
#include <iomanip>
#include <set>
#include <thread>
#include <unordered_map>
#include <boost/bind.hpp>
#include <boost/smart_ptr.hpp>
#include <boost/asio.hpp>
#include <boost/thread/scoped_thread.hpp>
#include <boost/tokenizer.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <utils/etc.h>
#include "VisClient.hpp"
#include "Link.h"
#include "Cell.h"
#include "Vehicle.h"
#include "Simulation.h"
#include "VisMsg.h"

using boost::asio::ip::tcp;

namespace libsalt {

    /**
     * Constructor.
     */
    VisClient::VisClient(SALT::SimulationController* SC_, std::string& remoteHost_, int remotePort_) : SC(SC_), remoteHost(remoteHost_), remotePort(remotePort_), socket(io), strand(io) {
        std::cout << std::fixed << std::setprecision(6)
                  << "[DefaultView] roadType=cell, extend="
                  << minx << "," << miny << " " << maxx << "," << maxy
                  << std::endl;

        statusTimer = new boost::asio::deadline_timer(io, boost::posix_time::seconds(statusSleepTime));
        statusTimer->async_wait(strand.wrap(boost::bind(&VisClient::sendStatusMessage, this, boost::asio::placeholders::error)));

        // Disabled for step-based data sending
        // dataTimer = new boost::asio::deadline_timer(io, boost::posix_time::seconds(dataSleepTime));
        // dataTimer->async_wait(strand.wrap(boost::bind(&VisClient::sendDataMessage, this, boost::asio::placeholders::error)));
    }

    /**
     * Handle connection operation.
     */
    void VisClient::handleConnection(const boost::system::error_code &e) {
        if (e) {
            // an error occurred
            std::cerr << __FUNCTION__ << "() error code : " << e.message() << std::endl;
            return;
        }

        // Successfully established connection
        std::cout << __FUNCTION__ << "() Connected to VisServer: " << remoteHost << ":" << std::to_string(remotePort) << std::endl;

        // Send init message
        sendInitMessage();

        // Read 1st message
        read();
    }

    /**
     * Asynchronously read a data structure from the socket
     */
    void VisClient::read() {
        // std::cout << __FUNCTION__ << "() Reading message from VisServer" << std::endl;

        // Read message from VisServer and store them in inBuf
        // Message is handled by handleReadComplete()
        memset(&data, '\0', sizeof(data));
        boost::asio::async_read(socket, boost::asio::buffer(data),
                                boost::asio::transfer_exactly(sizeof(MsgHeader)),
                                boost::bind(&VisClient::handleRead,
                                this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
    }

    /**
     * Handle completion of read message
     */
    void VisClient::handleRead(const boost::system::error_code& e, const size_t bytes_transferred) {
        if (e) {
            if (e.message() == "Operation canceled") {
                // ignore
                return;
            } else if (e == boost::asio::error::eof) {
                std::cout << __FUNCTION__ << "() Client Disconnected" << std::endl;
                stop();
                return;
            }

            // an error occurred
            std::cerr << __FUNCTION__ << "() error code : " << e.message() << std::endl;
            return;
        }

        if (bytes_transferred > 0) {
            std::cout << __FUNCTION__ << "() Received MsgHeader, received=" << bytes_transferred << std::endl;

            // Decode message type
            MsgHeader *header = reinterpret_cast<MsgHeader *>(&data);
            long timestamp = header->timestamp;
            unsigned int bodySize = header->bodySize;

            if (MSG_SET == header->type) {
                std::vector<char> buf;
                buf.resize(header->bodySize);
                boost::system::error_code statusErr;
                size_t received_bytes = boost::asio::read(socket, boost::asio::buffer(buf),
                                                          boost::asio::transfer_exactly(header->bodySize), statusErr);
                std::cout << __FUNCTION__ << "() Received SET message, type=" << header->type << ", bodySize="
                          << header->bodySize << ", received_bytes=" << received_bytes << std::endl;

                // Decode message
                typedef float floatArray[];
                const floatArray *extendPtr = reinterpret_cast<floatArray *>(&buf[0]);
                auto extend = *extendPtr;
                const VisRoadType *roadTypePtr = reinterpret_cast<VisRoadType *>(&buf[sizeof(float) * EXTEND_LENGTH]);
                {
                    std::lock_guard<std::mutex> lock(viewStateMutex);
                    roadType = *roadTypePtr;

                    // Decode geometries of zoom area
                    minx = extend[0];
                    miny = extend[1];
                    maxx = extend[2];
                    maxy = extend[3];
                    viewRevision.fetch_add(1);
                }

                std::string roadTypeStr = "cell";
                if (*roadTypePtr == LINK) {
                    roadTypeStr = "link";
                }

                std::string extendStr =
                        to_string(minx) + "," + to_string(miny) + " " + to_string(maxx) + "," + to_string(maxy);
                cout << std::fixed << std::setprecision(0)
                     << "[SetMsg] timestamp=" << timestamp << ", bodySize=" << bodySize
                     << ", roadType=" << roadTypeStr << ", extend=" << extendStr << std::endl;

            } else if (MSG_STOP == header->type) {
                // std::cout << __FUNCTION__ << "() Received STOP message, type=" << header->type<< std::endl;
                cout << "[StopMsg] timestamp=" << timestamp << ", bodySize=" << bodySize << std::endl;

                // Stop simulation
                //boost::thread th1 = boost::thread(boost::bind(&VisClient::stopSimulation, &io));
                //th1.join();

                // std::thread t( [this]() {io.run(); } );
                // boost::thread t1(&VisClient::stopSimulation, &io);
                // std::thread t1(&VisClient::stop);
                // t1.join();
//                if (io_thd) {
//                    io.stop();
//                    io_thd->join();
//                }

                stop();
                // status = VisClientState::FINISHED;
            } else {
                // Error
                std::cout << __FUNCTION__ << "() Received UNKNOWN message, type=" << header->type << std::endl;
            }
        }

/*
        if (bytes_transferred > 0) {
            std::cout << __FUNCTION__ << "() Read message from VisServer, bytes_transferred=" << bytes_transferred << std::endl;

            // Decode message type
            unsigned int *ptrType = reinterpret_cast<unsigned int *>(&inBuf);
            unsigned int type = *ptrType;
            // std::cout << "Received message's type: " << type << std::endl;

            if (MSG_SET == type) {
                std::cout << __FUNCTION__ << "() Received SET message, type=" << type << std::endl;

                // Decode message
                const SetMsg *msg = reinterpret_cast<const SetMsg *>(inBuf);

                long timestamp = msg->header.timestamp;
                unsigned int bodySize = msg->header.bodySize;

                // Decode geometries of zoom area
                minx = msg->extend[0];
                miny = msg->extend[1];
                maxx = msg->extend[2];
                maxy = msg->extend[3];

                roadType = msg->roadType;
                std::string roadTypeStr = "cell";
                if (msg->roadType == LINK) {
                    roadTypeStr = "link";
                }

                std::string extend = to_string(minx) + "," + to_string(miny) + " " + to_string(maxx) + "," + to_string(maxy);
                cout << "[SetMsg] timestamp=" << timestamp << ", roadType=" << roadTypeStr << ", extend=" << extend << std::endl;

            } else if (MSG_STOP == type) {
                std::cout << __FUNCTION__ << "() Received STOP message, type=" << type << std::endl;

                // Decode message
                const StopMsg *msg = reinterpret_cast<const StopMsg *>(inBuf);

                cout << "[StopMsg] timestamp=" << msg->header.timestamp << std::endl;

                // Stop simulation
                stop();
            } else {
                // error
                std::cout << __FUNCTION__ << "() Received UNKNOWN message, type=" << type << std::endl;
            }

            std::cout << __FUNCTION__ << " READ messages from VisServer" << std::endl;
        }
*/

        // Read next message
        read();
    }

    /**
     * Send init message to VisServer.
     */
    void VisClient::sendInitMessage() {
        // std::cout << __FUNCTION__ << "() Starting " << std::endl;

        if (status == VisClientState::RUNNING) {
            InitMsg msg = InitMsg();
            msg.header = MsgHeader();
            msg.header.type = MSG_INIT;

            const auto p1 = std::chrono::system_clock::now();
            long timestamp = std::chrono::duration_cast<std::chrono::seconds>(
                    p1.time_since_epoch()).count();
            msg.header.timestamp = timestamp;

            std::memset(msg.simulationId, 0, sizeof(msg.simulationId));
            //std::strncpy(msg.simulationId, SC->getSimID().c_str(), sizeof(msg.simulationId) - 1);
            std::strncpy(msg.simulationId, SC->getSimID().c_str(), MAX_SIMULATION_ID);
            msg.header.bodySize = sizeof(msg.simulationId);

            // std::cout << "[InitMsg] timestamp=" << msg.header.timestamp << ", simulationId=[" << msg.simulationId << "]" << std::endl;
            std::cout << std::fixed << std::setprecision(0)
                      << "[InitMsg] timestamp=" << msg.header.timestamp
                      << ", simulationId=[" << std::string(msg.simulationId, sizeof(msg.simulationId)) << "]" << std::endl;

            enqueueWrite(&msg, sizeof(MsgHeader) + msg.header.bodySize);
        }
    }

    /**
     * Tokenize shape string to vector of string.
     */
    vector<string>
    VisClient::tokenizeShape(const std::string& data, const std::string& dropped_delim, const std::string& kept_delim) {
        // boost::char_separator<char> sep(dropped_delim.c_str(), kept_delim.c_str(), boost::keep_empty_tokens);
        boost::char_separator<char> sep(dropped_delim.c_str(), kept_delim.c_str(), boost::drop_empty_tokens);
        boost::tokenizer<boost::char_separator<char>> tokens(data, sep);

        vector<std::string> result { begin(tokens), end(tokens) };
        return result;
    }

    /**
     * Rebuild the visible-road cache for the latest SET viewport.
     *
     * This method is intentionally called from the DataMsg/simulation thread,
     * not from the asynchronous SET handler. Geometry is parsed once per SET
     * revision and is not reparsed for every DataMsg.
     */
    void VisClient::rebuildVisibleRoadCache(VisRoadType currentRoadType,
                                            float currentMinx,
                                            float currentMiny,
                                            float currentMaxx,
                                            float currentMaxy,
                                            unsigned long long currentViewRevision) {
        const auto cacheStart = std::chrono::steady_clock::now();

        visibleLinks.clear();
        visibleCells.clear();

        if (currentRoadType == VisRoadType::LINK) {
            const std::vector<SALT::Link*> links = SC->getLinkList();
            visibleLinks.reserve(links.size());

            for (SALT::Link* link : links) {
                std::vector<std::string> shapes;
                boost::split(shapes, link->getShape(), boost::is_any_of(" ,"));
                if (shapes.size() < 4) {
                    continue;
                }

                const float roadMinx = boost::lexical_cast<float>(shapes[0]);
                const float roadMiny = boost::lexical_cast<float>(shapes[1]);
                const float roadMaxx = boost::lexical_cast<float>(shapes[shapes.size() - 2]);
                const float roadMaxy = boost::lexical_cast<float>(shapes[shapes.size() - 1]);

                if ((roadMinx >= currentMinx) && (roadMiny <= currentMiny) &&
                    (roadMaxx <= currentMaxx) && (roadMaxy > currentMaxy)) {
                    visibleLinks.push_back(link);
                }
            }
        } else {
            const std::vector<SALT::CellInterface*> cells = SC->getCellList();
            visibleCells.reserve(cells.size());

            for (SALT::CellInterface* cell : cells) {
                std::vector<std::string> shapes;
                boost::split(shapes, cell->getMyLink()->getShape(), boost::is_any_of(" ,"));
                if (shapes.size() < 4) {
                    continue;
                }

                const float roadMinx = boost::lexical_cast<float>(shapes[0]);
                const float roadMiny = boost::lexical_cast<float>(shapes[1]);
                const float roadMaxx = boost::lexical_cast<float>(shapes[shapes.size() - 2]);
                const float roadMaxy = boost::lexical_cast<float>(shapes[shapes.size() - 1]);

                if ((roadMinx >= currentMinx) && (roadMiny <= currentMiny) &&
                    (roadMaxx <= currentMaxx) && (roadMaxy >= currentMaxy)) {
                    visibleCells.push_back(cell);
                }
            }
        }

        cachedViewRevision = currentViewRevision;

        const std::chrono::duration<double> cacheElapsed =
                std::chrono::steady_clock::now() - cacheStart;
        std::cout << "[ViewCache] revision=" << cachedViewRevision
                  << ", roadType=" << currentRoadType
                  << ", visibleLinks=" << visibleLinks.size()
                  << ", visibleCells=" << visibleCells.size()
                  << ", rebuildTime=" << std::fixed << std::setprecision(6)
                  << cacheElapsed.count() << " sec" << std::endl;
    }

    /**
     * Send periodic data message to VisServer
     */
    void VisClient::sendDataMessage(const boost::system::error_code& e) {
        if (e) {
            if (e.message() == "Operation canceled") {
                // ignore
                return;
            }

            // an error occurred
            std::cerr << __FUNCTION__ << "() error code : " << e.message() << std::endl;
            return;
        }

        try {
            sendDataMessageInternal();

            dataTimer->expires_at(dataTimer->expires_at() + boost::posix_time::seconds(dataSleepTime));
            dataTimer->async_wait(strand.wrap(boost::bind(&VisClient::sendDataMessage, this, boost::asio::placeholders::error)));
        } catch (bad_alloc &bad) {
            cout << "Not enough memory" << endl;
        } catch (exception &exp) {
            cout << "Exception: " << exp.what() << endl;
        }
    }

    void VisClient::sendDataMessageInternal() {
        bool expected = false;
        if (!dataMessageRunning.compare_exchange_strong(expected, true)) {
            std::cerr << "[DataMsg] skipped: previous construction is still running" << std::endl;
            return;
        }

        struct RunningGuard {
            std::atomic<bool>& flag;
            ~RunningGuard() { flag.store(false); }
        } runningGuard { dataMessageRunning };

        const auto perfStart = std::chrono::steady_clock::now();

        if (status == VisClientState::RUNNING) {

            VisRoadType currentRoadType;
            float currentMinx;
            float currentMiny;
            float currentMaxx;
            float currentMaxy;
            unsigned long long currentViewRevision;
            {
                std::lock_guard<std::mutex> lock(viewStateMutex);
                currentRoadType = roadType;
                currentMinx = minx;
                currentMiny = miny;
                currentMaxx = maxx;
                currentMaxy = maxy;
                currentViewRevision = viewRevision.load();
            }

            if (cachedViewRevision != currentViewRevision) {
                rebuildVisibleRoadCache(currentRoadType,
                                        currentMinx,
                                        currentMiny,
                                        currentMaxx,
                                        currentMaxy,
                                        currentViewRevision);
            }

            unsigned int msgSize = 0;

            DataMsg msg = DataMsg();
            msg.header = MsgHeader();
            msg.header.type = MSG_DATA;

            const auto p1 = std::chrono::system_clock::now();
            long timestamp = std::chrono::duration_cast<std::chrono::seconds>(
                    p1.time_since_epoch()).count();
            msg.header.timestamp = timestamp;

            msgSize += sizeof(MsgHeader);

            int numRoads = 0;
            unsigned int truncatedClassRoads = 0;
            unsigned int maxVehiclesOnRoad = 0;
            std::string maxVehiclesRoadId;
            double getVehiclesSeconds = 0.0;
            double averageSpeedSeconds = 0.0;
            unsigned int missingVehicleIndexCount = 0;

            // Build a fresh index for this DataMsg. The simulation step is
            // blocked while this function runs, so these pointers remain
            // valid for the lifetime of the local index. Rebuilding on every
            // DataMsg automatically includes newly departed vehicles and
            // excludes vehicles no longer in the running set.
            const auto vehicleIndexStart = std::chrono::steady_clock::now();
            const std::set<SALT::VehicleInterface*> runningVehicles =
                    Simulation::getVehicleManager()->getRunningSet();
            std::unordered_map<std::string, SALT::VehicleInterface*> runningVehicleIndex;
            runningVehicleIndex.reserve(runningVehicles.size());
            for (SALT::VehicleInterface* vehicle : runningVehicles) {
                if (vehicle != nullptr) {
                    runningVehicleIndex.emplace(vehicle->getMyName(), vehicle);
                }
            }
            const double vehicleIndexSeconds = std::chrono::duration<double>(
                    std::chrono::steady_clock::now() - vehicleIndexStart).count();

            if (currentRoadType == VisRoadType::LINK) {
                for (SALT::Link* link : visibleLinks) {
                    const auto vehiclesStart = std::chrono::steady_clock::now();
                    std::vector<std::string> vehicles = link->getVehicles();
                    getVehiclesSeconds += std::chrono::duration<double>(
                            std::chrono::steady_clock::now() - vehiclesStart).count();
                    int numVehicles = vehicles.size();
                    if (numVehicles == 0) {
                        continue;
                    }
                    if (numRoads >= MAX_ROADS) {
                        std::cerr << "[DataMsg] road list truncated at " << MAX_ROADS << std::endl;
                        break;
                    }
                        Road& road = msg.roads[numRoads];
                        road = Road();

                        std::memset(road.roadId, 0, sizeof(road.roadId));
                        std::strncpy(road.roadId, link->getID().c_str(), sizeof(road.roadId) - 1);
                        msgSize += sizeof(road.roadId);
                        //std::cout << "roadId=" << road.roadId << ", size=" << sizeof(road.roadId) << std::endl;

                        // =========================
                        // Get link's traffic signal
                        // =========================
                        std::vector<SALT::Link*> nextLinks = link->getNextLinks();
                        road.currentSignal = VisSignalType::RED;
                        for (auto nextLink : nextLinks) {
                            // rotationDir: s(straight), l(left), r(right)
                            std::string rotationDir = link->computeConnectionRotation(*nextLink);
                            // cout << "rotationDir=" << rotationDir << endl;

                            // Do not consider right direction
                            if (rotationDir != "r") {
                                SALT::TrafficSignalState state = link->getTrafficSignalState(nextLink);
                                // if one of the signals is GREEN, link's signal is GREEN
                                if (state == SALT::TrafficSignalState::GREEN) {
                                    road.currentSignal = VisSignalType::GREEN;
                                    break;
                                }
                            }
                        }

                        msgSize += sizeof(road.currentSignal);

                        const int encodedNumVehicles = std::min(numVehicles, static_cast<int>(MAX_VEHICLES));
                        // Keep the actual number of vehicles on the road. Only
                        // the per-vehicle class samples stored in vehicles[]
                        // are capped by the fixed wire-format capacity.
                        road.numVehicles = static_cast<unsigned int>(numVehicles);
                        if (numVehicles > static_cast<int>(MAX_VEHICLES)) {
                            ++truncatedClassRoads;
                            if (static_cast<unsigned int>(numVehicles) > maxVehiclesOnRoad) {
                                maxVehiclesOnRoad = static_cast<unsigned int>(numVehicles);
                                maxVehiclesRoadId = link->getID();
                            }
                        }

                        msgSize += sizeof(road.numVehicles);

                        // ===================================================
                        // Calculate size of vehicles
                        // ---------------------------------------------------
                        // 1. variable size
                        //road.vehicles = static_cast<VisVehicleClass *>(malloc(road.numVehicles * sizeof(VisVehicleClass)));
                        //msgSize += road.numVehicles * sizeof(VisVehicleClass);
                        //roadsSize += road.numVehicles * sizeof(VisVehicleClass);
                        // 2. fixed size
                        msgSize += sizeof(road.vehicles);
                        // ===================================================

                        //cout << "numVehicles=" << numVehicles << ", vehicles.size()=" << vehicles.size() << endl;
                        for (int i = 0; i < encodedNumVehicles; i++) {
                            const std::string& vehicleID = vehicles[i];
                            const auto vehicleIt = runningVehicleIndex.find(vehicleID);
                            if (vehicleIt != runningVehicleIndex.end() &&
                                vehicleIt->second != nullptr &&
                                vehicleIt->second->getMyClass() == SALT::BUS) {
                                road.vehicles[i] = VisVehicleClass::LARGE;
                            } else {
                                road.vehicles[i] = VisVehicleClass::SMALL;
                                if (vehicleIt == runningVehicleIndex.end()) {
                                    ++missingVehicleIndexCount;
                                }
                            }
                        }

                        const auto speedStart = std::chrono::steady_clock::now();
                        road.speed = (unsigned int)Link::getAverageVehicleSpeed(road.roadId);
                        averageSpeedSeconds += std::chrono::duration<double>(
                                std::chrono::steady_clock::now() - speedStart).count();
                        msgSize += sizeof(road.speed);
                        // std::cout << "roadId=" << road.roadId << ", currentSignal=" << road.currentSignal << ", numVehicles=" << road.numVehicles << ", speed=" << road.speed << std::endl;

                        ++numRoads;

                        // TEST: only 3 roads
//                            if (numRoads == 3) {
//                                break;
//                            }
                }
            } else {
                // roadType == CELL
                for (SALT::CellInterface* cell : visibleCells) {
                    const auto vehiclesStart = std::chrono::steady_clock::now();
                    std::vector<std::string> vehicles = cell->getVehicles();
                    getVehiclesSeconds += std::chrono::duration<double>(
                            std::chrono::steady_clock::now() - vehiclesStart).count();
                    int numVehicles = vehicles.size();
                    if (numVehicles == 0) {
                        continue;
                    }
                    if (numRoads >= MAX_ROADS) {
                        std::cerr << "[DataMsg] road list truncated at " << MAX_ROADS << std::endl;
                        break;
                    }
                        Road& road = msg.roads[numRoads];
                        road = Road();

                        std::memset(road.roadId, 0, sizeof(road.roadId));
                        std::strncpy(road.roadId, cell->getID().c_str(), sizeof(road.roadId) - 1);
                        msgSize += sizeof(road.roadId);

                        road.currentSignal = VisSignalType::GREEN;

                        // =========================
                        // Get cell's traffic signal
                        // =========================
                        SALT::Link* link = cell->getMyLink();
                        std::vector<SALT::Link*> nextLinks = link->getNextLinks();
                        // Default is RED
                        road.currentSignal = VisSignalType::RED;
                        for (auto nextLink : nextLinks) {
                            SALT::Connection* conn = link->getConnectionTo(nextLink, cell->getLane());
                            if (conn != nullptr) {
                                SALT::TrafficSignalState state = link->getTrafficSignalState(nextLink);
                                // if one of the signals is GREEN, link's signal is GREEN
                                if (state == SALT::TrafficSignalState::GREEN) {
                                    road.currentSignal = VisSignalType::GREEN;
                                    break;
                                }
                            }
                        }

                        msgSize += sizeof(road.currentSignal);

                        const int encodedNumVehicles = std::min(numVehicles, static_cast<int>(MAX_VEHICLES));
                        // Keep the actual number of vehicles on the road. Only
                        // the per-vehicle class samples stored in vehicles[]
                        // are capped by the fixed wire-format capacity.
                        road.numVehicles = static_cast<unsigned int>(numVehicles);
                        if (numVehicles > static_cast<int>(MAX_VEHICLES)) {
                            ++truncatedClassRoads;
                            if (static_cast<unsigned int>(numVehicles) > maxVehiclesOnRoad) {
                                maxVehiclesOnRoad = static_cast<unsigned int>(numVehicles);
                                maxVehiclesRoadId = cell->getID();
                            }
                        }

                        msgSize += sizeof(road.numVehicles);

                        // ===================================================
                        // Calculate size of vehicles
                        // ---------------------------------------------------
                        // 1. variable size
                        // road.vehicles = static_cast<VisVehicleClass *>(malloc(road.numVehicles * sizeof(VisVehicleClass)));
                        // msgSize += road.numVehicles * sizeof(VisVehicleClass);
                        // 2. fixed size
                        msgSize += sizeof(road.vehicles);
                        // ===================================================

                        for (int i = 0; i < encodedNumVehicles; i++) {
                            const std::string& vehicleID = vehicles[i];
                            const auto vehicleIt = runningVehicleIndex.find(vehicleID);
                            if (vehicleIt != runningVehicleIndex.end() &&
                                vehicleIt->second != nullptr &&
                                vehicleIt->second->getMyClass() == SALT::BUS) {
                                road.vehicles[i] = VisVehicleClass::LARGE;
                            } else {
                                road.vehicles[i] = VisVehicleClass::SMALL;
                                if (vehicleIt == runningVehicleIndex.end()) {
                                    ++missingVehicleIndexCount;
                                }
                            }
                        }

                        const auto speedStart = std::chrono::steady_clock::now();
                        double speedSum = 0.0;
                        unsigned int speedVehicleCount = 0;
                        for (const std::string& vehicleID : vehicles) {
                            const auto vehicleIt = runningVehicleIndex.find(vehicleID);
                            if (vehicleIt == runningVehicleIndex.end() || vehicleIt->second == nullptr) {
                                continue;
                            }
                            speedSum += vehicleIt->second->getMySpeed();
                            ++speedVehicleCount;
                        }
                        road.speed = speedVehicleCount > 0
                                ? static_cast<unsigned int>(speedSum / speedVehicleCount)
                                : 0;
                        averageSpeedSeconds += std::chrono::duration<double>(
                                std::chrono::steady_clock::now() - speedStart).count();
                        msgSize += sizeof(road.speed);
                        // std::cout << "roadId=" << road.roadId << ", currentSignal=" << road.currentSignal << ", numVehicles=" << road.numVehicles << ", speed=" << road.speed << std::endl;

                        ++numRoads;
                }
            }

            if (numRoads > 0) {
                msg.numRoads = numRoads;
                // Use the packed wire-layout size directly. This avoids any
                // drift between manual field accounting and the actual packet.
                msgSize = static_cast<unsigned int>(
                        sizeof(MsgHeader) + sizeof(msg.numRoads) +
                        static_cast<std::size_t>(numRoads) * sizeof(Road));

                msg.header.bodySize = msgSize - sizeof(MsgHeader);

                const auto perfEnd = std::chrono::steady_clock::now();
                const std::chrono::duration<double> elapsed = perfEnd - perfStart;

                std::cout << std::fixed << std::setprecision(0)
                          << "[DataMsg] roadType=" << currentRoadType << ", timestamp=" << msg.header.timestamp
                          << ", numRoads=" << msg.numRoads << ", msgSize=" << msgSize << ", headerSize=" << sizeof(MsgHeader) << ", bodySize=" << msg.header.bodySize << std::endl;
                std::cout << std::setprecision(6)
                          << "[DataMsg] constructionTime=" << elapsed.count() << " sec" << std::endl;
                std::cout << "[DataMsgPerf] candidates="
                          << (currentRoadType == VisRoadType::LINK ? visibleLinks.size() : visibleCells.size())
                          << ", runningVehicles=" << runningVehicleIndex.size()
                          << ", vehicleIndexTime=" << vehicleIndexSeconds
                          << " sec, missingClassLookups=" << missingVehicleIndexCount
                          << ", getVehiclesTime=" << getVehiclesSeconds
                          << " sec, averageSpeedTime=" << averageSpeedSeconds
                          << " sec, otherTime="
                          << std::max(0.0, elapsed.count() - vehicleIndexSeconds -
                                             getVehiclesSeconds - averageSpeedSeconds)
                          << " sec" << std::endl;

                if (truncatedClassRoads > 0) {
                    std::cerr << "[DataMsg] vehicle-class truncation summary: roads="
                              << truncatedClassRoads << ", maxVehicles=" << maxVehiclesOnRoad
                              << ", maxRoad=" << maxVehiclesRoadId
                              << ", classLimit=" << MAX_VEHICLES << std::endl;
                }

                enqueueWrite(&msg, msgSize);
            }
        }

    }

    /**
     * Send periodic status message to VisServer.
     */
    void VisClient::sendStatusMessage(const boost::system::error_code& e) {
        if (e) {
            if (e.message() == "Operation canceled") {
                // ignore
                return;
            }

            // an error occurred
            std::cerr << __FUNCTION__ << "() error code : " << e.message() << std::endl;
            return;
        }

        // std::cout << __FUNCTION__ << "() Starting " << std::endl;

        try {
            if (status == VisClientState::RUNNING) {
                if (SC->getProgress() >= 0) {
                    StatusMsg msg = StatusMsg();
                    msg.header = MsgHeader();
                    msg.header.type = MSG_STATUS;

                    const auto p1 = std::chrono::system_clock::now();
                    long timestamp = std::chrono::duration_cast<std::chrono::seconds>(
                            p1.time_since_epoch()).count();
                    msg.header.timestamp = timestamp;

                    msg.status = status;
                    msg.progress = (unsigned int) SC->getProgress();

                    msg.header.bodySize = sizeof(VisClientState) + sizeof(unsigned int);

                    // std::cout << "msg.size=" << sizeof(msg) << ", header.size=" << sizeof(msg.header) << ", header.type.size=" << sizeof(msg.header.type) << ", header.timestamp.size=" << sizeof(msg.header.timestamp) << ", msg.status.size=" << sizeof(msg.status) << ", msg.progress.size=" << sizeof(msg.progress) << std::endl;

                    std::cout << std::fixed << std::setprecision(0)
                              << "[StatusMsg] timestamp=" << msg.header.timestamp << ", status=" << msg.status
                              << ", progress=" << msg.progress << std::endl;

                    enqueueWrite(&msg, sizeof(MsgHeader) + msg.header.bodySize);
                }
            }

            statusTimer->expires_at(statusTimer->expires_at() + boost::posix_time::seconds(statusSleepTime));
            statusTimer->async_wait(strand.wrap(boost::bind(&VisClient::sendStatusMessage, this, boost::asio::placeholders::error)));
        } catch (bad_alloc &bad) {
            cout << "Not enough memory" << endl;
        } catch (exception &exp) {
            cout << "Exception: " << exp.what() << endl;
        }
    }

//    // Stop simulation
//    void VisClient::stopSimulation() {
//        if (status == VisClientState::FINISHED) {
//            cout << this_thread::get_id() << ": Stopping Simulation" << endl;
//            stop();
//        }
//
//        try {
//            stopTimer->expires_at(stopTimer->expires_at() + boost::posix_time::seconds(1));
//            stopTimer->async_wait(strand.wrap(boost::bind(&VisClient::stopSimulation, this)));
//        } catch (bad_alloc &bad) {
//            cout << "Not enough memory" << endl;
//        } catch (exception &exp) {
//            cout << "Exception: " << exp.what() << endl;
//        }
//    }

    /**
     * Send last status message to VisServer.
     */
    void VisClient::sendLastStatusMessage() {
        StatusMsg msg = StatusMsg();
        msg.header = MsgHeader();
        msg.header.type = MSG_STATUS;

        const auto p1 = std::chrono::system_clock::now();
        long timestamp = std::chrono::duration_cast<std::chrono::seconds>(
                p1.time_since_epoch()).count();
        msg.header.timestamp = timestamp;

        msg.status = status;
        msg.progress = (unsigned int) SC->getProgress();

        msg.header.bodySize = sizeof(VisClientState) + sizeof(unsigned int);

        // std::cout << "msg.size=" << sizeof(msg) << ", header.size=" << sizeof(msg.header) << ", header.type.size=" << sizeof(msg.header.type) << ", header.timestamp.size=" << sizeof(msg.header.timestamp) << ", msg.status.size=" << sizeof(msg.status) << ", msg.progress.size=" << sizeof(msg.progress) << std::endl;

        std::cout << std::fixed << std::setprecision(0)
                  << "[StatusMsg] timestamp=" << msg.header.timestamp << ", status=" << msg.status
                  << ", progress=" << msg.progress << std::endl;

        enqueueWrite(&msg, sizeof(MsgHeader) + msg.header.bodySize);
    }

    /**
     * Copy a packet immediately, then enqueue its write on the socket strand.
     * The shared packet remains alive until async_write completes.
     */
    void VisClient::enqueueWrite(const void* source, std::size_t size) {
        PacketPtr packet = std::make_shared<std::vector<char>>(size);
        std::memcpy(packet->data(), source, size);
        strand.post(boost::bind(&VisClient::enqueueWriteOnStrand, this, packet));
    }

    void VisClient::enqueueWriteOnStrand(PacketPtr packet) {
        const bool writeInProgress = !writeQueue.empty();
        writeQueue.push_back(packet);
        if (!writeInProgress) {
            startNextWrite();
        }
    }

    void VisClient::startNextWrite() {
        if (writeQueue.empty()) {
            return;
        }

        PacketPtr packet = writeQueue.front();
        boost::asio::async_write(
                socket,
                boost::asio::buffer(*packet),
                strand.wrap(boost::bind(&VisClient::handleQueuedWrite,
                                        this,
                                        boost::asio::placeholders::error,
                                        boost::asio::placeholders::bytes_transferred,
                                        packet)));
    }

    void VisClient::handleQueuedWrite(const boost::system::error_code& e,
                                      const size_t bytes_transferred,
                                      PacketPtr packet) {
        if (!writeQueue.empty() && writeQueue.front() == packet) {
            writeQueue.pop_front();
        }

        if (e) {
            std::cerr << __FUNCTION__ << "() error code: " << e.message() << std::endl;
            writeQueue.clear();
            return;
        }

        (void)bytes_transferred;
        startNextWrite();
    }

    /**
     * Handle write message.
     */
    void VisClient::handleWrite(const boost::system::error_code& e, const size_t bytes_transferred, std::vector<boost::asio::const_buffer> buffers) {
        //std::cout << __FUNCTION__ << "() SENT message to VisServer: " << bytes_transferred << " bytes" << std::endl;
        buffers.clear();
    }

} // end of the namespace
