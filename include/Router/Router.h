//
// Created by Hyewon Song on 2023/11/23.
//

#ifndef SALT_ROUTER_H
#define SALT_ROUTER_H

#include <limits.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <boost/filesystem.hpp>
#include <iostream>
#include <fstream>
#include <map>
#include <set>
#include <string>
#include <random>
#include <algorithm>
#include <ctime>
#include <iomanip>

#include <utils/config.h>
#include <utils/etc.h>
#include "tinyxml.h"
#include "json/json.h"
#include "CSVReader.h"
#include "CSVWriter.h"

typedef std::chrono::high_resolution_clock Clock;

using namespace std;

#define DEAD_END "dead_end"

namespace SALT{
    class SimulationController;
    class Node;
    class Link;

    struct config_info
            {
        string scenario_id;
        float time_begin;
        float time_end;

        string input_fileType;
        string input_node;
        string input_link;
        string input_connection;
        string input_trafficLightSystem;
        string input_route;

        string region_target;
        string day_target;
        string hour_target;
        string taz_target;
        string vds_target;
        string zone2taz;
        string vds2stdlink;
        string od_raw;
        string vds_raw;
        string vds_raw_dir;
        string od_raw_target;
        string vds_raw_target;

        string od_matrix;
        string t_od_matrix;
        string trip_data;
        string vds_table;
        string vds_renewal;

        string rt_dist;
        string rt_next;
        string rt_renewal;

        string log_file;

        string route_trips;
        string route_weightType;

        float parameter_minCellLength;
        float parameter_vehLength;
        string output_fileDir;
        float output_period;
        string output_level;
        int output_save;
        string with_via;
            };
    struct trip_info
            {
        int id;
        double departtime;
        string from;
        string to;
        string via;
            };
    struct path_info
            {
        std::string path;
        double distance;
            };

    struct od_raw_info
            {
        string o_zone;
        string d_zone;
        // float walk_byc;
        // float pass_car;
        // float bus;
        // float train;
        // float taxi;
        // float etc;
        // float pass_taxi;
        // float etc_sum;
        float sum;
            };
    struct od_matrix_info
            {
        string o_taz;
        string d_taz;
        float total;
        // float weight;
            };
    struct vds2stdlink_info
            {
        string vds_id; //
        string link_id;
        string edge_id; //
        string taz;             //
        string std_link_id;
            };
    struct simulation_info
            {
        int intervalbegin;
        int intervalend;
        string roadID;
        int VehPassed;
        float AverageSpeed;
        float AverageDensity;
        float WaitingQLength;
        float WaitingTime;
        int SumTravelLength;
        int SumTravelTime;
            };
    struct tazboundary_info
            {
        string taz;
        string gu_name;
        string dir;
            };
    struct othertaz_info
            {
        string taz;
        string dj_dir;
        string dj_ysg_dir;
        string dj_sg_dir;
        string dj_jg_dir;
        string dj_dg_dir;
        string dj_ddg_dir;
        string dj_sample_dir;
        string sj_dir;
        string sj_sample_dir;
        string dj_dunsan_dir;
        string dj_dunsan_grid_dir;
        string dj_doan_dir;
        string dj_cdd1_dir;
        string dj_cdd2_dir;
        string dj_cdd3_dir;
        string dj_vissim_dir;
        string ic_check;
            };
    struct spot_info
            {
        string edgeid;
        string edge_taz;
        string edge_matching;
        string edge_date;
        string edge_fprefix;
        string spot_name;
        string spot_dir;
        string spot_date;
        string spot_fprefix;
        string spot_dir_dist;
        string spot_vw;
            };
    struct vds_raw_info
            {
        string VDS_ID;
        // string EDGE_ID;
        // string TAZ;

        string YMD_ID; //*-->등록시각
        string HH_ID;  //*-->등록시각
        // string MN_ID; //*-->등록시각
        // string DAY_CLS; //*-->요일구분
        // string LINK_ID; //*vds2stdlink_dj (std_link_id)
        // string ROAD_ROUTE_ID;
        //  string ROUTE_DIR;
        //  string WTHR_GRP_CD;

        // string RSE_LINK_TRVL_SPD;
        // string RSE_TRVL_TM;
        // string RSE_COLCT_CAR_CNT;
        // string SECTN_TRVL_SPD_5MN;
        // string SECTN_TRVL_TM_5M;
        // string SECTN_TRVL_SPD_15MN;
        // string SECTN_TRVL_TM_15M;
        // string SECTN_TRVL_SPD_YMDH;
        // string SECTN_TRVL_TM_YMDH;
        // string SECTN_CAR_CNT;
        // string SECTN_CNGTN_GRD;
        // string SECTN_COLCT_TYPE_CLS;
        // string PTRN_TRVL_SPD_5MN;
        // string PTRN_TRVL_SPD_15MN;
        // string PTRN_TRVL_SPD_YMDH;

        // string VDS_TR_VOL_5MN;       //*-->교통량int
        // string VDS_SM_TR_VOL_5MN;    //*-->소형교통량int
        // string VDS_MD_TR_VOL_5MN;    //*-->중형교통량int
        // string VDS_LG_TR_VOL_5MN;    //*-->대형교통량int
        // //string VDS_TRVL_SPD_5MN;   //*-->통행속도float
        // //string VDS_TRVL_TM_5MN;
        // //string VDS_OCCUPY_RATE_5MN;        //*-->점유율float
        // //string VDS_AVG_CAR_LEN_5MN;        //*-->평균차량길이float
        // //string VDS_AVG_CAR_TM_5MN; //*-->평균차량시간float

        // string VDS_TR_VOL_15MN;
        // string VDS_SM_TR_VOL_15MN;
        // string VDS_MD_TR_VOL_15MN;
        // string VDS_LG_TR_VOL_15MN;
        // string VDS_TRVL_SPD_15MN;
        // string VDS_TRVL_TM_15MN;
        // string VDS_OCCUPY_RATE_15MN;
        // string VDS_AVG_CAR_LEN_15MN;
        // string VDS_AVG_CAR_TM_15MN;

        string VDS_TR_VOL_YMDH;
        // string VDS_SM_TR_VOL_YMDH;
        // string VDS_MD_TR_VOL_YMDH;
        // string VDS_LG_TR_VOL_YMDH;
        // string VDS_TRVL_SPD_YMDH;
        // string VDS_TRVL_TM_YMDH;
        // string VDS_OCCUPY_RATE_YMDH;
        // string VDS_AVG_CAR_LEN_YMDH;
        // string VDS_AVG_CAR_TM_YMDH;

        // string SECTN_DELAY_TM;
        // string BASE_SPD_TM;
        // string DELAY_TM;
        // string FREEFLOW_SPD;
        // string FREEFLOW_TRVL_TM;
        // string TRVL_TM_95;
        // string TTI;
        // string PTI;
        // string BI;

        // string ETL_TYPE;//*-->
        // string ETL_DATE;//*-->
            };
    struct vds_weight_info
            {
        int time_int;
        string time_str;
        int count;
        double weight_sum;
        double weight_avg;
            };
    struct vds_volume_info
            {
        int time_int;
        string time_str;
        int count;
        double volume_sum;
        double volume_avg;
            };

    class Router {

    public:
        Router();
        virtual ~Router();
        static int COMP_BY_TRIP_INFO_departtime(const trip_info &x, const trip_info &y);
        static bool COMP_BY_OD_RAW_TARGET(const od_raw_info &x, const od_raw_info &y);
        static bool COMP_BY_VDS_RAW_TARGET(const vds_raw_info &x, const vds_raw_info &y);
        vector<string> split(string input, char dlim);
        std::string &ltrim(std::string &s);
        std::string &rtrim(std::string &s);
        std::string &trim(std::string &s);
        bool startsWith(const std::string str, const std::string prefix);
        bool endsWith(std::string const &fullString, std::string const &ending);
        bool fexists(std::string filename);
        vector<string> flist(string path);
        string time_now();
        bool contain_edge_id_in_via(string via, string edge_id);
        bool contain_edge_ids_in_via(string via, set<string> edge_ids);
        bool getConfig(string path);
        void log_file_write(string code, string desc, bool init);
        void get_taz_target(string path, bool pr = false);
        void get_vds_target(string path, bool pr = false);
        void get_vds2stdlink(string path, bool pr = false);
        void get_vds2stdlink_spot(bool pr = false);
        void get_vds2stdlink_sj(string path, bool pr = false);
        void get_zone2taz(string path, bool pr = false);
        bool chk_taz_target(string taz);
        void get_od_raw(string path_r, string path_w);
        void get_od_raw_target(string path_r);
        void make_od_matrix(string path_w);
        void get_od_matrix(string path);
        void get_vds_raw_dj(string path_r, string path_w);
        void get_vds_raw_sj(string vdsr_dir, string path_w);
        void get_vds_raw_target(string path_r);
        void make_vds_table(string path_w);
        void make_vds_weight_day_avg(string day);
        void make_vds_volume_day(string day);
        string valid_vds_volume_day();
        map<int, vds_weight_info> make_cal_vds_weight_day_avg(map<string, vector<double>> _vds_weight_day);
        bool make_taz_edge(SimulationController* sc);
        bool make_vds_edge();
        std::vector<std::vector<float>> buildCostGraph(map<int, SALT::Link *> links);
        std::vector<std::vector<int>> buildAdjGraph(map<SALT::Link *, int> links, int num_vertex);
        void writepaths(std::vector<std::pair<std::vector<double>, string>> paths);
        void make_routing_table(std::vector<Link *> links);
        void make_routing_table_XXX();
        void get_sim_result(string path);
        vector<int> distribution_random_int(const int nrolls, int min, int max, bool print_out); // range : [min, max];
        vector<int> distribution_uniform_int(const int nrolls, const double min, const double max, bool print_out);
        vector<double> distribution_uniform_real(const int nrolls, const int nstars, const int nintervals, const double min, const double max, bool print_out);
        vector<double> distribution_normal(const int nrolls, const int nstars, const double mean, const double std, bool print_out);
        vector<int> distribution_poisson(const int nrolls, const int nstars, const double mean, bool print_out);
        vector<double> distribution_exponential(const int nrolls, const double lamda, bool print_out);
        void print_csv(std::vector<std::vector<double>> dist, std::vector<std::vector<float>> next, map<int, SALT::Link *> mapG);
        void print_routing_table_mem(std::vector<std::vector<double>> dist, std::vector<std::vector<float>> next, map<int, SALT::Link *> mapG);
        string get_routing_path(string _from, string _to);

        map<int, SALT::Link *> get_map_graph_to_link() {
            return this->mapGraphToLink;
        }

        int run_dijkstra(SALT::SimulationController* SC);

        void write_routing_table_file();
        int read_routing_table_file();
        void print_route_csv_csv();
        void print_route_xml_csv();
        void print_route_csv_xml();
        void print_route_xml_xml();
        void printRouting();
        void make_vds_od_taz_probability();
        vector<string> valid_vds_od_taz_probability();
        void make_time_based_od_matrix(string t_od_matrix_path);
        map<string, vector<int>> make_lack_vds_volume_day();
        void generate_via_mem();
        void write_file_route_mem_xml();
        void write_trips_info();

        void set_mapGraph(SALT::Link * _link, int pos) {
            mapGraph[_link] = pos;
        }
        void set_mapGraphToLink(int pos, SALT::Link * _link) {
            mapGraphToLink[pos] = _link;
        }
        void set_mapGraphID2IDX(string id, int pos){
            mapGraphID2IDX[id] = pos;
        }

        //--------------> GLOBAL VRIABLES
        string Global_Path;

        ofstream log_file;

        SimulationController *SC;
        config_info routing_config;
        vector<trip_info> trips_info;
        map<string, map<std::string, path_info *>> from_path_info;
        string day_target;
        vector<string> hour_target;
        vector<string> taz_target;
        vector<string> vds_target;
        map<string, string> zone2taz;                                      // key zone
        map<string, string> taz2zone;                                      // key taz
        map<string, vds2stdlink_info> vds2stdlink_vds_id;  // key vds_id
        map<string, vds2stdlink_info> vds2stdlink_edge_id; // key edge_id
        map<string, vds2stdlink_info> vds2stdlink_link_id; // key vds_link_id

        map<string, vds2stdlink_info> vds2stdlink_spot_vds_id;  // key vds_id
        map<string, vds2stdlink_info> vds2stdlink_spot_edge_id; // key edge_id
        map<string, vds2stdlink_info> vds2stdlink_spot_link_id; // key vds_link_id

        vector<od_raw_info> od_raw;
        vector<vds_raw_info> vds_raw;
        vector<od_raw_info> od_raw_target;
        vector<vds_raw_info> vds_raw_target;

        vector<od_matrix_info> od_matrix;
        map<string, map<string, map<int, int>>> vds_table; // key VDS_ID --> YMD_ID --> HH_ID --> VDS_TR_VOL_YMDH

        map<string, vector<string>> taz_edge; // key taz
        map<string, vector<string>> taz_edge_o;
        map<string, vector<string>> taz_edge_d;
        map<string, vds2stdlink_info> vds_edge_vds_id;  // key vds_id
        map<string, vds2stdlink_info> vds_edge_edge_id; // key edge_id
        // map<pair<string, string>, vds2stdlink_info*> vds_edge; //key vds_id, edge_id

        map<int, vds_weight_info> vds_weights_day_avg; // key time(string) 01:00:00 --> time(int) 1 ---> ***** OD(O) *****
        map<string, vector<int>> vds_volume_day;           // key vds_id ---> ***** CAL(O) *****

        map<int, vector<od_matrix_info>> time_based_od_matrix; // key time 0~24

        // ROUTING TABLE
        int num_vertices;
        map<SALT::Link *, int> mapGraph;
        map<int, SALT::Link *> mapGraphToLink;
        map<string, int> mapGraphID2IDX;
        std::vector<std::vector<double>> RT_DIST;
        std::vector<std::vector<float>> RT_NEXT;

        map<pair<string, string>, map<string, float>> od_taz_vds_probability; //; //od_taz, vds, count
        map<string, map<pair<string, string>, float>> vds_od_taz_probability; //; //vds, od_taz, count

        vector<simulation_info> simulation_result;

        vector<map<string, vector<double>>> vds_sim_volume_rate_vector;
        vector<map<string, vector<double>>> sim_vds_volume_rate_vector;
        vector<map<string, vector<int>>> sim_result_VehPassed_vector;
        vector<map<string, vector<int>>> vds_volume_day_vector;
        vector<map<string, vector<int>>> vds_sim_volume_count_vector;
        vector<map<string, vector<double>>> sim_sum_mape_vector;


    };
}

#endif //SALT_ROUTING_H
