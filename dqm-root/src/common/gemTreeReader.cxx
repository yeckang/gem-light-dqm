#define NVFAT 24
#define NSLOTS 12
#define NGTX 2
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

// #include "gem/readout/GEMslotContents.h"
#include "GEMClusterization/GEMStrip.h"
#include "GEMClusterization/GEMStripCollection.h"
#include "GEMClusterization/GEMClusterContainer.h"
#include "GEMClusterization/GEMClusterizer.h"
#include "plotter.cxx"
#include "logger.cxx"
#include "integrity_checker.cxx"
#include "GEMDQMerrors.cxx"
#include "AMC13_histogram.cxx"
// #include "db_interface.cxx"

using namespace std;

//!A class that creates the subdirectories and histograms from VFAT, GEM, AMC and AMC13 data
class treeReader
{
public:

  //!The constructor requires a file name ending with .raw.root
  treeReader(const std::string &ifilename)
  {
    if(ifilename.find("chunk") == string::npos)
      RunName = ifilename.substr(ifilename.find("run"),ifilename.size()-9);
    else
      RunName = ifilename.substr(ifilename.find("run"),ifilename.find("chunk")-1);
    
    std::cout << "RunName:" << RunName << std::endl;
    std::string tmp = ifilename.substr(ifilename.size()-9, ifilename.size());
    if (tmp != ".raw.root") throw std::runtime_error("Wrong input filename (should end with '.raw.root'): "+ifilename);
    ifile = new TFile(ifilename.c_str(), "READ");
    ofilename = ifilename.substr(0,ifilename.size()-9);
    ofilename += ".analyzed.root";
    ofile = new TFile(ofilename.c_str(), "RECREATE");
    if (DEBUG) std::cout << std::dec << "[gemTreeReader]: File for histograms created" << std::endl;   


    if (DEBUG) std::cout << std::dec << "[gemTreeReader]: Connecting to database on port " << PORT << std::endl;
    Database = connectDB();
    
    if (DEBUG) std::cout << std::dec << "[gemTreeReader]: Fetching hardware" << std::endl;   
    this->fetchHardwareDB();

    // if (DEBUG) std::cout << std::dec << "[gemTreeReader]: Booking histograms" << std::endl;   
    // this->bookAllHistograms();
    this->fillAllHistograms();




  }
  ~treeReader(){}

private:
  TFile *ifile;   ///<Input File, assigned in the constructor
  TFile *ofile;   ///<Output File, assigned in the constructor, where all the histograms will go
  std::string ofilename;  ///<Name of output file, same as input file, but is .analyzed.root instead of .raw.root

  std::string RunName;

  //std::vector<TDirectory*> AMC13dir;  //unused  
  //std::vector<TDirectory*> AMCdir;    //unused
  //std::vector<TDirectory*> GEBdir;    //unused
  //std::vector<TDirectory*> VFATdir;   //unused

  vector<AMC13Event> v_amc13;    ///<Vector of AMC13Event
  vector<AMCdata> v_amc;         ///<Vector of AMCdata
  vector<GEBdata> v_geb;         ///<Vector of GEBdata
  vector<VFATdata> v_vfat;       ///Vector of VFATdata


  //vector<AMC13_histogram> v_amc13H;   //unused
  AMC_histogram * v_amcH;        ///<Vector of AMC_histogram
  GEB_histogram * v_gebH;        ///<Vector of GEB_histogram
  VFAT_histogram * v_vfatH;      ///<Vector of VFAT_histogram

  
  //vector<vector<int> > VFATids(
  
  //unordered_map<std::string, int> vfat_map;
  int VFATMap[12][2][24];
  unordered_map<std::string, int> geb_map;

  AMC13_histogram * m_amc13H;
  AMC_histogram * m_amcH;
  GEB_histogram * m_gebH;
  VFAT_histogram * m_vfatH;

  int m_RunType;
  int m_deltaV;
  int m_Latency;

  MYSQL *Database;

  
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

  void fetchHardwareDB()
  {
    VFATMap = {{{0}}};
    char * diramc13 = "AMC13-1";
    
    m_amc13H = new AMC13_histogram(ofilename, gDirectory->mkdir(diramc13), "1");
    m_amc13H->bookHistograms();


    string AMC13Query = "select id from ldqm_db_run where Name like '";
    AMC13Query += RunName;
    AMC13Query += "'";

    string RunIDstr = stringFromChar(simpleDBQuery(Database, AMC13Query));
    
    string AMCQuery = "select amc_id from ldqm_db_run_amcs where run_id like '"+RunIDstr+"'";
    //string AMCQuery = "select amc_id from ldqm_db_run_amcs where run_id like ";
    cout << AMCQuery <<endl;
    vector<string> AMCs = manyDBQuery(Database,AMCQuery);
    cout << "Size: " << AMCs.size() << endl;
    for ( int amc = 0; amc < AMCs.size(); amc++ )
      {
        string currentAMCid = stringFromChar(AMCs.at(amc).c_str());
        cout << "AMC id: " << currentAMCid << endl;
        string AMCBidQuery = "select BoardID from ldqm_db_amc where id like '"+currentAMCid+"'";
        char* AMCBidchar = simpleDBQuery(Database,AMCBidQuery);
        cout << "AMC Board ID: " << AMCBidchar << endl;
        string AMCBidstr(AMCBidchar);
        string aslot_ch = AMCBidstr.substr(AMCBidstr.find("-")+1,AMCBidstr.size());
        cout << "aslot_ch: " << aslot_ch << endl;
        m_amcH = new AMC_histogram(ofilename, gDirectory->mkdir(AMCBidchar), aslot_ch.c_str());
        m_amcH->bookHistograms();

        string GEBQuery = "select geb_id from ldqm_db_amc_gebs where amc_id like '"+currentAMCid+"'";
        vector<string> GEBs = manyDBQuery(Database,GEBQuery);

        for ( int geb = 0; geb < GEBs.size(); geb++ )
          {
            string currentGEBid = stringFromChar(GEBs.at(geb).c_str());
            cout << "GEB id: " << currentGEBid << endl;
            string GEBBidQuery = "select ChamberID from ldqm_db_geb where id like '"+currentGEBid+"'";
            char* GEBBidchar = simpleDBQuery(Database,GEBBidQuery);
            cout << "GEB Board ID: " << GEBBidchar << endl;
            string GEBBidstr(GEBBidchar);
            string gslot_ch = GEBBidstr.substr(GEBBidstr.find("-")+1,GEBBidstr.size());
            cout << "gslot_ch: " << gslot_ch << endl;
            m_gebH = new GEB_histogram(ofilename, gDirectory->mkdir(GEBBidchar), gslot_ch.c_str());
            m_gebH->bookHistograms();


            string VFATQuery = "select vfat_id from ldqm_db_geb_vfats where geb_id like '"+currentGEBid+"'";
            vector<string> VFATs = manyDBQuery(Database,VFATQuery);

            for ( int vfat = 0; vfat < VFATs.size(); vfat++ )
              {
                string currentVFATid = stringFromChar(VFATs.at(vfat).c_str());
                cout << "VFAT id: " << currentVFATid << endl;
                string VFATSlotQuery = "select Slot from ldqm_db_vfat where id like '"+currentVFATid+"'";
                char* VFATSlotchar = simpleDBQuery(Database,VFATSlotQuery);
                cout << "VFAT Slot: " << VFATSlotchar << endl;


                char VFATdirname[30];    //filename for GEB directory
                VFATdirname[0]='\0';
                strcat(VFATdirname,"VFAT-");
                strcat(VFATdirname,VFATSlotchar);
                cout << "VFAT Dir Name: " << VFATdirname << endl;

                
                string VFATChipQuery = "select ChipID from ldqm_db_vfat where id like '"+currentVFATid+"'";
                char* VFATChipchar = simpleDBQuery(Database,VFATChipQuery);
                cout << "VFAT ChipID: " << VFATChipchar << endl;

                VFATMap[atoi(aslot_ch.c_str())][atoi(gslot_ch.c_str())][atoi(VFATSlotchar)] = atoi(VFATChipchar);

                m_vfatH = new VFAT_histogram(ofilename, gDirectory->mkdir(VFATdirname), VFATSlotchar);
                m_vfatH->bookHistograms();
            

                m_gebH->addVFATH(m_vfatH,atoi(VFATSlotchar));
                //if (DEBUG) std::cout << std::dec << "[gemTreeReader]: GEB VFATs size " << m_gebH->vfatsH().size() << std::endl;

                gDirectory->cd("..");   //moves back to previous directory
              } /* END VFAT LOOP */
            gDirectory->cd("..");     //moves back to previous directory
            m_amcH->addGEBH(m_gebH,atoi(gslot_ch.c_str()));
          } /* END GEB LOOP */
        gDirectory->cd("..");       //moves back to previous directory
        m_amc13H->addAMCH(m_amcH, atoi(aslot_ch.c_str()));
      } /* END AMC LOOP */
  } /* END AMC13 LOOP */

  int slotFromMap(int a, int g, int cid)
  {
    for (int i=0; i<24; i++){
      if (VFATMap[a][g][i] == cid) {return i;}
    }
  }

  
  // //!Creates the subdirectories for AMC13, AMC, GEB, and VFAT and books the histograms
  // void bookAllHistograms()
  // {
  //   int a13_c=0;    //counter through AMC13s
  //   int a_c=0;      //counter through AMCs
  //   int g_c=0;      //counter through GEBs
  //   int v_c=0;      //counter through VFATs
    
  //   std::vector<string> vfat_useddirs;
    
  //   /* LOOP THROUGH AMC13s */
  //   for(auto a13 = v_amc13.begin(); a13!=v_amc13.end(); a13++){
  //     v_amc = a13->amcs();

  //     char diramc13[30];        //filename for AMC13 directory
  //     diramc13[0]='\0';         
  //     char serial_ch[20];       //char used to put serial number into directory name
  //     serial_ch[0] = '\0';
  //     int serial = v_amc13[a13_c].nAMC();  //obtains the serial number from the AMC13 Event
  //     sprintf(serial_ch, "%d", serial);
  //     strcat(diramc13,"AMC13-");
  //     strcat(diramc13,serial_ch);
  //     if (DEBUG) std::cout << std::dec << "[gemTreeReader]: AMC13 Directory " << diramc13 << " created" << std::endl;
  //     //AMC13 HISTOGRAMS HERE
  //     m_amc13H = new AMC13_histogram(ofilename, gDirectory->mkdir(diramc13), serial_ch);
  //     m_amc13H->bookHistograms();

  //     a_c=0;

  //     /* LOOP THROUGH AMCs */
  //     for(auto a=v_amc.begin(); a!=v_amc.end(); a++){
  //       v_geb = a->gebs();
  //       char diramc[30];        //filename for AMC directory  
  //       diramc[0]='\0';
  //       char aslot_ch[2];       //char used to put AMC slot number inot directory name
  //       aslot_ch[0] = '\0';
  //       int aslot = v_amc[a_c].AMCnum();  //obtains the slot number from the AMCdata
  //       sprintf(aslot_ch, "%d", aslot);
  //       strcat(diramc,"AMC-");
  //       strcat(diramc, aslot_ch);

  //       std::string AMCID = diramc;
        
        


  //       if (DEBUG) std::cout << std::dec << "[gemTreeReader]: AMC Directory " << diramc << " created" << std::endl;
  //       m_amcH = new AMC_histogram(ofilename, gDirectory->mkdir(diramc), aslot_ch);
  //       m_amcH->bookHistograms();
  //       if (DEBUG) std::cout << std::dec << "[gemTreeReader]: AMC13 AMCs size " << m_amc13H->amcsH().size() << std::endl;
  //       m_RunType = v_amc[a_c].Rtype();  //obtain the run type 
  //       g_c=0;

  //       /* LOOP THROUGH GEBs */
  //       for(auto g=v_geb.begin(); g!=v_geb.end();g++){
  //         v_vfat=g->vfats();
  //         char dirgeb[30];    //filename for GEB directory
  //         dirgeb[0]='\0';    
  //         char g_ch[2];       //char used to put GEB number into directory name
  //         g_ch[0]='\0';
  //         int g_inputID = g->InputID();
  //         sprintf(g_ch, "%d", g_inputID);
  //         strcat(dirgeb,"GTX-");
  //         strcat(dirgeb,g_ch);

  //         geb_map.insert(std::make_pair(g_ch, g_c));
  //         if (DEBUG) std::cout << std::dec << "[gemTreeReader]: GEB Directory " << dirgeb << " created" << std::endl;
  //         //GEB HISTOGRAMS HERE
  //         m_gebH = new GEB_histogram(ofilename, gDirectory->mkdir(dirgeb), g_ch);
  //         m_gebH->bookHistograms();
  //         if (DEBUG) std::cout << std::dec << "[gemTreeReader]: AMC GEBs size " << m_amcH->gebsH().size() << std::endl;

  //         std::string GEBID = dirgeb;
          
  //         v_c=0;

  //         /* LOOP THROUGH VFATs */
  //         //for(auto v=v_vfat.begin(); v!=v_vfat.end();v++){
  //         for(int i=0; i<24; i++){
  //           char dirvfat[30];   //filename for VFAT directory
  //           dirvfat[0]='\0';    
  //           char vslot_ch[24];   //char used to put VFAT number into directory name
  //           vslot_ch[0] = '\0';

            
  //           // std::unique_ptr<gem::readout::GEMslotContents> slotInfo_ = std::unique_ptr<gem::readout::GEMslotContents> (new gem::readout::GEMslotContents("slot_table.csv"));     
  //           // int t_chipID = slotInfo_->GEBChipIdFromSlot(i);
  //           // //int vslot = slotInfo_->GEBslotIndex(v->ChipID());  //converts Chip ID into VFAT slot number
  //           // int vslot = slotInfo_->GEBslotIndex(t_chipID);

  //           int t_chipID = getVFATChipID(Database,RunName,AMCID,GEBID,i);
  //           int vslot = i;

	    

  //           sprintf(vslot_ch, "%d", vslot);
  //           strcat(dirvfat,"VFAT-");
  //           strcat(dirvfat, vslot_ch);
            
  //           // Make sure not to attempt to create directory that already exists (segfaults)            
  //           if (std::find(vfat_useddirs.begin(),vfat_useddirs.end(), dirvfat) != vfat_useddirs.end()) {
  //             std::cout << "[bookAllHistograms]: Repeated VFAT slot, check slot tables!" << std::endl;
  //             strcat(dirvfat,"-");
  //             string str = std::to_string((long long int)i);
  //             const char* it = str.c_str();
  //             strcat(dirvfat,it);
  //           }
  //           vfat_useddirs.push_back(dirvfat);

  //           int vID = t_chipID;
  //           if (DEBUG) std::cout << std::dec << "[gemTreeReader]: VFAT chip ID " << std::hex << vID << std::dec << std::endl;
  //           char vID_ch[10]; //encoded VFAT location <amcnum><gtxnum><chipid>   
  //           vID_ch[0] = '\0';
  //           sprintf(vID_ch, "%d", vID);
  //           char buff[10];
  //           buff[0] = '\0';

  //           strcpy(buff,aslot_ch);
  //           strcat(buff,g_ch);
  //           strcat(buff,vID_ch);
  //           strcpy(vID_ch,buff);
  //           std::cout << "DEBUG: vID_ch=" << vID_ch << " vslot=" << vslot << std::endl;
  //           vfat_map.insert(std::make_pair(vID_ch, vslot)); //assign slot to encoded location

  //           if (DEBUG) std::cout << std::dec << "[gemTreeReader]: VFAT Directory " << dirvfat << " created" << std::endl;
  //           //VFAT HISTOGRAMS HERE
  //           m_vfatH = new VFAT_histogram(ofilename, gDirectory->mkdir(dirvfat), vslot_ch);
  //           m_vfatH->bookHistograms();
  //           //std::cout << "VFAT ID " << vID_ch << std::endl;

  //           m_gebH->addVFATH(*m_vfatH);
  //           if (DEBUG) std::cout << std::dec << "[gemTreeReader]: GEB VFATs size " << m_gebH->vfatsH().size() << std::endl;

  //           gDirectory->cd("..");   //moves back to previous directory
  //           v_c++;
  //         } /* END VFAT LOOP */
  //         gDirectory->cd("..");     //moves back to previous directory
  //         g_c++;
  //         m_amcH->addGEBH(*m_gebH);
  //       } /* END GEB LOOP */
  //       gDirectory->cd("..");       //moves back to previous directory
  //       a_c++;
  //       m_amc13H->addAMCH(*m_amcH);
  //     } /* END AMC LOOP */
  //     a13_c++;
  //   } /* END AMC13 LOOP */
  // }

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
        m_amc13H->fillHistograms(&*a13);
        /* LOOP THROUGH AMCs */
        for(auto a=v_amc.begin(); a!=v_amc.end(); a++){
          v_geb = a->gebs();
	  std::cout << "v_geb size: " << v_geb.size() << std::endl;
          a_c=a->AMCnum();
          cout << "a_c = " << a_c << endl;
          //v_gebH = v_amcH[a_c].gebsH();
          //AMC_histogram * t_amcH = &(m_amc13H->amcsH().at(a_c));
          v_amcH = m_amc13H->amcsH(a_c);
          v_amcH->fillHistograms(&*a);

	  int serial = a->AMCnum();
	  char serial_ch[10];
	  serial_ch[0] = '\0';
	  sprintf(serial_ch, "%d", serial);


          if (m_RunType){
            m_deltaV = a->Param2() - a->Param3();
            m_Latency = a->Param1();
          }
          g_c=0;
          /* LOOP THROUGH GEBs */
          for(auto g=v_geb.begin(); g!=v_geb.end();g++){
            v_vfat = g->vfats();
            int gID = g->InputID();
            //char gID_ch[10];
            //gID_ch[0] = '\0';
            //sprintf(gID_ch, "%d", gID);
            v_gebH = m_amcH->gebsH(gID);
            v_gebH->fillHistograms(&*g);
            //auto gebH_ = geb_map.find(gID_ch);
            //if(gebH_ != geb_map.end()) {
            //  v_gebH[gebH_->second].fillHistograms(&*g);
            //  v_vfatH = v_gebH[gebH_->second].vfatsH();
            //}
            //else {
            //  std::cout << "GEB Not found\n";
	    //  std::cout << "gID_ch: " << gID_ch << std::endl;
            //  continue;
            //}
            /* LOOP THROUGH VFATs */
            for(auto v=v_vfat.begin(); v!=v_vfat.end();v++){
              int vID = v->ChipID();
	      vID = vID | 0xf000;
              int slot = slotFromMap(a_c, gID, vID);
              v_vfatH = m_gebH->vfatsH(slot);
              //char vID_ch[10]; //encoded VFAT location <amcnum><gtxnum><chipid>
              //vID_ch[0] = '\0';
              //sprintf(vID_ch, "%d", vID);
              //char buff[10];
              //buff[0] = '\0';
	      //strcpy(buff,serial_ch);
              //strcat(buff,gID_ch);
              //strcat(buff,vID_ch);
              //strcpy(vID_ch,buff);
              //auto vfatH_ = vfat_map.find(vID_ch);
              //if(vfatH_ != vfat_map.end()) {
              //  bool final = i == nentries-1;
              //  v_vfatH[vfatH_->second].fillHistograms(&*v,final);
              //  if (m_RunType){
              //    v_vfatH[vfatH_->second].fillScanHistograms(&*v, m_RunType, m_deltaV, m_Latency);
              //  }
              //}
              //else {
              //  std::cout << "VFAT Not found\n";
	      //  std::cout << "vID: " << vID << " = " << std::hex << vID << std::dec << std::endl;
              //  std::cout << "vID_ch: " << vID_ch <<"\n";
              //}
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
