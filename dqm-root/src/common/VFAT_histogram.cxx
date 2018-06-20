#include "Hardware_histogram.h"
#include "TH1.h"
#include "TH2.h"
#include <Event.h>
#define NCHANNELS 128

//!A class that creates histogram for VFAT data

class VFAT_histogram: public Hardware_histogram
{
public:
  //!Calls the base constructor
  VFAT_histogram(const std::string & filename, TDirectory * dir, const std::string & hwid):Hardware_histogram(filename, dir, hwid){}//call base constructor
  //!Calls the base constructor
  VFAT_histogram(VFAT_histogram * vH):Hardware_histogram("dummy", vH->m_dir, vH->m_HWID){}//call base constructor
  ~VFAT_histogram(){}

  //!Books histograms for VFAT data
  /*!
    This books histograms for the following data: Difference between crc and recalculated crc, Control Bit 1010, Control Bit 1100, Control Bit 1110, Bunch Crossing Number, Event Counter, Control Flags, and Chip ID, and Fired Channels. It also creates a subdirectory for Threshold Scans and books those histograms.
  */
  void bookHistograms(){
    m_dir->cd();
    n_hits_per_event    = new TH1F("n_hits_per_event", "n_hits_per_event", 129,  -0.5 , 128.5);
    //b1010    = new TH1F("b1010", "Control Bits", 15,  0x0 , 0xf);
    BC       = new TH1D("BC", "Bunch Crossing Number", 4096,  -0.5, 4095.5);
    EC       = new TH1F("EC", "Event Counter", 255,  0x0 , 0xff);
    Header     = new TH1F("Header", "Header", 32,  0x0 , 0xff);
    SlotN    = new TH1F("SlotN", "Slot Number", 24,  0, 24);
    FiredChannels = new TH1F("FiredChannels", "FiredChannels", 128, -0.5, 127.5);
    FiredStrips   = new TH1F("FiredStrips",   "FiredStrips",   128, -0.5, 127.5);
    crc      = new TH1F("crc", "check sum value", 0xffff,  0x0 , 0xffff);
    crc_calc = new TH1F("crc_calc", "check sum value recalculated", 0xffff,  0x0 , 0xffff);
    crc_difference = new TH1F("crc_difference", "difference between crc and crc_calc", 0xffff,  -32768 , 32768);
    latencyScan   = new TH1D("latencyScan",   "Latency Scan", 256,  -0.5, 255.5);
    latencyBXdiffScan   = new TH1D("latencyBXdiffScan",   "Latency Scan BX subtracted", 4352,  -256.5, 4095.5);
    //latencyScan2D = new TH2F("latencyScan2D", "Latency Scan", 256,  -0.5, 255.5, 128,  -0.5, 127.5);
    latencyScanBX2D = new TH2D("latencyScanBX2D", "Latency Scan vs BX", 256,  -0.5, 255.5, 4096,  -0.5,4095.5);
    latencyScanBX2D_extraHighOcc = new TH2F("latencyScanBX2D_extraHighOcc", "Latency Scan vs BX when number of fired channels is greater than 100", 256,  -0.5, 255.5, 4096,  -0.5,4095.5);
    thresholdScanChip   = new TH1F("thresholdScan",  "Threshold Scan",256, -0.5, 255.5);
    thresholdScanChip2D = new TH2F("thresholdScan2D","Threshold Scan",256, -0.5, 255.5, 128,  -0.5, 127.5);
    const char *warning_labels[3] = {"Flag raised", "No channels fired", "Excessive channels fired"};
    const char *error_labels[1] = {"CRC mismatch"};
    Warnings = new TH1I("Warnings", "Warnings", 3,  0, 3);
    for (int i = 1; i<4; i++) Warnings->GetXaxis()->SetBinLabel(i, warning_labels[i-1]);
    Errors   = new TH1I("Errors", "Critical errors", 1,  0, 1);
    for (int i = 1; i<2; i++) Errors->GetXaxis()->SetBinLabel(i, error_labels[i-1]);

    TDirectory * scandir = gDirectory->mkdir("Threshold_Scans");
    scandir->cd();
    for (int i = 0; i < 128; i++){
      thresholdScan[i] = new TH1F(("thresholdScan"+to_string(static_cast<long long int>(i))).c_str(),("thresholdScan"+to_string(static_cast<long long int>(i))).c_str(),256,0,256);
    }// end loop on channels
    //get maps
    m_sn = std::stoi(m_HWID);
    readMapFromFile(m_sn,m_strip_map);
    gDirectory->cd("..");
  }

  //!Fills histograms for VFAT data
  /*!
    This fills histograms for the following data: Difference between crc and recalculated crc, Control Bit 1010, Control Bit 1100, Control Bit 1110, Bunch Crossing Number, Event Counter, Control Flags, and Chip ID, and Fired Channels
  */
  void fillHistograms(VFATdata * vfat, long long int orbitNumber){
    setVFATBlockWords(vfat);
    int crc_diff = vfat->crc()-checkCRC(vfatBlockWords);
    if (crc_diff != 0) crc_difference->Fill(crc_diff);
    //b1010->Fill(vfat->b1010());
    //b1100->Fill(vfat->b1100());
    //b1110->Fill(vfat->b1110());
    BC->Fill(vfat->BC());
    EC->Fill(vfat->EC());
    //Flag->Fill(vfat->Flag());
    Header->Fill(vfat->Header());
    //ChipID->Fill(vfat->ChipID());
    m_sn = std::stoi(m_HWID);
    crc->Fill(vfat->crc());
    if (vfat->CRRCcheck()) {
      Errors->Fill(0);
    }
    SlotN->Fill(m_sn);
    uint16_t chan0xf = 0;
    int n_hits_fired = 0;
    for (int chan = 0; chan < 128; ++chan) {
      if (chan < 64){
	chan0xf = ((vfat->lsData() >> chan) & 0x1);
	if(chan0xf) {
          n_hits_fired++;
	  FiredChannels->Fill(chan);
	  FiredStrips->Fill(m_strip_map[chan]);
	}
      } else {
	chan0xf = ((vfat->msData() >> (chan-64)) & 0x1);
	if(chan0xf) {
          n_hits_fired++;
	  FiredChannels->Fill(chan);
	  FiredStrips->Fill(m_strip_map[chan]);
	}
      }
    }
    n_hits_per_event->Fill(n_hits_fired);

  }
  void fillWarnings(){
    if (FiredChannels->GetEntries() == 0) {
      Warnings->Fill(1);
    }
    else if (FiredChannels->GetEntries() > 64*b1010->GetEntries()) {
      Warnings->Fill(2);
    }
  }
  //!Fills the histograms for the Threshold Scans
  void fillScanHistograms(VFATdata * vfat, int runtype, int deltaV, int latency){
    bool channelFired = false;
    int n_h_fired = 0;
    for (int i = 0; i < 128; i++){
      uint16_t chan0xf = 0;
      if (i < 64){
	chan0xf = ((vfat->lsData() >> i) & 0x1);
	if(chan0xf) {
	  thresholdScan[i]->Fill(deltaV);
	  thresholdScanChip2D->Fill(deltaV,i);
	  //latencyScan2D->Fill(latency,i);
	  channelFired = true;
          n_h_fired++;
	}
      } else {
	chan0xf = ((vfat->msData() >> (i-64)) & 0x1);
	if(chan0xf) {
	  thresholdScan[i]->Fill(deltaV);
	  thresholdScanChip2D->Fill(deltaV,i);
	  //latencyScan2D->Fill(latency,i);
	  channelFired = true;
          n_h_fired++;
	}
      }
    }// end loop on channels
    if (channelFired) {
      latencyScan->Fill(latency);
      latencyBXdiffScan->Fill(vfat->BC()-latency);
      latencyScanBX2D->Fill(latency, vfat->BC());
      thresholdScanChip->Fill(deltaV);
      if (n_h_fired > 100) {latencyScanBX2D_extraHighOcc->Fill(latency, vfat->BC());}
    }
  }

  TH1F* getb1010() { return b1010; }
  TH1F* getb1100() { return b1100; }
  TH1F* getb1110() { return b1110; }
  TH1F* getFlag()  { return Flag; }
  TH1F* getCRC()   { return crc_difference; }

  int * getMap(){ return m_strip_map;}



private:
  TH1F* n_hits_per_event;
  //TH1F* b1010;            ///<Histogram for control bit 1010
  TH1D* BC;               ///<Histogram for Bunch Crossing Number
  //TH1F* b1100;            ///<Histogram for control bit 1100
  TH1F* EC;               ///<Histogram for Event Counter
  //TH1F* Flag;             ///<Histogram for Control Flags
  TH1F* Header;             ///<Histogram for Control Flags
  //TH1F* b1110;            ///<Histogram for contorl bit 1110
  //TH1F* ChipID;           ///<Histogram for Chip ID
  TH1F* FiredChannels;    ///<Histogram for Fired Channels (uses lsData and fmData)
  TH1F* crc_difference;   ///<Histogram for difference of crc and recalculated crc
  TH1F* SlotN;
  TH1F* FiredStrips;
  TH1F* crc;
  TH1F* crc_calc;
  TH1D* latencyScan;
  TH1D* latencyBXdiffScan;
  //TH2F* latencyScan2D;
  TH2D* latencyScanBX2D;
  TH2F* latencyScanBX2D_extraHighOcc;
  TH1F* thresholdScanChip;
  TH2F* thresholdScanChip2D;
  TH1F* thresholdScan[NCHANNELS];
  TH1I* Warnings;
  TH1I* Errors;
  int m_sn;
  int m_strip_map[128];

  uint16_t vfatBlockWords[12];   ///<Array of uint16_t used for setVFATBlockWords

  //!This puts the VFAT data in an array of uint16_t to be used for the crc check
  void setVFATBlockWords(VFATdata * vfat_)
  {
    vfatBlockWords[11] = ((0x000f & vfat_->b1010())<<12) | vfat_->BC();
    vfatBlockWords[10] = ((0x000f & vfat_->b1100())<<12) | ((0x00ff & vfat_->EC()) <<4) | (0x000f & vfat_->Flag());
    vfatBlockWords[9]  = ((0x000f & vfat_->b1110())<<12) | vfat_->ChipID();
    vfatBlockWords[8]  = (0xffff000000000000 & vfat_->msData()) >> 48;
    vfatBlockWords[7]  = (0x0000ffff00000000 & vfat_->msData()) >> 32;
    vfatBlockWords[6]  = (0x00000000ffff0000 & vfat_->msData()) >> 16;
    vfatBlockWords[5]  = (0x000000000000ffff & vfat_->msData());
    vfatBlockWords[4]  = (0xffff000000000000 & vfat_->lsData()) >> 48;
    vfatBlockWords[3]  = (0x0000ffff00000000 & vfat_->lsData()) >> 32;
    vfatBlockWords[2]  = (0x00000000ffff0000 & vfat_->lsData()) >> 16;
    vfatBlockWords[1]  = (0x000000000000ffff & vfat_->lsData());
  }

  //!Recalculates the CRC to be compared to original CRC. Difference should be 0.
  uint16_t checkCRC(uint16_t dataVFAT[11])
  {
    uint16_t crc_fin = 0xffff;
    for (int i = 11; i >= 1; i--)
      {
	crc_fin = this->crc_cal(crc_fin, dataVFAT[i]);
      }
    return(crc_fin);
  }
  //!Called by checkCRC
  uint16_t crc_cal(uint16_t crc_in, uint16_t dato)
  {
    uint16_t v = 0x0001;
    uint16_t mask = 0x0001;
    bool d=0;
    uint16_t crc_temp = crc_in;
    unsigned char datalen = 16;

    for (int i=0; i<datalen; i++){
      if (dato & v) d = 1;
      else d = 0;
      if ((crc_temp & mask)^d) crc_temp = crc_temp>>1 ^ 0x8408;
      else crc_temp = crc_temp>>1;
      v<<=1;
    }
    return(crc_temp);
  }
};
