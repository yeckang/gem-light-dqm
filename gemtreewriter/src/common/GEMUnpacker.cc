#include <cstdio>
#include <iostream>
#include <cstdint>
#include <vector>
#include <array>
#include <bitset>
//#include "GEMAMC13EventFormat.h"
#if defined(__CINT__) && !defined(__MAKECINT__)
    #include "libEvent.so"
#else
    #include "Event.h"
#endif
#include <TFile.h>
#include <TTree.h>

class GEMUnpacker
{
  public:
    GEMUnpacker(const std::string & ifilename, const std::string & isFedKit);
    ~GEMUnpacker()
    void unpack();
  private:
    std::FILE *m_file;
    uint64_t* m_word;
    uint64_t* begin;
    uint64_t* end;
    GEMRawToDigi rawToDigi;
    AMC13Event * m_AMC13Event;
    std::string ofilename;
    std::string m_isFedKit;
};
GEMUnpacker::GEMUnpacker(const std::string & ifilename, const std::string & isFedKit)
{
  try {
    m_file = std::fopen(ifilename.c_str(), "rb");
    vector<uint64_t> testWord;
    uint64_t temp_word;
    while (true){
      std::size_t sz = std::fread(&temp_word, sizeof(uint64_t), 1, m_file);
      if (sz == 0 ) break;
      else testWord.push_back(temp_word);
    }
    uint64_t *word = new uint64_t[testWord.size()];
    copy(testWord.begin(), testWord.end(), word);
    begin = word;
    end = word + testWord.size();
  }
  catch (int e)
  {
    std::cout << "An exception occured. Exception code " << e << std::endl;
  }
  ofilename = ifilename.substr(0, ifilename.size()-4);
  ofilename += ".raw.root";
  m_isFedKit = isFedKit;
}

GEMUnpacker::~GEMUnpacker()
{
  if (m_file != NULL) std::fclose(m_file);
}
 
void unpack()
{
  TFile *hfile = new TFile(ofilename.c_str(),"RECREATE","GEM Raw ROOT");
  TTree GEMtree("GEMtree","A Tree with GEM Events");
  Event *ev = new Event(); 
  GEMtree.Branch("GEMEvents", &ev);

  while(word != end) {
    if (m_isFedKit == "ferol") word+=3;
    auto amc13Event = rawToDigi.convertWordToAMC13Event(word);
    
    if (amc13Event == NULL) {
      cout << "RAW FILE have error!!" << endl;
      return 1;
    }

    for (auto amcHeader : *(amc13Event->getAMCheaders())) {
      cout << "amcSize :: " << ((gem::AMCHeader{amcHeader}.amcSize)) << endl;
    }
    cout << "amc13Event length :: " << gem::CDFTrailer{amc13Event->getCDFTrailer()}.evtLength << endl;
    word+= gem::CDFTrailer{amc13Event->getCDFTrailer()}.evtLength;
    ev->Build(true);
    ev->SetHeader(m_AMC13Event->LV1_id(), 0, 0);
    ev->addAMC13Event(*m_AMC13Event);
    GEMtree.Fill();
    ev->Clear();
  }
  hfile->Write();// Save file with tree
}
int main (int argc, char** argv)
{
  std::cout << "[GEMUnpacker]: ---> Main()" << std::endl;
  if (argc<3) 
  {
    std::cout << "Please provide input filename and source type" << std::endl;
    std::cout << "Usage: <path>/unpacker ifile ferol(sdram)" << std::endl;
    return 0;
  }
  std::string ifile   = argv[1];
  std::string isFedKit = argv[2];
  GEMUnpacker * m_unpacker = new GEMUnpacker(ifile, isFedKit);
  m_unpacker->unpack();
  delete m_unpacker;
}
