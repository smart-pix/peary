
#include <fstream>
#include <sstream>
#include "dSiPMFrameDecoder.hpp"
#include "utils/log.hpp"

#include <TTree.h>

int main(int argc, char const* argv[]) {

  bool PrintEndFrame = false;
  bool read2bit = false;
  bool needHelp = false;

  for(int i = 1; i < argc; ++i) {

    if(!strcmp(argv[i], "-pe"))
      PrintEndFrame = atoi(argv[++i]);

    if(!strcmp(argv[i], "-r2"))
      read2bit = atoi(argv[++i]);

    if(!strcmp(argv[i], "-h"))
      needHelp = true;

  } // argc

  if(argc == 1 || needHelp) {
    std::cout << "Use like:" << std::endl
              << "./dSiPMBinaryDecoder filename.bin" << std::endl
              << "Options:" << std::endl
              << "-r2 1 -- switching 2bit mode on" << std::endl
              << "-pe 1 -- print end of frame marker" << std::endl
              << "-h    -- print this help" << std::endl
              << "Filename need to be given after options." << std::endl;
    return 0;
  }

  // need to read more if read2bit
  unsigned int read_n_times = 1;
  if(read2bit) {
    read_n_times = 2;
  }

  std::string s_filename = argv[argc - 1];
  std::string name = s_filename.substr(0, s_filename.size() - 4); // strip .bin
  std::string ext = ".txt";
  name += ext;

  std::ifstream data_in;
  data_in.open(argv[argc - 1], std::ios::ate | std::ios::binary);

  if(data_in) {

    // get length of file:
    data_in.seekg(0, data_in.end);
    auto length = data_in.tellg();
    data_in.seekg(0, data_in.beg);

    std::cout << "Reading " << length << " bytes " << std::endl;

    uint32_t indata[read_n_times * (DSIPM_N_HITDATA + DSIPM_N_TIMEDATA + DSIPM_N_DAQDATA)];
    std::streamsize framesize = read_n_times * (DSIPM_N_HITDATA + DSIPM_N_TIMEDATA + DSIPM_N_DAQDATA) * 4;
    static caribou::dSiPMFrameDecoder decoder;

    while(data_in.read(reinterpret_cast<char*>(&indata), framesize)) {
      caribou::pearyRawData rawdata;
      for(unsigned int j = 0; j < read_n_times * (DSIPM_N_HITDATA + DSIPM_N_TIMEDATA + DSIPM_N_DAQDATA); j++) {
        rawdata.push_back(indata[j]);
      }
      auto frame = decoder.decodeFrame(rawdata, 1, read2bit);
      decoder.printFrame(frame, name.c_str(), PrintEndFrame);
      // Printing also frame ID from FPGA ts
      // decoder.printFrame(rawdata, name.c_str(), PrintEndFrame, 1);
    }

    std::cout << length / (read_n_times * (DSIPM_N_HITDATA + DSIPM_N_TIMEDATA + DSIPM_N_DAQDATA) * 4) << " frames decoded"
              << std::endl;

  } else {
    std::cout << "Failed to open " << argv[argc - 1] << std::endl;
  }
  return 0;
}
