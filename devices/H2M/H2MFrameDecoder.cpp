#include "H2MFrameDecoder.hpp"

using namespace caribou;

void H2MFrameDecoder::printFrame(const pearydata& frame, const char* fname, bool PrintEndFrame) {

  // Open and print preamble
  m_outfileFrames.open(fname, std::ios_base::app); // append
  if(m_outfileFrames.is_open()) {

    LOG(INFO) << "Printing pixel to " << fname;
    LOG(INFO) << "col \trow \tdata \tmode";
    m_outfileFrames << "col \trow \tdata \tmode" << std::endl;
  } else {

    LOG(ERROR) << "Failed to open " << fname;
    return;
  }

  // Print to LOG(INFO) and file
  for(const auto& pixel_ptr : frame) {
    auto pixel = dynamic_cast<caribou::h2m_pixel_readout*>(pixel_ptr.second.get());
    LOG(INFO) << "\t" << pixel_ptr.first.first << "\t" << pixel_ptr.first.second << "\t"
              << static_cast<int>(pixel->GetData()) << "\t" << static_cast<int>(pixel->GetMode());
    m_outfileFrames << pixel_ptr.first.first << "  " << pixel_ptr.first.second << "  " << static_cast<int>(pixel->GetData())
                    << "  " << static_cast<int>(pixel->GetMode()) << std::endl;
  }
  // Add end of frame -1 flag
  if(PrintEndFrame) {
    m_outfileFrames << -1 << "  " << -1 << "  " << -1 << "  " << -1 << std::endl;
  }

  m_outfileFrames.close();
}
