// #include <sstream>
// #include <iostream>
// #include <fstream>
// #include <iomanip>

// #if !defined(__CINT__) || defined(__MAKECINT__)
// #include "TProfile.h"
// #include "TLegend.h"
// #include "TROOT.h"
// #include "TVirtualPad.h"
// #include "TLine.h"
// #include "TCanvas.h"
// #include "TPostScript.h"
// #include "THStack.h"
// #include "TH1.h"
// #include "TH2.h"
// #include "TF1.h"
// #include "TAxis.h"
// #include "TGaxis.h"
// #include "TMath.h"
// #include "TROOT.h"
// #include "TStyle.h"
// #include "TGraph.h"
// #include "TGraphAsymmErrors.h"
// #include "TObject.h"
// #include "TH1.h"
// #include "TH1F.h"
// #include <TFile.h>
// #include "TPaveStats.h"
// #include <math.h>
// #include "TBufferJSON.h"

// #include <iostream>

// //#include "plotter.cxx"
// //#include "gtprinter.cxx"
// //using namespace std;

// #endif


void fillGEBCanvases()
{
  gROOT->SetBatch(kTRUE);
  fillGEBIntegrityCanvas();
  fillGEBOccupancyCanvas();
  fillGEBClusterSizeCanvas();
  fillGEBClusterMultCanvas();
}

void fillGEBIntegrityCanvas()
{
  TGaxis::SetMaxDigits(3);
  gStyle->SetOptStat(0000);

  Integrity_canvas->cd(1);
  Totalb1010->Draw();
  //gPad->SetLogy();
  Integrity_canvas->cd(2);
  Totalb1100->Draw();
  
  Integrity_canvas->cd(3);
  Totalb1110->Draw();

  Integrity_canvas->cd(4);
  SlotN->Draw();
  
  Integrity_canvas->cd(5);
  TotalFlag->Draw();

  Integrity_canvas->cd(6);
  TotalCRC->Draw();

  // ->Update() ?
}


void fillGEBOccupancyCanvas()
{
  Occupancy_canvas->cd(1);
  std::stringstream ss;
  for (int nb = 1; nb < 9; nb ++)
  {
    std::string name = "eta_";
    ss.str(std::string());
    ss << 9-nb;
    name+=ss.str();
    BeamProfile->GetXaxis()->SetBinLabel(nb, name.c_str());
  }
  BeamProfile->GetYaxis()->SetTitle("Strips");
  BeamProfile->GetXaxis()->SetTitle("Pseudorapidity partitions");
  BeamProfile->Draw("colz");

  TH1D* p_temp;
  for (int p_i = 1; p_i < 9; p_i++)
  {
    Occupancy_canvas->cd(p_i+1);
    std::string title = "eta_";
    ss.str(std::string());
    ss << 9-p_i;
    title+=ss.str();
    p_temp = BeamProfile->ProjectionY(title.c_str(),p_i,p_i);
    p_temp->SetTitle(title.c_str());
    p_temp->GetYaxis()->SetTitle("Number of events");
    p_temp->Draw();
    gPad->Update();
 
  }

  //Occupancy_canvas->Draw();
  //Occupancy_canvas->Update();
  //Occupancy_canvas->WaitPrimitive();
}


void fillGEBClusterSizeCanvas()
{
  gStyle->SetOptStat("emr");
  std::stringstream ss;
  ClusterSize_canvas->cd(1);
  ClusterSize->GetYaxis()->SetTitle("Number of entries");
  ClusterSize->GetXaxis()->SetTitle("Cluster size");
  ClusterSize->SetTitle("Integrated over pseudorapidity");
  ClusterSize->Draw();
  gPad->SetLogy();
  for (int p_i = 1; p_i < NETA+1; p_i++)
  {
    ClusterSize_canvas->cd(p_i+1);
    std::string title = "eta_";
    ss.str(std::string());
    ss << 9-p_i;
    title+=ss.str();
    ClusterSizeEta[NETA-p_i]->GetYaxis()->SetTitle("Number of entries");
    ClusterSizeEta[NETA-p_i]->GetXaxis()->SetTitle("Cluster size");
    ClusterSizeEta[NETA-p_i]->SetTitle(title.c_str());
    ClusterSizeEta[NETA-p_i]->Draw();
    gPad->SetLogy();
  }
 
  //clusterSize_canvas->Draw();
  //clusterSize_canvas->Update();
  //clusterSize_canvas->WaitPrimitive();
  gStyle->SetOptStat(0000);
}

void fillGEBClusterMultCanvas()
{
  gStyle->SetOptStat("emr");
  std::stringstream ss;
  ClusterMult_canvas->cd(1);
  ClusterMult->GetYaxis()->SetTitle("Number of entries");
  ClusterMult->GetXaxis()->SetTitle("Cluster multiplicity");
  ClusterMult->SetTitle("Integrated over pseudorapidity");
  ClusterMult->Draw();
  gPad->SetLogy();
  for (int p_i = 1; p_i < NETA+1; p_i++)
  {
    ClusterMult_canvas->cd(p_i+1);
    std::string title = "eta_";
    ss.str(std::string());
    ss << 9-p_i;
    title+=ss.str();
    ClusterMultEta[NETA-p_i]->GetYaxis()->SetTitle("Number of entries");
    ClusterMultEta[NETA-p_i]->GetXaxis()->SetTitle("Cluster multiplicity");
    ClusterMultEta[NETA-p_i]->SetTitle(title.c_str());
    ClusterMultEta[NETA-p_i]->Draw();
    gPad->SetLogy();
  }
 
  //clusterMult->Draw();
  //clusterMult->Update();
  //clusterMult->WaitPrimitive();
  gStyle->SetOptStat(0000);
}
