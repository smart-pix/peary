/////////////////////////////////////////////////////////////////////////////////////
// DCR analysis of dSiPM data obtained with printFrames
// Usage:
/*
root -l
gStyle->SetOptStat(0)                              # Switch off stat box (for maps)
.x dSiPMFrameDCR.C+("frameData.txt")               # just make DRC plots
.x dSiPMFrameDCR.C+("frameData.txt", 1)            # save plots and exit
.x dSiPMFrameDCR.C+("frameData.txt", 0, 0.10)      # exclude 10% noisiest pixels
.x dSiPMFrameDCR.C+("frameData.txt", 0, -1, 10200) # assume 10200 frames (default)
*/
/////////////////////////////////////////////////////////////////////////////////////

// make compilable
#include <fstream>
#include <iostream>
#include "TCanvas.h"
#include "TH1.h"
#include "TH2.h"
#include "TStyle.h"
#include "TSystem.h"
#include "TTree.h"

// global constants
const double bunchClock = 3.0 - (4.0 / 204.0); // [MHz] subtracting dead time for frame reset
const double maxDCR = 0.3;                     // [MHz]
const unsigned int nBins = 10000;              // For dcr histogra

// get bins from DCR map and fill into TH1D
TH1D* getHistFromMap(TH2D* in);

// main
void dSiPMFrameDCR(const char* filename, bool save = 0, double noisy_pixel_cut = -1, unsigned int nFrames = 10200) {

  // cosmetics
  gStyle->SetPalette(kNeon); // this is ugly, but min and max of the scale are visible on white BG

  // get plotname and voltage from filename
  string s_filename = filename;
  string name = s_filename.substr(0, s_filename.size() - 4); // strip .txt
  // string volt = s_filename.substr(19, s_filename.size() - 23); // get voltage
  string volt = s_filename.substr(20, s_filename.size() - 24); // get voltage
  // string volt = "30";
  std::replace(volt.begin(), volt.end(), 'V', '.'); // replace all 'V' to '.'

  // ascii to tree
  TTree* dsipm_tree = new TTree("dsipm_tree", "tree of dSiPM output data");
  dsipm_tree->ReadFile(filename, "bunchCounter/I:col/I:row/I:hit/I:valid/I:TF/I:TC/I");

  // prepare histograms
  int N = 32;
  TH2D* h_colrow = new TH2D("h_colrow", ";col;row;", N, 0 - 0.5, N - 0.5, N, 0 - 0.5, N - 0.5);
  TH2D* h_dcrMap = new TH2D();
  TH1D* h_dcr = new TH1D();

  // draw
  TCanvas* c_colrow = new TCanvas("c_colrow", "c_colrow", 100, 100, 640, 640);
  dsipm_tree->Draw("row:col>>h_colrow", "", "COLZ");

  TCanvas* c_dcrMap = new TCanvas("c_dcrMap", "c_dcrMap", 200, 200, 640, 640);
  c_dcrMap->SetLogz();
  // clone, scale and adjust histogram
  h_dcrMap = (TH2D*)h_colrow->Clone();
  h_dcrMap->SetTitle(Form("DCR [MHz], at 1C and %s V", volt.c_str()));
  h_dcrMap->Scale(bunchClock / (double)nFrames);
  h_dcrMap->SetMaximum(maxDCR);
  h_dcrMap->Draw("COLZ");

  TCanvas* c_dcr = new TCanvas("c_dcr", "c_dcr", 300, 300, 640, 640);
  h_dcr = getHistFromMap(h_dcrMap);

  // cut the given amount of noisy pixels
  unsigned int integral = h_dcr->GetEntries(); // include oveflow bin
  if(noisy_pixel_cut > 0 && integral > 0 && h_dcr->GetBinContent(1) != integral) {

    // check cut range
    if(noisy_pixel_cut >= 1) {
      cout << "ERROR: Please choose noisy pixel cut between 0 and 1" << endl;
      return;
    }

    // find range to exclude the requested percantage of noisy pixels
    unsigned int reduce = 0;
    while(h_dcr->Integral() > (integral - integral * noisy_pixel_cut) && h_dcr->Integral() > 0 && reduce <= nBins) {
      h_dcr->GetXaxis()->SetRangeUser(0, h_dcr->GetBinCenter(nBins - reduce));
      reduce++;
    }

    // check overflow bin content
    if(reduce == 0) {
      cout << "ERROR: Already " << h_dcr->GetBinContent(nBins + 1) << " entries in overflow bin. Adjust histogram range!"
           << endl;
    }
  }

  h_dcr->Draw();
  h_dcr->SetTitle(Form("%s V", volt.c_str()));

  // print some statistics to file
  std::ofstream outfileDCR;
  outfileDCR.open("scanDCR.txt", std::ios_base::app);
  double meanDCR = h_dcr->GetMean();
  outfileDCR << volt << " " << meanDCR << endl;
  outfileDCR.close();

  double maxiDCR = h_dcr->GetBinCenter(h_dcr->FindLastBinAbove(0));
  cout << volt << " " << meanDCR << " " << maxiDCR << endl;

  // save and exit
  if(save) {
    c_dcrMap->SaveAs(Form("dcrMap_%s.png", name.c_str()));
    c_dcr->SaveAs(Form("dcrHst_%s.png", name.c_str()));
    gSystem->Exit(0);
  }

} // main

TH1D* getHistFromMap(TH2D* in) {

  TH1D* out = new TH1D("h_out", ";DCR [MHz];", nBins, 0, maxDCR);

  // loop map
  for(int col = 0; col < in->GetNbinsX(); col++) {
    for(int row = 0; row < in->GetNbinsY(); row++) {
      double val = in->GetBinContent(col, row);
      out->Fill(val);
    }
  }

  return out;
}
