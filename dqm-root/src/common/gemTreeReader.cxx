//#ifndef gemTreeReader_h
//#define gemTreeReader_h

#ifndef DEBUG
#define DEBUG 0
#endif
#include <TChain.h>
#include <TProofOutputFile.h>
#include <TSelector.h>

#include <TTreeReader.h>
#include <TTreeReaderValue.h>
#include <TTreeReaderArray.h>


#include <iomanip> 
#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <cstdlib>
#include <sstream>
#include <vector>
#include <TSystem.h>

#include "AMC13_histogram.cxx"

class gemTreeReader: public TSelector {
public :
  TTree          *fChain;   //!pointer to the analyzed TTree or TChain
   TTreeReader     fReader;  //!the tree reader
  
   // Readers to access the data (delete the ones you do not need).
   TTreeReaderValue<unsigned int> fUniqueID = {fReader, "fUniqueID"};
   TTreeReaderValue<unsigned int> fBits = {fReader, "fBits"};
   TTreeReaderValue<Int_t> fEvtHdr_fEvtNum = {fReader, "fEvtHdr.fEvtNum"};
   TTreeReaderValue<Int_t> fEvtHdr_fRun = {fReader, "fEvtHdr.fRun"};
   TTreeReaderValue<Int_t> fEvtHdr_fDate = {fReader, "fEvtHdr.fDate"};
   TTreeReaderArray<UChar_t> famc13s_m_cb5 = {fReader, "famc13s.m_cb5"};
   TTreeReaderArray<UChar_t> famc13s_m_Evt_ty = {fReader, "famc13s.m_Evt_ty"};
   TTreeReaderArray<unsigned int> famc13s_m_LV1_id = {fReader, "famc13s.m_LV1_id"};
   TTreeReaderArray<unsigned short> famc13s_m_BX_id = {fReader, "famc13s.m_BX_id"};
   TTreeReaderArray<unsigned short> famc13s_m_Source_id = {fReader, "famc13s.m_Source_id"};
   TTreeReaderArray<UChar_t> famc13s_m_CalTyp = {fReader, "famc13s.m_CalTyp"};
   TTreeReaderArray<UChar_t> famc13s_m_nAMC = {fReader, "famc13s.m_nAMC"};
   TTreeReaderArray<unsigned int> famc13s_m_OrN = {fReader, "famc13s.m_OrN"};
   TTreeReaderArray<UChar_t> famc13s_m_cb0 = {fReader, "famc13s.m_cb0"};
   TTreeReaderArray<vector<unsigned int>> famc13s_m_AMC_size = {fReader, "famc13s.m_AMC_size"};
   TTreeReaderArray<vector<unsigned char>> famc13s_m_Blk_No = {fReader, "famc13s.m_Blk_No"};
   TTreeReaderArray<vector<unsigned char>> famc13s_m_AMC_No = {fReader, "famc13s.m_AMC_No"};
   TTreeReaderArray<vector<unsigned short>> famc13s_m_BoardID = {fReader, "famc13s.m_BoardID"};
   TTreeReaderArray<vector<AMCdata>> famc13s_m_amcs = {fReader, "famc13s.m_amcs"};
   TTreeReaderArray<unsigned int> famc13s_m_CRC_amc13 = {fReader, "famc13s.m_CRC_amc13"};
   TTreeReaderArray<UChar_t> famc13s_m_Blk_NoT = {fReader, "famc13s.m_Blk_NoT"};
   TTreeReaderArray<UChar_t> famc13s_m_LV1_idT = {fReader, "famc13s.m_LV1_idT"};
   TTreeReaderArray<unsigned short> famc13s_m_BX_idT = {fReader, "famc13s.m_BX_idT"};
   TTreeReaderArray<UChar_t> famc13s_m_cbA = {fReader, "famc13s.m_cbA"};
   TTreeReaderArray<unsigned int> famc13s_m_EvtLength = {fReader, "famc13s.m_EvtLength"};
   TTreeReaderArray<unsigned short> famc13s_m_CRC_cdf = {fReader, "famc13s.m_CRC_cdf"};
   TTreeReaderValue<Bool_t> fisEventGood = {fReader, "fisEventGood"};


  // Declaration of leaf types
  Event           *GEMEvents;
  // List of branches
  TBranch        *b_GEMEvents;   //!
  gemTreeReader(TTree * /*tree*/ =0) : fChain(0) { }
  //gemTreeReader(TTree * /*tree*/ =0) : { }
  virtual ~gemTreeReader() { }
  virtual Int_t   Version() const { return 2; }
  virtual void    Begin(TTree *tree);
  virtual void    SlaveBegin(TTree *tree);
  virtual void    Init(TTree *tree);
  virtual Bool_t  Notify();
  virtual Bool_t  Process(Long64_t entry);
  virtual Int_t   GetEntry(Long64_t entry, Int_t getall = 0) { return fChain ? fChain->GetTree()->GetEntry(entry, getall) : 0; }
  virtual void    SetOption(const char *option) { fOption = option; }
  virtual void    SetObject(TObject *obj) { fObject = obj; }
  virtual void    SetInputList(TList *input) { fInput = input; }
  virtual TList  *GetOutputList() const { return fOutput; }
  virtual void    SlaveTerminate();
  virtual void    Terminate();

  //int slotFromMap(int a, int g, int cid);

  vector<AMC13Event> v_amc13;    ///<Vector of AMC13Event
  vector<AMCdata> v_amc;         ///<Vector of AMCdata
  vector<GEBdata> v_geb;         ///<Vector of GEBdata
  vector<VFATdata> v_vfat;       ///Vector of VFATdata

  AMC_histogram * v_amcH;        ///<Vector of AMC_histogram
  GEB_histogram * v_gebH;        ///<Vector of GEB_histogram
  VFAT_histogram * v_vfatH;      ///<Vector of VFAT_histogram

  //int VFATMap[12][12][24];

  AMC13_histogram * m_amc13H;
  AMC_histogram * m_amcH;
  GEB_histogram * m_gebH;
  VFAT_histogram * m_vfatH;

  int m_RunType;
  int m_deltaV;
  int m_Latency;
  long long int m_OrbitNumber;
  long long int m_RelOrbitNumber;
  TDirectory* onlineHistsDir;

  TFile *fFile;
  TProofOutputFile *fProofFile;

  ClassDef(gemTreeReader,2);
};

//#endif//
//#include "gemTreeReader.h"
void gemTreeReader::Begin(TTree * /*tree*/)
{
  // The Begin() function is called at the start of the query.
  // When running with PROOF Begin() is only called on the client.
  // The tree argument is deprecated (on PROOF 0 is passed).

  if (DEBUG) std::cout << "MASTER BEGIN"<< std::endl;
  TString option = GetOption();
} 

void gemTreeReader::SlaveBegin(TTree * /*tree*/)
{
  // The SlaveBegin() function is called after the Begin() function.
  // When running with PROOF SlaveBegin() is called on each slave server.
  // The tree argument is deprecated (on PROOF 0 is passed).

  if (DEBUG) std::cout << "SLAVE BEGIN"<< std::endl;
  TString option = GetOption();
  gSystem->Load("libEvent.so");

  // The file for merging
  fProofFile = new TProofOutputFile("SimpleFile.root", "M");
  TNamed *out = (TNamed *) fInput->FindObject("PROOF_OUTPUTFILE");
  if (out) fProofFile->SetOutputFileName(out->GetTitle());
  TDirectory *savedir = gDirectory;
  fFile = fProofFile->OpenFile("RECREATE");
  if (fFile && fFile->IsZombie()) SafeDelete(fFile);
  savedir->cd();

  // Cannot continue
  if (!fFile) {
     TString amsg = TString::Format("ProofSimpleFile::SlaveBegin: could not create '%s':"
                                    " instance is invalid!", fProofFile->GetName());
     Abort(amsg, kAbortProcess);
     return;
  } 
  fFile->cd();

  //VFATMap = {{{0}}};
  TObjString * diramc13 = new TObjString("AMC13-1");
  m_amc13H = new AMC13_histogram("preved", gDirectory->mkdir(diramc13->String()), "1");
  m_amc13H->bookHistograms();
  if (DEBUG) std::cout << "Slave Begin: try to get config "<< std::endl;
  TList *config_s = (TList*)fInput->FindObject("config");
  if (DEBUG) std::cout << "Slave Begin: retrieved config name " << config_s->GetName() << std::endl;
  if (DEBUG) std::cout << "Slave Begin: try to get config iterator"<< std::endl;

  m_amcH = new AMC_histogram("preved", gDirectory->mkdir("AMC-5"), "5");
  m_amcH->bookHistograms();
  for (int j=0; j<12; j++){
    std::string str0 = "GEB-";
    str0.append(to_string(j).c_str());
    m_gebH = new GEB_histogram("preved", gDirectory->mkdir(str0.c_str()), to_string(j).c_str());
    m_gebH->bookHistograms();
    for (int i=0; i<24; i++){
      std::string str = "VFAT-";
      str.append(to_string(i).c_str());
      m_vfatH = new VFAT_histogram("preved", gDirectory->mkdir(str.c_str()), to_string(i).c_str());
      m_vfatH->bookHistograms();
      m_gebH->addVFATH(m_vfatH,i);
      gDirectory->cd("..");   //moves back to previous directory
    }
    gDirectory->cd("..");     //moves back to previous directory
    m_amcH->addGEBH(m_gebH,j);
  }
  gDirectory->cd("..");       //moves back to previous directory
  m_amc13H->addAMCH(m_amcH, 5);

  //TIter nextamc(config_s);
  //TObject *amc;
  //while ((amc = nextamc())) 
  //{
  //  std::cout << "Slave Begin: found object " << amc->GetName() << std::endl;
  //  TString a_slot_s = (TString) amc->GetName();
  //  int a_slot = a_slot_s.Atoi(); // retrieve a_slot from the config somehow
  //  a_slot_s.Insert(0,"AMC-");
  //  m_amcH = new AMC_histogram("preved", gDirectory->mkdir(a_slot_s.Data()), amc->GetName());
  //  m_amcH->bookHistograms();
  //  TIter nextgeb((TList*)amc);
  //  TObject *geb;
  //  while ((geb = nextgeb())) 
  //  {
  //    std::cout << "Slave Begin: found object " << geb->GetName() << std::endl;
  //    TString g_slot_s = (TString)geb->GetName();
  //    int g_slot = g_slot_s.Atoi(); // retrieve g_slot from the config somehow
  //    // FIXME!!! Tmp plug to correct offset introduced in the DB
  //    g_slot = g_slot -1;
  //    g_slot_s.Form("%d",g_slot);
  //    g_slot_s.Insert(0,"GTX-");
  //    // end of FIXME
  //    m_gebH = new GEB_histogram("preved", gDirectory->mkdir(g_slot_s.Data()), geb->GetName());
  //    m_gebH->bookHistograms();
  //    TMapIter nextvfat((TMap*)geb);
  //    TPair *vfat;
  //    while ((vfat = (TPair*)nextvfat())) 
  //    {
  //      //TObject *chipID_s = ((TPair*)geb->FindObject(vfat))->Value();
  //      TString v_slot_s = (TString)vfat->GetName();
  //      //TString s_chipID_s = (TString)chipID_s->GetName();
  //      //if (DEBUG) cout << "s_chipID_s " << s_chipID_s << endl;
  //      //s_chipID_s = TString::BaseConvert(s_chipID_s, 16,10);
  //      int v_slot = v_slot_s.Atoi(); // retrieve v_slot from the config somehow
  //      //int i_chipID = s_chipID_s.Atoi();
  //      //i_chipID = i_chipID & 0x0FFF;
  //      //VFATMap[a_slot][g_slot][v_slot] = i_chipID;
  //      //if (DEBUG) cout << "Insert chip ID " << i_chipID << " in place a,g,v: " << a_slot << ", " << g_slot << ", " << v_slot << endl;
  //      v_slot_s.Insert(0,"VFAT-");
  //      m_vfatH = new VFAT_histogram("preved", gDirectory->mkdir(v_slot_s.Data()), to_string(v_slot).c_str());
  //      m_vfatH->bookHistograms();
  //      m_gebH->addVFATH(m_vfatH,v_slot);
  //      std::cout << "Slave Begin: add vfat " << v_slot << std::endl;
  //      gDirectory->cd("..");   //moves back to previous directory
  //    } /* END VFAT LOOP */
  //    gDirectory->cd("..");     //moves back to previous directory
  //    m_amcH->addGEBH(m_gebH,g_slot);
  //    std::cout << "Slave Begin: add geb " << g_slot << std::endl;
  //  } /* END GEB LOOP */
  //  gDirectory->cd("..");       //moves back to previous directory
  //  m_amc13H->addAMCH(m_amcH, a_slot);
  //  std::cout << "Slave Begin: add amc " << a_slot << std::endl;
  //} /* END AMC LOOP */

  gDirectory = savedir;
  if (DEBUG) std::cout << "SLAVE END"<< std::endl;
}

//!Fills the histograms that were book from bookAllHistograms
Bool_t gemTreeReader::Process(Long64_t entry)
{
  //fReader.SetEntry(entry);
  fChain->GetEntry(entry);
  int a13_c=0;    //counter through AMC13s
  int a_c=0;      //counter through AMCs
  int g_c=0;      //counter through GEBs
  int v_c=0;      //counter through VFATs

  v_amc13 = GEMEvents->amc13s();
  if (DEBUG) cout << "Get a vector of AMC13 "<< endl;
  /* LOOP THROUGH AMC13s */
  for(auto a13 = v_amc13.begin(); a13!=v_amc13.end(); a13++){
    if (DEBUG) cout << "Get AMC13 "<< endl;
    m_amc13H->fillHistograms(&*a13);
    if (DEBUG) cout << "AMC13 histograms filled "<< endl;
    v_amc = a13->amcs();
    /* LOOP THROUGH AMCs */
    for(auto a=v_amc.begin(); a!=v_amc.end(); a++){
      if (DEBUG) cout << "Get AMC "<< endl;
      v_geb = a->gebs();
      if (DEBUG) cout << "Get GEB "<< endl;
      a_c=a->AMCnum();
      v_amcH = m_amc13H->amcsH(a_c);
      if (DEBUG) cout << "Get AMC H "<< endl;
      if (v_amcH) v_amcH->fillHistograms(&*a);
      if (DEBUG) cout << "Fill AMC histograms"<< endl;
      m_RunType = a->Rtype();
      if (m_RunType){
        m_deltaV = a->Param2() - a->Param1();
        m_Latency = a->Param3();
      }
      //if ( (m_Latency > 166) && (m_Latency < 172) ) {continue;}
      g_c=0;
      /* LOOP THROUGH GEBs */
      for(auto g=v_geb.begin(); g!=v_geb.end();g++){
        v_vfat = g->vfats();
        int gID = g->InputID();
        //if (gID!=4) continue;
        v_gebH = v_amcH->gebsH(gID);
        std::map<int,int> slot_map;
        //for(auto v=v_vfat.begin(); v!=v_vfat.end();v++){
        //  int vID = v->ChipID();
        //  //vID = vID | 0xf000;
        //  if (DEBUG) cout << "Look for slot number of " << v->ChipID() << ", in AMC "<< a_c << " and GEB " << gID <<  endl;
        //  int slot = slotFromMap(a_c, gID, vID);
        //  slot_map.insert(std::make_pair(v->ChipID(), slot));
        //  if (DEBUG) cout << "Inserted in map: chip ID " << v->ChipID() << ", slot "<< slot <<  endl;
        //}
        if (DEBUG) std::cout << "v_gebH " << std::hex << std::setw(8) << std::setfill('0') << v_gebH << std::dec << std::endl;
        if (v_gebH) v_gebH->fillHistograms(&*g, slot_map);
        /* LOOP THROUGH VFATs */
        for(auto v=v_vfat.begin(); v!=v_vfat.end();v++){
          //int slot = slot_map.find(v->ChipID())->second;
          int slot = v->Pos();
          //if (DEBUG) cout << "Try get slot for chip ID " << v->ChipID() << ", retrieved slot " << slot <<  endl;
          //if ( (slot == 6) || (slot == 7) || (slot == 8) || (slot == 14) || (slot == 15) || (slot == 19) || (slot == 22) || (slot == 23) ) continue;
          if (slot>-1) {v_vfatH = v_gebH->vfatsH(slot);} else { continue;}
          if (v_vfatH) {
            v_vfatH->fillHistograms(&*v, m_RelOrbitNumber);
            if (m_RunType == 1 || m_RunType == 2 || m_RunType == 3){
              v_vfatH->fillScanHistograms(&*v, m_RunType, m_deltaV, m_Latency);
            }
          }
        } /* END VFAT LOOP */
      } /* END GEB LOOP */
      a_c++;
    } /* END AMC LOOP */
    a13_c++;
  } /* END AMC13 LOOP */

  return kTRUE;
}

void gemTreeReader::SlaveTerminate()
{
  // The SlaveTerminate() function is called after all entries or objects
  // have been processed. When running with PROOF SlaveTerminate() is called
  // on each slave server.
  //
  // Should we fill summary canvases here?
  //
  //for (unsigned int i = 0; i < 12; i++){
  //    if (auto a = m_amc13H->amcsH(i)){
  //        for (unsigned int j = 0; j < 12; j++){
  //            if (auto g = a->gebsH(j)){
  //                g->fillSummaryCanvases();
  //                for (unsigned int k = 0; k < 24; k++){
  //                    if (auto v = g->vfatsH(k)){
  //                        v->fillWarnings();
  //                    }
  //                }
  //            }
  //        }
  //    }
  //}
  TDirectory *savedir = gDirectory;
  fFile->cd();
  fFile->Write();
  fOutput->Add(fProofFile);
  fFile->Close();
}
void gemTreeReader::Terminate()
{
  // The Terminate() function is the last function to be called during
  // a query. It always runs on the client, it can be used to present
  // the results graphically or save the results to file.

  if ((fProofFile = dynamic_cast<TProofOutputFile*>(fOutput->FindObject("SimpleFile.root")))) {
    TString outputFile(fProofFile->GetOutputFileName());
    TString outputName(fProofFile->GetName());
    Printf("outputFile: %s", outputFile.Data());
  }
}
//int gemTreeReader::slotFromMap(int a, int g, int cid)
//{
//  int res = -1;
//  for (int i=0; i<24; i++){
//    if (VFATMap[a][g][i] == cid) {res = i;}
//  }
//  return res;
//}
void gemTreeReader::Init(TTree *tree)
{
   // The Init() function is called when the selector needs to initialize
   // a new tree or chain. Typically here the branch addresses and branch
   // pointers of the tree will be set.
   // It is normally not necessary to make changes to the generated
   // code, but the routine can be extended by the user if needed.
   // Init() will be called many times when running on PROOF
   // (once per file to be processed).

   // Set branch addresses and branch pointers
   if (!tree) return;
   fChain = tree;
   fChain->SetMakeClass(0);

   GEMEvents = new Event();

   fChain->SetBranchAddress("GEMEvents", &GEMEvents, &b_GEMEvents);
}

Bool_t gemTreeReader::Notify()
{
   // The Notify() function is called when a new file is opened. This
   // can be either for a new TTree in a TChain or when when a new TTree
   // is started when using PROOF. It is normally not necessary to make changes
   // to the generated code, but the routine can be extended by the
   // user if needed. The return value is currently not used.

   return kTRUE;
}


