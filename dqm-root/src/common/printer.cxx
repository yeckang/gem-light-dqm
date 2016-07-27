#ifndef DEBUG
#define DEBUG 1
#endif

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

  //Retrieve histograms from current directory
  vector<TH1*> hs;
  retrieveHistograms(source, hs);
  int numH = hs.size();
  if(DEBUG) std::cout<<"[gemTreePrint]"<< "In directory: " << source->GetName() << std::endl;
  if(DEBUG) std::cout<<"[gemTreePrint]"<< "Number of histrograms retrieved: " << numH << std::endl;

  //Check for directories, print histograms
  TList* keylist = new TList;
  keylist = source->GetListOfKeys();
  TIter nextkey(keylist);
  TKey *key = new TKey;
  int key_c = 1; //counter
  while (key = (TKey*)nextkey())
    {
      if(DEBUG) std::cout<< std::endl;
      if(DEBUG) std::cout<<"[gemTreePrint]"<< "Key: " << key_c << std::endl;
      if(DEBUG) std::cout<<"[gemTreePrint]"<< "Key Name: " << key->GetName() << std::endl;
      if(DEBUG) std::cout<<"[gemTreePrint]"<< "Key Class: " << key->GetClassName() << std::endl;
      key_c++;
      TClass *cl = gROOT->GetClass(key->GetClassName());
      
      //Recursively loop through directories
      if (cl->InheritsFrom(TDirectory::Class())) {
        source->cd(key->GetName());
        TDirectory *subdir = gDirectory;
        gemTreePrint(subdir, newPath, false);
      }
      //Print if key is a histogram
      if (cl->InheritsFrom("TH1")) {
	if(DEBUG) std::cout<<"[gemTreePrint]"<< "Printing histogram... " << std::endl;
	TH1 *h = (TH1*)key->ReadObj();
	gtprint(h,key->GetName(),newPath);
      }
      //Print summary canvases
      if (cl->InheritsFrom("TCanvas")) {
	if(DEBUG) std::cout<<"[gemTreePrint]"<< "Printing canvas... " << std::endl;
        gROOT->ProcessLine(".!mkdir -p "+newPath+"summary_canvases/");
	TString fullPath = newPath + "summary_canvases/" + key->GetName();
	TCanvas *c = (TCanvas*)key->ReadObj();
	gtprintCanvas(c,fullPath);
      }
     
    }
  return;
}

