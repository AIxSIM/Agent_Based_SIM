//
// Created by hwonsong on 2018. 5. 18..
//

#ifndef SALT_SIMSCENARIO_H
#define SALT_SIMSCENARIO_H

#include <utils/config.h>
#include <string>
#include <tuple>
#include <list>
#include <vector>
//#include <comm/CmdMessage.hpp>
#include <utility>

namespace SALT{

class SimScenarioInfo {
public:
    SimScenarioInfo();

    explicit SimScenarioInfo(string scenariofile); 
    explicit SimScenarioInfo(string scenariofile, string subSimID); // sub simulation in distributed mode
    explicit SimScenarioInfo(string scenariofile, int eventOption); // with event
    virtual ~SimScenarioInfo();

    void setOutDirPrefix(string outdirPrefix);

    //Simulation ID
    string simid;

    //VisServer's port
    string simhost = "127.0.0.1";
    int simport = 0;
    int siminterval = 1;

    //Simulation Begin time & End time
    SALTTime simbegin;
    SALTTime simend;

    //PartitionInfo
    string partitionid;
    tuple<float,float,float,float> parea;

    //InputParameters - paths to read file
    string scenarioFile;
    string fileType;
    string dataDir;

    string nodeFile;
    string linkFile;
    string connectionFile;
    string tlsFile;
    vector<string> routeFileList;   // list --> vector (change)

    string eventFile;
    string weatherFile;

    //ModelParameters
    Length minCellLength = 30.0;
    Length vehLength = 7.0;

    //OutputParameters
    string outputDir;
    string outputPrefix;
    string outputlevel = "cell";
    int outputperiod = 0;
    int outputsave = 0;

    // debug
    string purpose = "normal"; // normal / debug / test
};

}
#endif //SALT_SIMSCENARIO_H





// example1 standalone scenario json file format
// {
//   "scenario": {
//     "id": "test-salt",
//     "time": {
//       "begin": "0",
//       "end": "3599"
//     },
//     "input": {
//       "fileType" : "SALT",
//       "node": "Gangdong-181130/standalone/monday/before/node.xml",
//       "link": "Gangdong-181130/standalone/monday/before/edge.xml",
//       "connection": "Gangdong-181130/standalone/monday/before/connection.xml",
//       "route": "routes/gd_trips_odpair_weighted_selection_mon.rou.xml",
//       "trafficLightSystem": "Gangdong-181130/standalone/monday/before/tss.xml"
      
//     },
//     "parameter":{
//       "minCellLength": "30.0",
//       "vehLength": "3.0"
//     },
//     "output": {
//       "fileDir": "output/test-salt/",
//       "save": "1",
//       "period": "600",
//       "level": "cell"
    
//     }
//   }
// }  






