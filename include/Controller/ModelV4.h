//
// Created by Hyewon Song on 25. 7. 2.
//

#ifndef SALT_MODELV4_H
#define SALT_MODELV4_H


#include <Controller/ModelInterface.h>
#include <utils/config.h>
#include <list>
#include <tuple>

namespace SALT {

    class VehicleInterface;
    namespace version4 {

        class Model : public SALT::ModelInterface {
        public:
            Model(NetworkManager* _nm, VehicleManager* _vm, TrafficSignalManager* _tsm, OutputWriter* _ow)
                    : ModelInterface(_nm, _vm, _tsm, _ow) { };
            Model(NetworkManager* _nm, VehicleManager* _vm, TrafficSignalManager* _tsm, OutputWriter* _ow, EventManager* _ev)
            : ModelInterface(_nm, _vm, _tsm, _ow, _ev) {    setEventMode();   };
            virtual ~Model();

            // 1-step update function: calls phase 1, 2, 3 update functions
            virtual void update(SALTTime _step);

            // update phase 1
            virtual void updateBeforeSync(SALTTime _step);

#ifdef DISTRIBUTED
            // upate phase 2 : do sync
	virtual void doSync(SALTTime _step, std::vector<salt_comm::SyncDataElement>& importingSyncDataElementList);
#endif

            // update phase 3
            virtual void updateAfterSync(SALTTime _step);

            // dump/import
            virtual void doDump(SALTTime import_step);
            virtual void import(SALTTime import_step, string path_to_dump);
            // version specific dump/import
            void fillupVehicleManager(string path_to_dump);
            void deployVehicles(SALTTime import_step); // not implemented yet

            // util
            virtual void print(SALTTime _step, SCOPE _scope);

            // this version specific function
            void planVehicleInsertion(SALTTime _step, list<VehicleInterface*>& listVehToInsert);

            // @distributed
            void sendSyncData(list<tuple<VehicleInterface*, SALTTime> >& syncData); // send a list of SyncData to Master
            void receiveSyncData(list<tuple<VehicleInterface*, SALTTime> >& syncData); // receive a list of SyncData from Master

            const string& getMyMasterIp() const { return myMasterIP;	}
            void setMasterIp(const string& masterIP) { 	myMasterIP = masterIP;	}

            void setDistributedMode(){ 		distributedMode = true; 	}
            bool isDistributedMode(){ 	return distributedMode; 	}

            void setEventMode(){   eventMode = true;   }
            bool isEventMode(){   return eventMode;    }

        private:
            // @distributed
            string myMasterIP="";
            bool distributedMode=false;
            bool eventMode=false;
        };
    }

}


#endif //SALT_MODELV4_H
