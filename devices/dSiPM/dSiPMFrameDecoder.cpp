#include "dSiPMFrameDecoder.hpp"
#include "utils/lfsr.hpp"

using namespace caribou;

// simplifies getIpix
const dSiPMFrameDecoder::index_type rowBy8 = DSIPM_N_QUADROW / 4; //  4
const dSiPMFrameDecoder::index_type rowBy2 = DSIPM_N_QUADROW;     // 16
const dSiPMFrameDecoder::index_type rowTm2 = DSIPM_N_QUADROW * 4; // 64

void dSiPMFrameDecoder::printFrame(const pearydata& frame, const char* fname, bool PrintEndFrame) {
  // Open and print preamble
  m_outfileFrames.open(fname, std::ios_base::app); // append
  if(m_outfileFrames.is_open()) {
    LOG(INFO) << "Printing pixel to " << fname;
    LOG(INFO) << "\tbc \tcol \trow \tval \tvb \tft \tct";
  } else {
    LOG(ERROR) << "Failed to open " << fname;
    return;
  }
  // Print to LIG(INFO) and file
  for(const auto& pixel : frame) {
    auto ds_pix = dynamic_cast<caribou::dsipm_pixel*>(pixel.second.get());
    if(ds_pix->getBit() != 0) { // zero supression
      LOG(INFO) << "\t" << ds_pix->getBunchCounter() << "\t" << pixel.first.first << "\t" << pixel.first.second << "\t"
                << static_cast<int>(ds_pix->getBit()) << "\t" << ds_pix->getValidBit() << "\t"
                << static_cast<int>(ds_pix->getFineTime()) << "\t" << static_cast<int>(ds_pix->getCoarseTime());
      m_outfileFrames << ds_pix->getBunchCounter() << "  " << pixel.first.first << "  " << pixel.first.second << "  "
                      << static_cast<int>(ds_pix->getBit()) << "  " << ds_pix->getValidBit() << "  "
                      << static_cast<int>(ds_pix->getFineTime()) << "  " << static_cast<int>(ds_pix->getCoarseTime())
                      << std::endl;
    }
  }
  // Add end of frame -1 flag
  if(PrintEndFrame) {
    m_outfileFrames << -1 << "  " << -1 << "  " << -1 << "  " << -1 << "  " << -1 << "  " << -1 << "  " << -1 << std::endl;
  }

  m_outfileFrames.close();
}

void dSiPMFrameDecoder::printFrame(const pearyRawData& rawFrame, const char* fname, bool PrintEndFrame, bool read2bit) {

  uint16_t frameBit_f1 = 0;
  uint16_t frameBit_f2 = 0;

  // decode frame
  auto frame = decodeFrame(rawFrame, 0, read2bit);
  if(read2bit) {
    auto tsFPGA_f1 = decodeTrailer(rawFrame, 0);
    auto tsFPGA_f2 = decodeTrailer(rawFrame, 1);

    frameBit_f1 = tsFPGA_f1.second_2bit;
    frameBit_f2 = tsFPGA_f2.second_2bit;
  }

  // Open and print preamble
  m_outfileFrames.open(fname, std::ios_base::app); // append
  if(m_outfileFrames.is_open()) {
    LOG(INFO) << "Printing pixel to " << fname;
    LOG(INFO) << "\tbc \tcol \trow \tval \tvb \tft \tct \tfi1 \tfi2";
  } else {
    LOG(ERROR) << "Failed to open " << fname;
    return;
  }
  // Print to LIG(INFO) and file
  for(const auto& pixel : frame) {
    auto ds_pix = dynamic_cast<caribou::dsipm_pixel*>(pixel.second.get());
    if(ds_pix->getBit() != 0) { // zero supression
      LOG(INFO) << "\t" << ds_pix->getBunchCounter() << "\t" << pixel.first.first << "\t" << pixel.first.second << "\t"
                << static_cast<int>(ds_pix->getBit()) << "\t" << ds_pix->getValidBit() << "\t"
                << static_cast<int>(ds_pix->getFineTime()) << "\t" << static_cast<int>(ds_pix->getCoarseTime()) << "\t"
                << frameBit_f1 << "\t" << frameBit_f2;
      m_outfileFrames << ds_pix->getBunchCounter() << "  " << pixel.first.first << "  " << pixel.first.second << "  "
                      << static_cast<int>(ds_pix->getBit()) << "  " << ds_pix->getValidBit() << "  "
                      << static_cast<int>(ds_pix->getFineTime()) << "  " << static_cast<int>(ds_pix->getCoarseTime()) << "  "
                      << frameBit_f1 << "  " << frameBit_f2 << std::endl;
    }
  }
  // Add end of frame -1 flag
  if(PrintEndFrame) {
    m_outfileFrames << -1 << "  " << -1 << "  " << -1 << "  " << -1 << "  " << -1 << "  " << -1 << "  " << -1 << std::endl;
  }

  m_outfileFrames.close();
}

void dSiPMFrameDecoder::generateMapping() {
  LOG(INFO) << "Generatring lokup tables for pixel mapping.";

  for(index_type i = 0; i < DSIPM_N_PIX; i++) {
    m_indexToCol[i] = getCol(getIpix(i), getQuad(i));
    m_indexToRow[i] = getRow(getIpix(i), getQuad(i));
    m_indexToQuad[i] = getQuad(i);
    m_remap2bit[i] = getRemap2bit(i);
  }

  return;
}

dSiPMFrameDecoder::index_type dSiPMFrameDecoder::getIpix(index_type itot) {
  // Reduce to one pixel matrix.
  index_type i = itot % (DSIPM_N_QUADCOL * DSIPM_N_QUADCOL);

  // Map index to pixel number.
  //                |----------- jump 4 rows ----------| + |--- jump 1 row --| - |---- next column ----|
  index_type ipix = ((1 + rowBy8 * (i % rowBy8)) * rowBy2) + rowBy2 * (i / rowBy8) - (rowTm2 + 1) * (i / rowBy2);

  return ipix;
}

dSiPMFrameDecoder::index_type dSiPMFrameDecoder::getQuad(index_type itot) {
  // Calculate quadrant index.
  return itot / (DSIPM_N_QUADCOL * DSIPM_N_QUADCOL);
}

dSiPMFrameDecoder::index_type dSiPMFrameDecoder::getQuadCol(index_type ipix) {
  // Map quadrant index to quadrant column.
  return (ipix - 1) % DSIPM_N_QUADCOL;
}

dSiPMFrameDecoder::index_type dSiPMFrameDecoder::getQuadRow(index_type ipix) {
  // Map quadrant index to quadrant row.
  return (ipix - 1) / DSIPM_N_QUADCOL;
}

dSiPMFrameDecoder::index_type dSiPMFrameDecoder::getCol(index_type ipix, index_type quad) {
  // Map quadrant column to global column/
  index_type col;
  switch(quad) {
  case index_type(0):
    col = DSIPM_N_QUADCOL - 1 - getQuadCol(ipix);
    break;
  case index_type(1):
    col = DSIPM_N_QUADCOL + getQuadCol(ipix);
    break;
  case index_type(2):
    col = DSIPM_N_QUADCOL - 1 - getQuadCol(ipix);
    break;
  case index_type(3):
    col = DSIPM_N_QUADCOL + getQuadCol(ipix);
    break;
  default:
    LOG(ERROR) << "Conversion error in getCol, quadrant " << quad << " not defined.";
    col = DSIPM_N_COL; // This will be out of boundaries...
  }
  return col;
}

dSiPMFrameDecoder::index_type dSiPMFrameDecoder::getRow(index_type ipix, index_type quad) {
  // Map quadrant row to global row.
  index_type row;
  switch(quad) {
  case index_type(0):
    row = 2 * DSIPM_N_QUADROW - 1 - getQuadRow(ipix);
    break;
  case index_type(1):
    row = 2 * DSIPM_N_QUADROW - 1 - getQuadRow(ipix);
    break;
  case index_type(2):
    row = DSIPM_N_QUADROW - 1 - getQuadRow(ipix);
    break;
  case index_type(3):
    row = DSIPM_N_QUADROW - 1 - getQuadRow(ipix);
    break;
  default:
    LOG(ERROR) << "Conversion error in getRow, quadrant " << quad << " not defined.";
    row = DSIPM_N_ROW; // This will be out of boundaries...
  }
  return row;
}

dSiPMFrameDecoder::index_type dSiPMFrameDecoder::getRemap2bit(index_type ipix) {
  // There are 8 chunks of 1/2 quadrant size.
  index_type chunk_n = 8;
  index_type chunk_size = DSIPM_N_PIX / chunk_n;
  // Derive index inside a chunk.
  index_type i_in_chunk = ipix % chunk_size;
  // Derive the index of the chunk
  index_type i_of_chunk = ipix / chunk_size;
  // Now we have to map the chunk index 0, 1, 2, 3, 4, 5, 6, 7
  //                                 to 0, 2, 4, 6, 1, 3, 5, 7
  i_of_chunk = i_of_chunk * 2 - (chunk_n - 1) * (ipix / (DSIPM_N_PIX / 2));
  // And rebuild the index
  index_type remaped = i_of_chunk * chunk_size + i_in_chunk;
  // Check
  LOG(DEBUG) << "getRemap2bit " << ipix << " -> " << remaped;
  // F***, what a pain, but it works. And performance is no issue because
  // this is only calles uppon construction of the class.
  return remaped;
}
