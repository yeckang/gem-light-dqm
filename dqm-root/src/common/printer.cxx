
#define DEBUG 0


#include <sstream>
#include <iostream>
#include <fstream>
#include <iomanip>

#if !defined(__CINT__) || defined(__MAKECINT__)
#include "TProfile.h"
#include "TLegend.h"
#include "TROOT.h"
#include "TVirtualPad.h"
#include "TLine.h"
#include "TCanvas.h"
#include "TPostScript.h"
#include "THStack.h"
#include "TH1.h"
#include "TH2.h"
#include "TF1.h"
#include "TAxis.h"
#include "TGaxis.h"
#include "TMath.h"
#include "TROOT.h"
#include "TStyle.h"
#include "TGraph.h"
#include "TGraphAsymmErrors.h"
#include "TObject.h"
#include "TH1.h"
#include "TH1F.h"
#include <TFile.h>
#include "TPaveStats.h"
#include <math.h>
#include <TBufferJSON.h>
#include <iostream>

using namespace std;

#endif



void gtprintCanvas(TCanvas* canvas, TString opathName)
{

  // canvas->Print(opathName+".jpg","jpg");
  // canvas->Print(opathName+".png","png");

  //Create JSON file
  ofstream jsonfile;
  jsonfile.open(opathName+".json");
  TString json = TBufferJSON::ConvertToJSON(canvas);
  jsonfile << json;
  jsonfile.close();

}


void gtprint(TH1 *h, TString name, TString opath)
{
  TCanvas *cv = newCanvas(name);			
  cv->cd(1);
  int max=h->GetBinContent(h->GetMaximumBin());
  h->SetMaximum(max);
  h->SetMinimum(0);

  //Focus on Valid ChipIDs
  if (strcmp(h->GetName(),"ChipID")==0)
    h->GetXaxis()->SetRangeUser(3700,3800);

  //Set color/style
  h->SetMarkerColor(kBlue+3);
  h->SetMarkerStyle(1);
  h->SetFillStyle(1001);
  h->SetFillColor(kBlue+3);
  h->SetLineColor(kBlue+3);

  gStyle->SetOptStat(111111);

  h->Draw();

  //Add Legend
  TLegend* leg = new TLegend(0.78,0.4,0.98,0.45);
  leg->AddEntry(h,name,"l");
  leg->Draw();

  gROOT->ProcessLine(".!mkdir -p "+opath+"/");

  gtprintCanvas(cv, opath+name);
  
}

void gemTreePrint(TDirectory *source, TString outPath, bool first)
{

  //Create equivalent output directory via newPath (ignore initial .root directory)
  TString newPath;
  if(!first){
    newPath = outPath + source->GetName() + "/";      
    if(DEBUG) std::cout<<"[gemTreePrint]"<< "newPath: " << newPath << std::endl;
    gROOT->ProcessLine(".!mkdir -p "+newPath);
  }
  else newPath = outPath;

  Bool_t status = TH1::AddDirectoryStatus();
  TH1::AddDirectory(kFALSE);

  
  //Check for directories, print histograms
  TIter nextkey(source->GetListOfKeys());
  TKey *key = new TKey;
  int key_c = 1; //counter
  while (key = (TKey*)nextkey())
    {
      if(DEBUG) std::cout<< std::endl;
      if(DEBUG) std::cout<<"[gemTreePrint]"<< "Key: " << key_c << std::endl;
      if(DEBUG) std::cout<<"[gemTreePrint]"<< "Key Name: " << key->GetName() << std::endl;
      if(DEBUG) std::cout<<"[gemTreePrint]"<< "Key Class: " << key->GetClassName() << std::endl;
      key_c++;
      // TClass *cl = gROOT->GetClass(key->GetClassName());
      TObject *obj = key->ReadObj();
      //Recursively loop through directories
      string keyName = key->GetName();
      // if ((cl->InheritsFrom(TDirectory::Class())) and (keyName.find("OnlineHists")==string::npos)) {
      if (obj->IsA()->InheritsFrom(TDirectory::Class()) and (keyName.find("OnlineHists")==string::npos)) {
        if(DEBUG) std::cout<<"[gemTreePrint]"<< "Moving to directory: "<<keyName<<endl;
        source->cd(key->GetName());
        TDirectory *subdir = gDirectory;
        gemTreePrint(subdir, newPath, false);
      }
      //Print if key is a histogram
      // if (cl->InheritsFrom("TH1")) {
      if (obj->IsA()->InheritsFrom(TH1::Class())) {
	if(DEBUG) std::cout<<"[gemTreePrint]"<< "Printing histogram... " << std::endl;
	TH1 *h = (TH1*)key->ReadObj();
	gtprint(h,key->GetName(),newPath);
      }
      //Print summary canvases
      // if (cl->InheritsFrom("TCanvas")) {
      if (obj->IsA()->InheritsFrom(TCanvas::Class())) {
	if(DEBUG) std::cout<<"[gemTreePrint]"<< "Printing canvas... " << std::endl;
        gROOT->ProcessLine(".!mkdir -p "+newPath+"summary_canvases/");
	TString fullPath = newPath + "summary_canvases/" + key->GetName();
	TCanvas *c = (TCanvas*)key->ReadObj();
	gtprintCanvas(c,fullPath);
      }
     
    }
  TH1::AddDirectory(status);
  return;
}


void gemTreePrintOnline(TDirectory *source, TString outPath, bool first)
{

  if (gDirectory->cd("/OnlineHists")) {
    TList* keylist = new TList;
    TIter nextkey(gDirectory->GetListOfKeys());
    TKey *key = new TKey;
    int key_c = 1; //counter
    while (key = (TKey*)nextkey())
      {
        // Process name to get directory destination
        string chop = key->GetName();
        bool done = false;
        int vfat = -1;
        int geb = -1;
        int amc = -1;
        string histName = chop.substr(chop.find_last_of("_")+1);
        chop = chop.substr(0,chop.find_last_of("_"));
        while (!done) {
          int found = chop.find_last_of("_");
          if (found == string::npos)
            done = true;
          string hw = chop.substr(found+1);
          chop = chop.substr(0,found);
          if (hw.find("AMC") != string::npos)
            amc = stoi(hw.substr(4));
          else if ((hw.find("GTX") != string::npos) or (hw.find("GEB") != string::npos))
            geb = stoi(hw.substr(4));
          else if (hw.find("VFAT") != string::npos)
            vfat = stoi(hw.substr(5));
          else
            done = true;
        }
        TString newPath = outPath + "AMC13-1";
        if (amc > -1) {
          newPath = newPath + "/AMC-" + to_string((long long int)amc);
          if (geb > -1) {
            newPath = newPath + "/GTX-" + to_string((long long int)geb); 
            if (vfat > -1)
              newPath = newPath + "/VFAT-" + to_string((long long int)vfat);
          }
        }

        TClass *cl = gROOT->GetClass(key->GetClassName());
        //Print if key is a histogram
        if (cl->InheritsFrom("TH1")) {
          if(DEBUG) std::cout<<"[gemTreePrint]"<< "Printing histogram... " << std::endl;
          gROOT->ProcessLine(".!mkdir -p "+newPath);
          TH1 *h = (TH1*)key->ReadObj();
          gtprint(h,histName,newPath);
        }
        //Print summary canvases
        if (cl->InheritsFrom("TCanvas")) {
          if(DEBUG) std::cout<<"[gemTreePrint]"<< "Printing canvas... " << std::endl;
          gROOT->ProcessLine(".!mkdir -p "+newPath+"/summary_canvases/");
          TString fullPath = newPath + "/summary_canvases/" + histName;
          TCanvas *c = (TCanvas*)key->ReadObj();
          gtprintCanvas(c,fullPath);
        }
      }
  
  }
  else {
    cout << "Could not find online histograms directory!" << endl;
    return;
  }
  
  return;
 }


// void printSummaryCanvases(TDirectory *onlineDir, int amcNumber, int gebNumber) {

//   TString newPath = onlineDir->GetPath() + "/AMC13-1";
//   newPath = newPath + "/AMC-" + to_string((long long int)amcNumber);
//   newPath = newPath + "/GTX-" + to_string((long long int)gebNumber); 
//   newPath = newPath + "/summary_canvases/";
//   cout << "Path to print: " << newPath << endl;

//   string integrityName = "AMC-"+to_string((long long int)amcnum)+"_GTX-"+to_string((long long int)gebnum)+"_integrity";
//   string occupancyName = "AMC-"+to_string((long long int)amcnum)+"_GTX-"+to_string((long long int)gebnum)+"_occupancy";
//   string clusterSizeName = "AMC-"+to_string((long long int)amcnum)+"_GTX-"+to_string((long long int)gebnum)+"_clusterSize";
//   string clusterMultName = "AMC-"+to_string((long long int)amcnum)+"_GTX-"+to_string((long long int)gebnum)+"_clusterMult";


// }
