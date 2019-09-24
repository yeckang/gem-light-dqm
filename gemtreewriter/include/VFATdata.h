#ifndef EventFilter_GEMRawToDigi_VFATdata_h
#define EventFilter_GEMRawToDigi_VFATdata_h

#include <cstdint>

namespace gem {
  /// VFAT data structure - 3 words of 64 bits each
  union VFATfirst {
    uint64_t word;
    // v3 dataformat
    struct {
      uint64_t msData1 : 16;  ///<channels from 65to128
      uint64_t bc : 16;       ///<Bunch Crossing number, 16 bits
      uint64_t ec : 8;        ///<Event Counter, 8 bits
      uint64_t header : 8;    ///<normally 0x1E. 0x5E indicates that the VFAT3 internal buffer
      // is half-full, so it's like a warning
      uint64_t vc : 1;      /// VFAT CRC Error
      uint64_t unused : 7;  ///<bits 183:177 are not used, should be 0, bit 176 is 1 if CTP7 detected a CRC mismatch
      uint64_t pos : 8;     ///<an 8bit value indicating the VFAT position on this GEB (it can be 0 to 23)
    };
    // v2 dataformat
    struct {
      uint64_t msData1v2 : 16;  ///<channels from 65to128 - placeholder since msData1 reads same info
      uint64_t chipID : 12;     ///<Chip ID, 12 bits
      uint64_t b1110 : 4;       ///<1110:4 Control bits, shoud be 1110
      uint64_t flag : 4;        ///<Control Flags: 4 bits, Hamming Error/AFULL/SEUlogic/SUEI2C
      uint64_t ecV2 : 8;        ///<Event Counter, 8 bits
      uint64_t b1100 : 4;       ///<1100:4, Control bits, shoud be 1100
      uint64_t bcV2 : 12;       ///<Bunch Crossing number, 12 bits
      uint64_t b1010 : 4;       ///<1010:4 Control bits, shoud be 1010
    };
  };
  union VFATsecond {
    uint64_t word;
    struct {
      uint64_t lsData1 : 16;  ///<channels from 1to64
      uint64_t msData2 : 48;  ///<channels from 65to128
    };
  };
  union VFATthird {
    uint64_t word;
    struct {
      uint64_t crc : 16;      ///<Check Sum value, 16 bits
      uint64_t lsData2 : 48;  ///<channels from 1to64
    };
  };

  class VFATdata {
  public:
    VFATdata();
    // this constructor only used for packing sim digis
    VFATdata(const int vfatVer,
             const uint16_t BC,
             const uint8_t EC,
             const uint16_t chipID,
             const uint64_t lsDatas,
             const uint64_t msDatas);
    ~VFATdata() {}

    //!Read first word from the block.
    void read_fw(uint64_t word) { fw_ = word; }
    uint64_t get_fw() const { return fw_; }

    //!Read second word from the block.
    void read_sw(uint64_t word) { sw_ = word; }
    uint64_t get_sw() const { return sw_; }

    //!Read third word from the block.
    void read_tw(uint64_t word) { tw_ = word; }
    uint64_t get_tw() const { return tw_; }

    // local phi in chamber
    void setPhi(int i) { phiPos_ = i; }
    int phi() const { return phiPos_; }

    uint64_t lsData() const { return uint64_t(VFATsecond{sw_}.lsData1) << 48 | VFATthird{tw_}.lsData2; }
    uint64_t msData() const { return uint64_t(VFATfirst{fw_}.msData1) << 48 | VFATsecond{sw_}.msData2; }

    uint16_t bc() const {
      if (ver_ == 2)
        return VFATfirst{fw_}.bcV2;
      return VFATfirst{fw_}.bc;
    }
    uint8_t ec() const {
      if (ver_ == 2)
        return VFATfirst{fw_}.ecV2;
      return VFATfirst{fw_}.ec;
    }
    uint16_t vfatId() const {
      if (ver_ == 2)
        return VFATfirst{fw_}.chipID;
      return VFATfirst{fw_}.pos;
    }

    void setVersion(int i) { ver_ = i; }
    int version() const { return ver_; }

    /// quality flag - bit: 0 good, 1 crc fail, 2 b1010 fail, 3 b1100 fail, 4 b1110
    uint8_t quality();

    /// v3
    uint8_t header() const { return VFATfirst{fw_}.header; }
    uint8_t crcCheck() const { return VFATfirst{fw_}.vc; }
    uint8_t position() const { return VFATfirst{fw_}.pos; }

    /// v2
    uint8_t b1010() const { return VFATfirst{fw_}.b1010; }
    uint8_t b1100() const { return VFATfirst{fw_}.b1100; }
    uint8_t b1110() const { return VFATfirst{fw_}.b1110; }
    uint8_t flag() const { return VFATfirst{fw_}.flag; }
    uint16_t chipID() const { return VFATfirst{fw_}.chipID; }
    uint16_t crc() const { return VFATthird{tw_}.crc; }

    uint16_t crc_cal(uint16_t crc_in, uint16_t dato);
    uint16_t checkCRC();

    static const int nChannels = 128;
    static const int sizeChipID = 12;

  private:
    int ver_;     /// vfat version
    int phiPos_;  /// phi position of vfat in chamber

    uint64_t fw_;  // VFAT first word
    uint64_t sw_;  // VFAT second word
    uint64_t tw_;  // VFAT third word
  };
}  // namespace gem

using namespace gem;

VFATdata::VFATdata() : ver_(0), phiPos_(0), fw_(0), sw_(0), tw_(0) {}

VFATdata::VFATdata(const int vfatVer,
                   const uint16_t BC,
                   const uint8_t EC,
                   const uint16_t chipID,
                   const uint64_t lsDatas,
                   const uint64_t msDatas) {
  // this constructor only used for packing sim digis
  VFATfirst fw{0};
  VFATsecond sw{0};
  VFATthird tw{0};

  fw.header = 0x1E;

  if (vfatVer == 3) {
    fw.bc = BC;
    fw.ec = EC;
    fw.pos = chipID;
  } else {
    fw.chipID = chipID;
    fw.b1110 = 14;
    fw.b1100 = 12;
    fw.b1010 = 10;
    fw.ecV2 = EC;
    fw.bcV2 = BC;
  }

  sw.lsData1 = lsDatas >> 48;
  tw.lsData2 = lsDatas & 0x0000ffffffffffff;

  fw.msData1 = msDatas >> 48;
  sw.msData2 = msDatas & 0x0000ffffffffffff;
  ver_ = vfatVer;

  fw_ = fw.word;
  sw_ = sw.word;
  tw_ = tw.word;
  // checkCRC only works after words are set
  // checkCRC not yet implemented for v3
  tw.crc = checkCRC();
  // once crc is found, save new third word
  tw_ = tw.word;
}

uint8_t VFATdata::quality() {
  uint8_t q = 0;
  if (ver_ == 2) {
    if (VFATthird{tw_}.crc != checkCRC())
      q = 1;
    if (VFATfirst{fw_}.b1010 != 10)
      q |= 1UL << 1;
    if (VFATfirst{fw_}.b1100 != 12)
      q |= 1UL << 2;
    if (VFATfirst{fw_}.b1110 != 14)
      q |= 1UL << 3;
  }
  // quality test not yet implemented in v3
  return q;
}

uint16_t VFATdata::crc_cal(uint16_t crc_in, uint16_t dato) {
  uint16_t v = 0x0001;
  uint16_t mask = 0x0001;
  uint16_t d = 0x0000;
  uint16_t crc_temp = crc_in;
  unsigned char datalen = 16;
  for (int i = 0; i < datalen; i++) {
    if (dato & v)
      d = 0x0001;
    else
      d = 0x0000;
    if ((crc_temp & mask) ^ d)
      crc_temp = crc_temp >> 1 ^ 0x8408;
    else
      crc_temp = crc_temp >> 1;
    v <<= 1;
  }
  return crc_temp;
}

uint16_t VFATdata::checkCRC() {
  uint16_t vfatBlockWords[12];
  vfatBlockWords[11] = ((0x000f & VFATfirst{fw_}.b1010) << 12) | VFATfirst{fw_}.bcV2;
  vfatBlockWords[10] =
      ((0x000f & VFATfirst{fw_}.b1100) << 12) | ((0x00ff & VFATfirst{fw_}.ecV2) << 4) | (0x000f & VFATfirst{fw_}.flag);
  vfatBlockWords[9] = ((0x000f & VFATfirst{fw_}.b1110) << 12) | VFATfirst{fw_}.chipID;
  vfatBlockWords[8] = (0xffff000000000000 & msData()) >> 48;
  vfatBlockWords[7] = (0x0000ffff00000000 & msData()) >> 32;
  vfatBlockWords[6] = (0x00000000ffff0000 & msData()) >> 16;
  vfatBlockWords[5] = (0x000000000000ffff & msData());
  vfatBlockWords[4] = (0xffff000000000000 & lsData()) >> 48;
  vfatBlockWords[3] = (0x0000ffff00000000 & lsData()) >> 32;
  vfatBlockWords[2] = (0x00000000ffff0000 & lsData()) >> 16;
  vfatBlockWords[1] = (0x000000000000ffff & lsData());

  uint16_t crc_fin = 0xffff;
  for (int i = 11; i >= 1; i--) {
    crc_fin = crc_cal(crc_fin, vfatBlockWords[i]);
  }
  return crc_fin;
}
#endif
