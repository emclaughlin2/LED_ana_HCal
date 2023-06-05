//#include <caloreco/CaloWaveformProcessing.h>
#include <iostream>
#include <pmonitor/pmonitor.h>
#include "saHCal.h"

#include <odbc++/connection.h>
#include <odbc++/drivermanager.h>
#include <odbc++/resultset.h>
#include <odbc++/statement.h>  // for Statement
#include <odbc++/types.h>


//#include <TH1.h>
//#include <TH2.h>
//#include <TF1.h>
//#include <TMath.h>
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <thread>
#include <chrono>
#include <cstdlib>
#include <set>
#include <regex>
#include <vector>
#include <map>
#include <time.h>

//CaloWaveformProcessing* WaveformProcessing = nullptr;
int init_done = 0;
int fit_init_done = 0;

using namespace std;

int run;
time_t run_time;
char timestampStr[20];
int threshold = 0;
int nevt = 0;

const int numPacket  = 8;
const int numChannel = 192;
const int numDetector = 2;

float mean[numPacket][numChannel][numDetector] = {{{0.0}}};
float x2[numPacket][numChannel][numDetector] = {{{0.0}}};
bool packetExist[numPacket][numDetector] = {{false}};

vector<int> control_board_ids;
vector<int> control_board_indexes;

int led[1536][10];
int pin_diode[1536][10];
string gain[1536][2];
float current[1536][2];
float temp[1536][2];
float vmod[1536][2];
float vp[1536][2];
float vn[1536][2];
float vb[1536][2];
int towers[24] = {0};
string db_time;

bool missing_led = false;
bool missing_pin = false;
bool pedestal = false;

int section[2][3] = {{0}};
int d = 0;

// need to improve signal extraction but this is a fine place holder
std::vector<float> getSignal(Packet *p, const int channel)
{
  double baseline = 0;
  for ( int s = 0;  s < 3; s++) {
    baseline += p->iValue(s, channel);
  }
  baseline /= 3.;

  double signal = 0;
  for ( int s = 3;  s < p->iValue(0, "SAMPLES"); s++) {
    float x = p->iValue(s, channel) - baseline;      
    if(x > signal) signal=x;
  }
  std::vector<float> result;
  result.push_back(signal);
  result.push_back(baseline);
  result.push_back(0);

  return result;
}

std::vector<float> anaWaveform(Packet* p, const int channel)
{
  std::vector<float> waveform;
  for (int s = 0; s < p->iValue(0, "SAMPLES"); s++)
  {
    waveform.push_back(p->iValue(s, channel));
  }
  std::vector<std::vector<float>> multiple_wfs;
  multiple_wfs.push_back(waveform);

  std::vector<std::vector<float>> fitresults_ohcal;
  //fitresults_ohcal = WaveformProcessing->process_waveform(multiple_wfs);

  std::vector<float> result= {5,5,5};
  //result = fitresults_ohcal.at(0);

  return result;
}


//return the index of the vertical tower
int vertidx(int idx) {
  if (idx % 2  == 0) return idx + 1;

  if (idx % 2 == 1) return idx - 1;

  return -999;
}

void hcal_tower_index_mapping(int control_id, int index, int *towers) {

    int sector_start;
    int north_tower_channel_map[24] = {72,73,74,75,76,77,78,79,128,129,130,131,132,133,134,135,136,137,138,139,140,141,142,143};
    int south_tower_channel_map[24] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,64,65,66,67,68,69,70,71};

    map<int, int>control_map = {{80, 8}, {81, 12}, {82, 16}, {83, 20}, {84, 8}, {85, 12}, {86, 16}, {87, 20},
        {90, 24}, {91, 28}, {92, 0}, {93, 4}, {94, 24}, {95, 28}, {96, 0}, {97, 4}};

    sector_start = control_map.at(control_id);
    if (index % 2) {
        for (int i = 0; i < 24; i++) {
            towers[i] = north_tower_channel_map[i] + 48*sector_start + 16*(index/2);
        }
    } else {
        for (int i = 0; i < 24; i++) {
            towers[i] = south_tower_channel_map[i] + 48*sector_start + 16*(index/2);
        }
    }
}

void parse_led() {

    ifstream ledfile("led_info.txt");
    if (!ledfile.is_open()) {
        cerr << "Error opening led_info.txt file" << endl;
        missing_led = true;
        return;
    }

    set<int> ohcal = {80, 81, 82, 83, 90, 91, 92, 93};
    set<int> ihcal = {84, 85, 86, 87, 94, 95, 96, 97};

    string line;
    regex regex_control_board_id(R"(\d{1,3}\.\d{1,3}\.(\d{1,3})\.(\d{1,3}))");
    while(getline(ledfile, line)) {
        smatch matches;
        regex regex_control_board_index(R"(\$LR(\d):)");
        if (regex_search(line, matches, regex_control_board_id)) {
            control_board_ids.push_back(stoi(matches[2].str()));
            section[0][0]++;
        } else if (regex_search(line, matches, regex_control_board_index)) {
            control_board_indexes.push_back(stoi(matches[1].str()));
            section[0][1]++;
        } else if (line.find('|') == string::npos) { continue;
        } else {
            string values = line.substr(matches[0].length());
            regex regex_value(R"(\s*\|\s*([\d\.]+)\s*)");
            sregex_iterator it(values.begin(), values.end(), regex_value);
            if (ohcal.find(control_board_ids.back()) != ohcal.end()) d = 0; else d = 1;
            hcal_tower_index_mapping(control_board_ids.back(),control_board_indexes.back(),towers);
            for (int i = 0; i < 5; i++) {
                for (int j = 0; j < 24; j++) {
                   led[towers[j]][5*d+i] = stoi(it->str(1));
                }              
                ++it;
            }
            section[0][2]++;
        }
    }
}

void parse_pindiode() {

    ifstream pindiodefile("pindiode_info.txt");
    if (!pindiodefile.is_open()) {
        cerr << "Error opening pindiode_info.txt file" << endl;
        missing_pin = true;
        return;
    }

    set<int> ohcal = {80, 81, 82, 83, 90, 91, 92, 93};
    set<int> ihcal = {84, 85, 86, 87, 94, 95, 96, 97};

    string line;
    regex regex_control_board_id(R"(\d{1,3}\.\d{1,3}\.(\d{1,3})\.(\d{1,3}))");
    while (getline(pindiodefile, line)) {
        smatch matches;
        regex regex_control_board_index(R"(\$LA(\d):)");
        if (regex_search(line, matches, regex_control_board_id)) {
            control_board_ids.push_back(stoi(matches[2].str()));
            section[1][0]++;
        } else if (regex_search(line, matches, regex_control_board_index)) {
            control_board_indexes.push_back(stoi(matches[1].str()));
            section[1][1]++;
        } else if (line.find('|') == string::npos) { continue;
        } else {
            string values = line.substr(matches[0].length());
            regex regex_value(R"(\s*([\d\.]+)\s*\|\s*)");
            sregex_iterator it(values.begin(), values.end(), regex_value);
            if (ohcal.find(int(control_board_ids.back())) != ohcal.end()) d = 0; else d = 1; 
            hcal_tower_index_mapping(control_board_ids.back(),control_board_indexes.back(),towers);
            for (int i = 0; i < 5; i++) {
                for (int j = 0; j < 24; j++) {
                    pin_diode[towers[j]][5*d+i] = stoi(it->str(1));
                }
                ++it;
            }
            section[1][2]++;
        }
    }
}

void parse_hcal_db(string filename) {

    ifstream hcalfile(filename);
    if (!hcalfile.is_open()) {
        cerr << "Error opening " << filename << " file" << endl;
        return;
    }

    string line;
    getline(hcalfile, line); // skip header line
    vector<vector<string>> data;

    while (getline(hcalfile, line)) {
        vector<string> tokens;
        istringstream iss(line);
        string token;
        while (getline(iss, token, ',')) {
            tokens.push_back(token);
        }
        data.push_back(tokens);
    }

    db_time = data[0][0];
    // Print the parsed data
    // // time,detector,towerid,gain,vp,vb,vn,vmod,current,temp
    for (size_t i = 0; i < data.size(); ++i) {
        int t = stoi(data[i][2]);
        int d = stoi(data[i][1]);
    
        gain[t][d] = data[i][3];
        vp[t][d] = stof(data[i][4]);
        vb[t][d] = stof(data[i][5]);
        vn[t][d] = stof(data[i][6]);
        vmod[t][d] = stof(data[i][7]);
        current[t][d] = stof(data[i][8]);
        temp[t][d] = stof(data[i][9]);
    
    }
}

void parse_run_type() {
    ifstream runtypefile("run_type_info.txt");
    if (!runtypefile.is_open()) {
        cerr << "Error opening run_type_info.txt file" << endl;
        return;
    }

    string line;
    getline(runtypefile, line);
    if (line.find("led") != string::npos) {
        pedestal = false;
    } else if (line.find("pedestal") != string::npos) {
        pedestal = true;
    } else {
        cout << "Unknown run type, exiting anaylsis" << endl;
        exit(0);
    }
}



int pinit()
{
    
    parse_led();
    parse_pindiode();
    parse_run_type();
    //parse_hcal_db("ihcal_info.txt");
    //parse_hcal_db("ohcal_info.txt");

  //WaveformProcessing = new CaloWaveformProcessing();
  //WaveformProcessing->set_processing_type(CaloWaveformProcessing::FAST);

  //cout << "initiallizing" << endl;
  //char name[500];
  //for (int s = 0; s < numPacket; s++) {
  //  for (int c = 0; c < numChannel; c++) {
  //    sprintf(name, "signal_sec%d_ch%d", s, c);
  //    hsig[s][c] = new TH1F(name, name, 100, -100, 100);
  //    hsig[s][c]->StatOverflows(1);
  //  }
  //}
  return 0;
}

// this runs for every event
int process_event (Event * e)
{

  if ( e->getEvtType() == 9)
  {
    run = e->getRunNumber();
    run_time = e->getTime();
    cout << "Run number " << run << endl;
    std::strftime(timestampStr, sizeof(timestampStr), "%Y-%m-%d %H:%M:%S", std::localtime(&run_time));
    printf("%s\n", timestampStr);    
    return 0;
  }

  int returnval = 0;

  // data is ordered into packets, like looping over sectors
  for (int packet = 8001; packet <= 8008; packet++) {

    Packet *p = e->getPacket(packet);
    if (p)
    {
      int sect = packet - 8001; //packet to sector mapping
      packetExist[sect][0] = true;
      for ( int c = 0; c < p->iValue(0, "CHANNELS"); c++)
      {
        std::vector<float> result = getSignal(p, c);
        //std::vector<float> result = anaWaveform(p, c);
        float signal = result.at(0);
        //cout << "c: " << c << " p: " << packet <<endl;
        //hsig[sect][c]->Fill(signal);
        mean[sect][c][0] += signal;
        x2[sect][c][0] += signal * signal;
      } // channels

      delete p;
      

    } // if p
  } // packet loop
  for (int packet = 7001; packet <= 7008; packet++) {
      Packet *p = e->getPacket(packet);
      if (p) {
          int sect = packet - 7001;
          packetExist[sect][1] = true;
          for (int c = 0; c < p->iValue(0, "CHANNELS"); c++) {
            std::vector<float> result = getSignal(p, c);
            float signal = result.at(0);
            mean[sect][c][1] += signal;
            x2[sect][c][1] += signal * signal;
          }
      }
      delete p;
  }

  nevt++;
  return returnval;
}

int pclose()
{

  double stdev[numPacket][numChannel][numDetector]={{{0.0}}};
  for (int d = 0; d < numDetector; d++) {
    for (int s = 0; s < numPacket; s++) {
        for (int c = 0; c < numChannel; c++) {
            mean[s][c][d] /= nevt;
            x2[s][c][d] /= nevt;
            stdev[s][c][d] = sqrt(x2[s][c][d] - pow(mean[s][c][d],2));
        }  
    }
  }

  odbc::Connection *m_OdbcConnection = nullptr;
  //  Disconnect();
  int icount = 0;
  do {
    try {
      m_OdbcConnection = odbc::DriverManager::getConnection("daq","phnxrc","");
    }
    catch (odbc::SQLException &e) {
      std::cout << " Exception caught during DriverManager::getConnection" << std::endl;
      std::cout << "Message: " << e.getMessage() << std::endl;
    }
    icount++;
    if (!m_OdbcConnection)
    std::this_thread::sleep_for(std::chrono::seconds(30));  // sleep 30 seconds before retry
  } while (!m_OdbcConnection && icount < 5);

  std::string sql = "INSERT INTO ";
  if (pedestal) { sql += "hcal_pedestal "; }
  else { sql += "hcal_led "; }
  sql += "(run, time, detector, towerid, PinDiod_1, PinDiod_2, PinDiod_3, PinDiod_4, PinDiod_5, ";
  sql += "LedPulseWidth_1, LedPulseWidth_2, LedPulseWidth_3, LedPulseWidth_4, LedPulseWidth_5, ";
  if (pedestal) { sql += "pedestal_mean, pedestal_std) VALUES "; }
  else { sql += "led_mean, led_std) VALUES "; }
  for (int d = 0; d < numDetector; d++) {
      for (int s = 0; s < numPacket; s++) {
          if (!packetExist[s][d]) continue;
          for (int c = 0; c < numChannel; c++) {
              int towerid = s * 192 + c;
              sql += "(" + std::to_string(run) + ", '" + timestampStr + "', " + std::to_string(d) + ", " + std::to_string(towerid) + ", ";
              sql += std::to_string(pin_diode[towerid][5*d]) + ", " + std::to_string(pin_diode[towerid][5*d+1]) + ", " + std::to_string(pin_diode[towerid][5*d+2]) + ", ";
              sql += std::to_string(pin_diode[towerid][5*d+3]) + ", " + std::to_string(pin_diode[towerid][5*d+4]) + ", " + std::to_string(led[towerid][5*d]) + ", ";
              sql += std::to_string(led[towerid][5*d+1]) + ", " + std::to_string(led[towerid][5*d+2]) + ", " + std::to_string(led[towerid][5*d+3]) + ", ";
              sql += std::to_string(led[towerid][5*d+4]) + ", " + std::to_string(mean[s][c][d]) + ", " + std::to_string(stdev[s][c][d]) + ")";

              if (d == 1 && s == 7 && c == 191) {
                sql += "";
              } else if (d == 1 && s == 5 && c == 191) {
                sql += "";
              } else {
                sql += ",";
              }
          }
      }
  }

  odbc::Statement* stmt = m_OdbcConnection->createStatement();
  stmt->execute(sql);

  delete stmt;

  delete m_OdbcConnection;

  return 0;
}
