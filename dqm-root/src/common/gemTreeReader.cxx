#define DEBUG 1
#define NVFAT 24
#define NETA 8

#define PORT 3306
#include <mysql/mysql.h>
#include <Python.h>

#include <iomanip> 
#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <cstdlib>
#include <sstream>
#include <vector>
#include <cstdint>
#include <stdexcept>
#include <algorithm>
#include <functional>
#include <array>
#include <TFile.h>
#include <TKey.h>
#include <TDirectory.h>
#include <TNtuple.h>
#include <TH2.h>
#include <TProfile.h>
#include <TCanvas.h>
#include <TFrame.h>
#include <TROOT.h>
#include <TSystem.h>
#include <TRandom3.h>
#include <TBenchmark.h>
#include <TInterpreter.h>
#include <TApplication.h>
#include <TString.h>
#include <Event.h>
#include <TObject.h>
#include <TClonesArray.h>
#include <TRefArray.h>
#include <TRef.h>
#include <TH1.h>
#include <TBits.h>
#include <TMath.h>
#include <TFile.h>
#include <TClassTable.h>
#include <TTree.h>
#include <TBranch.h>
#include <TError.h>
#include <TBufferJSON.h>
#include <memory>
#include <unordered_map>

#include "gem/readout/GEMslotContents.h"
#include "GEMClusterization/GEMStrip.h"
#include "GEMClusterization/GEMStripCollection.h"
#include "GEMClusterization/GEMClusterContainer.h"
#include "GEMClusterization/GEMClusterizer.h"
#include "plotter.cxx"
#include "logger.cxx"
#include "integrity_checker.cxx"
#include "GEMDQMerrors.cxx"
#include "AMC13_histogram.cxx"

using namespace std;

//!A class that creates the subdirectories and histograms from VFAT, GEM, AMC and AMC13 data
class treeReader
{
public:

  //!The constructor requires a file name ending with .raw.root
  treeReader(const std::string &ifilename)
  {
    std::string tmp = ifilename.substr(ifilename.size()-9, ifilename.size());
    if (tmp != ".raw.root") throw std::runtime_error("Wrong input filename (should end with '.raw.root'): "+ifilename);
    ifile = new TFile(ifilename.c_str(), "READ");
    ofilename = ifilename.substr(0,ifilename.size()-9);
    ofilename += ".analyzed.root";
    ofile = new TFile(ofilename.c_str(), "RECREATE");
    if (DEBUG) std::cout << std::dec << "[gemTreeReader]: File for histograms created" << std::endl;   

    if (DEBUG) std::cout << std::dec << "[gemTreeReader]: Fetching hardware" << std::endl;   
    this->fetchHardware();

    if (DEBUG) std::cout << std::dec << "[gemTreeReader]: Connecting to database on port " << PORT << std::endl;
    bool DBConnected = this->connectDB();
    if (DBConnected) {
      std::cout << "[gemTreeReader]: Connected to database, getting run number." << std::endl;
      unsigned int runNum = this->getRunNumber();
      this->getChipIDFromID(220);
      this->getChipIDFromID(1281);

      this->getVFATDBID("AMC-9","GTX-1",17);


      // if (DEBUG) std::cout << std::dec << "[gemTreeReader]: Booking histograms" << std::endl;   
      // this->bookAllHistograms();
      // this->fillAllHistograms();
    }
    else
      std::cout << "Could not connect to DB!!" << std::endl;

  }
  ~treeReader(){}

private:
  TFile *ifile;   ///<Input File, assigned in the constructor
  TFile *ofile;   ///<Output File, assigned in the constructor, where all the histograms will go
  std::string ofilename;  ///<Name of output file, same as input file, but is .analyzed.root instead of .raw.root

  //std::vector<TDirectory*> AMC13dir;  //unused  
  //std::vector<TDirectory*> AMCdir;    //unused
  //std::vector<TDirectory*> GEBdir;    //unused
  //std::vector<TDirectory*> VFATdir;   //unused

  vector<AMC13Event> v_amc13;    ///<Vector of AMC13Event
  vector<AMCdata> v_amc;         ///<Vector of AMCdata
  vector<GEBdata> v_geb;         ///<Vector of GEBdata
  vector<VFATdata> v_vfat;       ///Vector of VFATdata


  //vector<AMC13_histogram> v_amc13H;   //unused
  vector<AMC_histogram> v_amcH;        ///<Vector of AMC_histogram
  vector<GEB_histogram> v_gebH;        ///<Vector of GEB_histogram
  vector<VFAT_histogram> v_vfatH;      ///<Vector of VFAT_histogram

  unordered_map<std::string, int> vfat_map;
  unordered_map<std::string, int> geb_map;

  AMC13_histogram * m_amc13H;
  AMC_histogram * m_amcH;
  GEB_histogram * m_gebH;
  VFAT_histogram * m_vfatH;

  int m_RunType;
  int m_deltaV;
  int m_Latency;

  MYSQL *Database;

  
  bool connectDB()
  {

    // p_gemDBHelper = std::make_shared<gem::utils::db::GEMDatabaseUtils>("gem904daq01.cern.ch",3306,"gemdaq","gemdaq");

    /** NECESSARY?? **/
    
    // Py_Initialize();
    // PyRun_SimpleString("from query import configure_db\n"
    //                    "configure_db()\n");
    // // DEBUG("GEMDatabaseUtils::configure_db");
    // Py_Finalize();

    Database = mysql_init(0);

    if (mysql_real_connect(Database,"gem904daq01.cern.ch","gemdaq","gemdaq","ldqm_db",PORT,0,CLIENT_COMPRESS) == 0) {
      std::string message("Error connecting to database '");
      // message += database;
      message += "' : ";
      message += mysql_error(Database);
      Database = 0;
      std::cout << message << std::endl;
      // ERROR(message);
      //XCEPT_RAISE(gem::exception::ConfigurationDatabaseException,message);
      return false;
    }
    return true;

  }


  unsigned int getRunNumber()
  {
  
    std::string    setup = "teststand";
    std::string   period = "2016T";
    std::string location = "TIF";
    std::string lastRunNumberQuery = "SELECT Number FROM ldqm_db_run WHERE Station LIKE '";
    lastRunNumberQuery += location;
    lastRunNumberQuery += "' ORDER BY id DESC LIMIT 1;";
    const char * query = lastRunNumberQuery.c_str();
    
    try {
      int rv = mysql_query(Database,query);
      if (rv)
        std::cout << "MySQL query error: " << std::string(mysql_error(Database)) << std::endl;
      else
        if(DEBUG) std::cout << "[getRunNumber]: MySQL query success: " << lastRunNumberQuery << std::endl;

      MYSQL_RES* res = mysql_use_result(Database);
      MYSQL_ROW  row = mysql_fetch_row(res);
      if (row == 0) {
        std::string errMsg = "[getRunNumber]: Query result " + lastRunNumberQuery + " empty";
        std::cout <<"[getRunNumber]: " << errMsg << std::endl;
      }
      unsigned int retval = strtoul(row[0],0,0);
      mysql_free_result(res);
      
      if (DEBUG) std::cout << "[getRunNumber]: New run number is: " << retval << std::endl;
      return retval;
    } catch (std::exception& e) {
      std::cout << "[getRunNumber]: caught std::exception " << e.what() << std::endl;
    }
  }

  unsigned int getChipIDFromID(unsigned int db_id)
  {
    std::string m_Query = "SELECT ChipID FROM ldqm_db_vfat WHERE id LIKE ";
    m_Query += std::to_string((long long int)db_id);
    const char * query = m_Query.c_str();
    try {
      int rv = mysql_query(Database,query);
      if (rv)
        std::cout << "MySQL query error: " << std::string(mysql_error(Database)) << std::endl;
      else
        if(DEBUG) std::cout << "[getChipIDFromID]: MySQL query success: " << m_Query << std::endl;
      MYSQL_RES* res = mysql_use_result(Database);
      MYSQL_ROW  row = mysql_fetch_row(res);
      if (row == 0) {
        std::string errMsg = "[getChipIDFromID]: Query result " + m_Query + " empty";
        std::cout << errMsg << std::endl;
        return NULL;
      }
      unsigned int retval = strtoul(row[0],0,0);
      mysql_free_result(res);
      
      if (DEBUG) std::cout << "[getChipIDFromID]: ChipID is: " << std::hex << retval << std::dec << std::endl;
      return retval;
    } catch (std::exception& e) {
      std::cout << "[getChipIDFromID]: caught std::exception " << e.what() << std::endl;
    }
  }

  unsigned int getVFATDBID(std::string AMCboardid, std::string GEBchamberid, int slot) {

    std::string AMCdbidQuery  = "SELECT id FROM ldqm_db_amc WHERE BoardID LIKE '";
    AMCdbidQuery += AMCboardid;
    AMCdbidQuery += "'";
    int AMCdbid = atoi(simpleDBQuery(AMCdbidQuery));
    std::cout << "AMCdbid: " << AMCdbid << std::endl;

    std::string GEBdbidsQuery  = "SELECT geb_id FROM ldqm_db_amc_gebs WHERE amc_id LIKE ";
    GEBdbidsQuery += std::to_string((long long int)AMCdbid);
    vector<char*> GEBdbids = manyDBQuery(GEBdbidsQuery);

    int correctGEBID = 0;
    for (int id = 0; id < GEBdbids.size(); id++) {
      int currentGEBid = atoi(GEBdbids[id]);
      cout << "GEBdbids: " << currentGEBid << endl;
      std::string GEBchambermatchQuery = "SELECT ChamberID FROM ldqm_db_geb WHERE id LIKE ";
      GEBchambermatchQuery += std::to_string((long long int)atoi(GEBdbids[id]));
      char* dbchamberid = simpleDBQuery(GEBchambermatchQuery);
      std::string chamberiddb = dbchamberid;
      if (chamberiddb.compare(GEBchamberid)) {
        std::cout << "Located correct GEB: " << chamberiddb << std::endl;
        correctGEBID = currentGEBid;
        break;
      }
    }

    if (correctGEBID==0) {
      std::cout << "Could not locate correct GEB" << std::endl;
      return 0;
    }

    std::cout << correctGEBID << std::endl;
    
    std::string GEBVFATsQuery = "SELECT vfat_id FROM ldqm_db_geb_vfats WHERE geb_id LIKE ";
    GEBVFATsQuery += std::to_string((long long int)correctGEBID);
    vector<char*> correctGEBVFATs = manyDBQuery(GEBVFATsQuery);

    int correctVFATID = 0;
    for (int vf=0; vf<correctGEBVFATs.size();vf++) {
      int currentVFATID = atoi(correctGEBVFATs[vf]);
      std::cout << currentVFATID << std::endl;

      std::string VFATSlotQuery = "SELECT Slot FROM ldqm_db_vfat WHERE id LIKE ";
      VFATSlotQuery += std::to_string((long long int)currentVFATID);
      int currentSlot = atoi(simpleDBQuery(VFATSlotQuery));
      if (currentSlot == slot) {
        correctVFATID = currentVFATID;
        break;
      }   
    }

    if (correctVFATID != 0) {
      int ChipID = strtol(simpleDBQuery("SELECT ChipID FROM ldqm_db_vfat WHERE id LIKE "+std::to_string((long long int)correctVFATID)),NULL,16);
      std::cout << std::hex << ChipID << std::endl;
      return ChipID;
    }
  }

  vector<char*> manyDBQuery(std::string m_Query)
  {
    vector<char*> result;
    const char * manyQuery = m_Query.c_str();

    int qresult = mysql_query(Database,manyQuery);
    if (qresult) {
      std::cout << "MySQL query error: " << std::string(mysql_error(Database)) << std::endl;
      return result;
    }
    else
      if(DEBUG) std::cout << "MySQL query success: " << manyQuery << std::endl;

    MYSQL_RES* res = mysql_use_result(Database);
    MYSQL_ROW row;

    while (((row=mysql_fetch_row(res)) !=NULL))
      {
        result.push_back(row[0]);
      }

    mysql_free_result(res);

    return result;
  }

  
  //Queries that should return a single value
  char* simpleDBQuery(std::string m_Query)
  {
    const char * query = m_Query.c_str();
    try {
      int rv = mysql_query(Database,query);
      if (rv) {
        std::cout << "MySQL query error: " << std::string(mysql_error(Database)) << std::endl;
        return NULL;
      }
      else
        if(DEBUG) std::cout << "MySQL query success: " << m_Query << std::endl;
      MYSQL_RES* res = mysql_use_result(Database);
      MYSQL_ROW  row = mysql_fetch_row(res);
      if (row == 0) {
        std::cout << "Query result " << m_Query << " empty" << std::endl;
        return NULL;
      }
      char* retval = row[0];
      mysql_free_result(res);
      return retval;
    } catch (std::exception& e) {
      std::cout << "simpleDBQuery caught std::exception " << e.what() << std::endl;
    }
    



  }
  
  //!Fetches data from AMC13, AMC, GEB, and VFAT and puts them into vectors
  void fetchHardware()
  {
    try{
      TTree *tree = (TTree*)ifile->Get("GEMtree");
      Event *event = new Event();
      TBranch *branch = tree->GetBranch("GEMEvents");
      branch->SetAddress(&event);
      Int_t nentries = tree->GetEntries();
      branch->GetEntry(0);
      v_amc13 = event->amc13s();
      for(auto a13 = v_amc13.begin(); a13!= v_amc13.end(); a13++){
        v_amc = a13->amcs();
        for(auto a=v_amc.begin(); a!=v_amc.end(); a++){
          v_geb = a->gebs();
          for(auto g=v_geb.begin(); g!=v_geb.end();g++){
            v_vfat=g->vfats();
          }
        }
      }
      if (DEBUG) std::cout<< "[gemTreeReader]: " << "Number of TTree entries: " << nentries << "\n";
      if (DEBUG) std::cout<< "[gemTreeReader]: " << "Number of AMC13s: " << v_amc13.size()<< "\n";
      if (DEBUG) std::cout<< "[gemTreeReader]: " << "Number of AMCs: " << v_amc.size()<< "\n";
      if (DEBUG) std::cout<< "[gemTreeReader]: " << "Number of GEBs: " << v_geb.size()<< "\n";
      if (DEBUG) std::cout<< "[gemTreeReader]: " << "Number of VFATs: " << v_vfat.size()<< "\n";
    }
    catch(...){
      std::cout<< "[gemTreeReader]: " << "No GEMtree in input raw file!" << std::endl;
      return;
    }
  }

  //!Creates the subdirectories for AMC13, AMC, GEB, and VFAT and books the histograms
  void bookAllHistograms()
  {
    int a13_c=0;    //counter through AMC13s
    int a_c=0;      //counter through AMCs
    int g_c=0;      //counter through GEBs
    int v_c=0;      //counter through VFATs
    
    std::vector<string> vfat_useddirs;
    
    /* LOOP THROUGH AMC13s */
    for(auto a13 = v_amc13.begin(); a13!=v_amc13.end(); a13++){
      v_amc = a13->amcs();

      char diramc13[30];        //filename for AMC13 directory
      diramc13[0]='\0';         
      char serial_ch[20];       //char used to put serial number into directory name
      serial_ch[0] = '\0';
      int serial = v_amc13[a13_c].nAMC();  //obtains the serial number from the AMC13 Event
      sprintf(serial_ch, "%d", serial);
      strcat(diramc13,"AMC13-");
      strcat(diramc13,serial_ch);
      if (DEBUG) std::cout << std::dec << "[gemTreeReader]: AMC13 Directory " << diramc13 << " created" << std::endl;
      //AMC13 HISTOGRAMS HERE
      m_amc13H = new AMC13_histogram(ofilename, gDirectory->mkdir(diramc13), serial_ch);
      m_amc13H->bookHistograms();

      a_c=0;

      /* LOOP THROUGH AMCs */
      for(auto a=v_amc.begin(); a!=v_amc.end(); a++){
        v_geb = a->gebs();
        char diramc[30];        //filename for AMC directory  
        diramc[0]='\0';
        char aslot_ch[2];       //char used to put AMC slot number inot directory name
        aslot_ch[0] = '\0';
        int aslot = v_amc[a_c].AMCnum();  //obtains the slot number from the AMCdata
        sprintf(aslot_ch, "%d", aslot);
        strcat(diramc,"AMC-");
        strcat(diramc, aslot_ch);

        std::string AMCID = diramc;
        
        


        if (DEBUG) std::cout << std::dec << "[gemTreeReader]: AMC Directory " << diramc << " created" << std::endl;
        m_amcH = new AMC_histogram(ofilename, gDirectory->mkdir(diramc), aslot_ch);
        m_amcH->bookHistograms();
        if (DEBUG) std::cout << std::dec << "[gemTreeReader]: AMC13 AMCs size " << m_amc13H->amcsH().size() << std::endl;
        m_RunType = v_amc[a_c].Rtype();  //obtain the run type 
        g_c=0;

        /* LOOP THROUGH GEBs */
        for(auto g=v_geb.begin(); g!=v_geb.end();g++){
          v_vfat=g->vfats();
          char dirgeb[30];    //filename for GEB directory
          dirgeb[0]='\0';    
          char g_ch[2];       //char used to put GEB number into directory name
          g_ch[0]='\0';
          int g_inputID = g->InputID();
          sprintf(g_ch, "%d", g_inputID);
          strcat(dirgeb,"GTX-");
          strcat(dirgeb,g_ch);
          //char buff[10];
          //buff[0] = '\0';
          //strcpy(buff, aslot_ch);
          //strcat(buff,g_ch);
          //strcpy(g_ch, buff);
          geb_map.insert(std::make_pair(g_ch, g_c));
          if (DEBUG) std::cout << std::dec << "[gemTreeReader]: GEB Directory " << dirgeb << " created" << std::endl;
          //GEB HISTOGRAMS HERE
          m_gebH = new GEB_histogram(ofilename, gDirectory->mkdir(dirgeb), g_ch);
          m_gebH->bookHistograms();
          if (DEBUG) std::cout << std::dec << "[gemTreeReader]: AMC GEBs size " << m_amcH->gebsH().size() << std::endl;

          v_c=0;

          /* LOOP THROUGH VFATs */
          //for(auto v=v_vfat.begin(); v!=v_vfat.end();v++){
          for(int i=0; i<24; i++){
            char dirvfat[30];   //filename for VFAT directory
            dirvfat[0]='\0';    
            char vslot_ch[24];   //char used to put VFAT number into directory name
            vslot_ch[0] = '\0';
            std::unique_ptr<gem::readout::GEMslotContents> slotInfo_ = std::unique_ptr<gem::readout::GEMslotContents> (new gem::readout::GEMslotContents("slot_table.csv"));     
            int t_chipID = slotInfo_->GEBChipIdFromSlot(i);
            //int vslot = slotInfo_->GEBslotIndex(v->ChipID());  //converts Chip ID into VFAT slot number
            int vslot = slotInfo_->GEBslotIndex(t_chipID);

            


            sprintf(vslot_ch, "%d", vslot);
            strcat(dirvfat,"VFAT-");
            strcat(dirvfat, vslot_ch);
            
            // Make sure not to attempt to create directory that already exists (segfaults)            
            if (std::find(vfat_useddirs.begin(),vfat_useddirs.end(), dirvfat) != vfat_useddirs.end()) {
              std::cout << "[bookAllHistograms]: Repeated VFAT slot, check slot tables!" << std::endl;
              strcat(dirvfat,"-");
              string str = std::to_string((long long int)i);
              const char* it = str.c_str();
              strcat(dirvfat,it);
            }
            vfat_useddirs.push_back(dirvfat);

            int vID = t_chipID;
            if (DEBUG) std::cout << std::dec << "[gemTreeReader]: VFAT chip ID " << std::hex << vID << std::dec << std::endl;
            char vID_ch[10];
            vID_ch[0] = '\0';
            sprintf(vID_ch, "%d", vID);
            char buff[10];
            buff[0] = '\0';
            strcpy(buff,g_ch);
            strcat(buff,vID_ch);
            strcpy(vID_ch,buff);
            if (DEBUG) std::cout << std::dec << "[gemTreeReader]: VFAT Directory " << dirvfat << " created" << std::endl;
            //VFAT HISTOGRAMS HERE
            m_vfatH = new VFAT_histogram(ofilename, gDirectory->mkdir(dirvfat), vslot_ch);
            m_vfatH->bookHistograms();
            //std::cout << "VFAT ID " << vID_ch << std::endl;
            vfat_map.insert(std::make_pair(vID_ch, v_c));
            m_gebH->addVFATH(*m_vfatH);
            if (DEBUG) std::cout << std::dec << "[gemTreeReader]: GEB VFATs size " << m_gebH->vfatsH().size() << std::endl;

            gDirectory->cd("..");   //moves back to previous directory
            v_c++;
          } /* END VFAT LOOP */
          gDirectory->cd("..");     //moves back to previous directory
          g_c++;
          m_amcH->addGEBH(*m_gebH);
        } /* END GEB LOOP */
        gDirectory->cd("..");       //moves back to previous directory
        a_c++;
        m_amc13H->addAMCH(*m_amcH);
      } /* END AMC LOOP */
      a13_c++;
    } /* END AMC13 LOOP */
  }

  //!Fills the histograms that were book from bookAllHistograms
  void fillAllHistograms()
  {
    int a13_c=0;    //counter through AMC13s
    int a_c=0;      //counter through AMCs
    int g_c=0;      //counter through GEBs
    int v_c=0;      //counter through VFATs

    TTree *tree = (TTree*)ifile->Get("GEMtree");
    Event *event = new Event();
    TBranch *branch = tree->GetBranch("GEMEvents");
    branch->SetAddress(&event);
    Int_t nentries = tree->GetEntries();
    /* LOOP THROUGH Events */
    for (int i = 0; i < nentries; i++){
      branch->GetEntry(i);
      v_amc13 = event->amc13s();
      /* LOOP THROUGH AMC13s */
      for(auto a13 = v_amc13.begin(); a13!=v_amc13.end(); a13++){
        v_amc = a13->amcs();
        v_amcH = m_amc13H->amcsH();
        m_amc13H->fillHistograms(&*a13);
        a_c=0;
        /* LOOP THROUGH AMCs */
        for(auto a=v_amc.begin(); a!=v_amc.end(); a++){
          v_geb = a->gebs();
          v_gebH = v_amcH[a_c].gebsH();
          //AMC_histogram * t_amcH = &(m_amc13H->amcsH().at(a_c));
          v_amcH[a_c].fillHistograms(&*a);

          if (m_RunType){
            m_deltaV = a->Param2() - a->Param3();
            m_Latency = a->Param1();
          }
          g_c=0;
          /* LOOP THROUGH GEBs */
          for(auto g=v_geb.begin(); g!=v_geb.end();g++){
            v_vfat = g->vfats();
            int gID = g->InputID();
            char gID_ch[10];
            gID_ch[0] = '\0';
            sprintf(gID_ch, "%d", gID);
            auto gebH_ = geb_map.find(gID_ch);
            if(gebH_ != geb_map.end()) {
              v_gebH[gebH_->second].fillHistograms(&*g);
              v_vfatH = v_gebH[gebH_->second].vfatsH();
            }
            else {
              std::cout << "GEB Not found\n";
              continue;
            }
            /* LOOP THROUGH VFATs */
            for(auto v=v_vfat.begin(); v!=v_vfat.end();v++){
              int vID = v->ChipID();
              char vID_ch[10];
              vID_ch[0] = '\0';
              sprintf(vID_ch, "%d", vID);
              char buff[10];
              buff[0] = '\0';
              strcpy(buff,gID_ch);
              strcat(buff,vID_ch);
              strcpy(vID_ch,buff);
              auto vfatH_ = vfat_map.find(vID_ch);
              if(vfatH_ != vfat_map.end()) {
                bool final = i == nentries-1;
                v_vfatH[vfatH_->second].fillHistograms(&*v,final);
                if (m_RunType){
                  v_vfatH[vfatH_->second].fillScanHistograms(&*v, m_RunType, m_deltaV, m_Latency);
                }
              }
              else {
                std::cout << "VFAT Not found\n";
                std::cout << "Chip ID " << vID_ch <<"\n";
              }
            } /* END VFAT LOOP */
            
          } /* END GEB LOOP */
          a_c++;
        } /* END AMC LOOP */
        a13_c++;
      } /* END AMC13 LOOP */
    } /* END EVENTS LOOP */
    ofile->Write();
  }
};
