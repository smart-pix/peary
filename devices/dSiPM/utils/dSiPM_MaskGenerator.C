/////////////////////////////////////////////////////////////////////////////////////
// Create a new mask file starting from an existing one, masking a given percentage of noisy pixels
// Usage:
/*
root -l
gStyle->SetOptStat(0)                              # Switch off stat box (for maps)
.x dSiPM_MaskGenerator.C("frameData.txt","oldmask.cfg","newmask.cfg",0.1)

#use frameData.txt to evaluate the noisy pixels then create newmask.cfg updating oldmask.cfg and reaching the 10% of masked pixel
#if more than 10% of the pixels are already masked in oldmask.cfg no new mask file will be created
*/
/////////////////////////////////////////////////////////////////////////////////////


// make compilable
#include <fstream>
#include <iostream>
#include <cmath>
#include "TCanvas.h"
#include "TH1.h"
#include "TH2.h"
#include "TStyle.h"
#include "TSystem.h"
#include "TTree.h"
#include "TFile.h"
//pixel struct

struct pixel {
  int col;
  int row;
  int val;
  bool mask;
  bool operator < (const pixel& another) const {return val > another.val;}
};

// main
void dSiPM_MaskGenerator(const char* filename, const char* oldmask, const char* newmask, double noisy_pixel_cut = 0.){
  // cosmetics
  gStyle->SetPalette(kNeon); // this is ugly, but min and max of the scale are visible on white BG

  // ascii to tree
  TTree* dsipm_tree = new TTree("dsipm_tree", "tree of dSiPM output data");
  dsipm_tree->ReadFile(filename, "bunchCounter/I:col/I:row/I:hit/I:valid/I:TF/I:TC/I");

  // prepare histograms
  int N = 32;
  TH2D* h_colrow = new TH2D("h_colrow", ";col;row;", N, 0 - 0.5, N - 0.5, N, 0 - 0.5, N - 0.5);

  // draw
  TCanvas* c_colrow = new TCanvas("c_colrow", "c_colrow", 100, 100, 640, 640);
  dsipm_tree->Draw("row:col>>h_colrow", "", "COLZ");


  //input old mask
  FILE *inputfile = fopen(oldmask,"r");
  bool CurrentMask[N][N];
  char temps[10];
  int tempint;
  for(int row = N-1; row >= 0; row--){
    fscanf(inputfile, "%s %i", temps, &tempint);
    for(int col = 0; col < N; col++){
    fscanf(inputfile, "%i",&tempint);
    CurrentMask[row][col] = tempint;
    }
  }
  fclose(inputfile);
  // counter for masked cells
  int counterMasked = 0;

  // fill pixel struct array
  pixel HitMap[1024];
  int position = 0;
  for(int col = 0; col < N; col++) {
    for(int row = 0; row < N; row++) {
      HitMap[position].col = col;
      HitMap[position].row = row;
      HitMap[position].val = h_colrow -> GetBinContent(col+1, row+1);
      HitMap[position].mask = CurrentMask[row][col];
      if(CurrentMask[row][col] == 0) counterMasked++;
      //cout<<"position: "<<position<<" x-y: "<<HitMap[position].col<<" "<<HitMap[position].row<<" val: "<<HitMap[position].val<<endl;
      position ++;
    }
  }

  //masking
  cout<<"Input file: "<<oldmask<<" contains "<<counterMasked<<" masked pixels (total masked = "<<((double)counterMasked/1024)<<"%)"<<endl;
  if(((double)counterMasked/1024)>=noisy_pixel_cut){
    cout<<"More than "<<noisy_pixel_cut*100<<"% of the pixels are already masked in the input mask file: "<<oldmask<<endl;
  }
  else{
    //sort pixel struct array
    std::sort(std::begin(HitMap), std::end(HitMap));

    //copute pixel to mask
    int pixelsToMask = round((noisy_pixel_cut-((double)counterMasked/1024))*1024);

    //mask pixels
    for (int i = 0; i < pixelsToMask; i++){
      CurrentMask[HitMap[i].row][HitMap[i].col]=0;
      cout<<"masking pixel "<< HitMap[i].col << " " << HitMap[i].row << endl;
    }

    //output new mask
    FILE *outputfile = fopen(newmask,"w");

    for(int row = N-1; row >= 0; row--){

      if(row >= 10) fprintf(outputfile, "%s %i  ", "row", row);
      else fprintf(outputfile, "%s  %i  ", "row", row);

      for(int col = 0; col < N; col++){
        fprintf(outputfile, "%i  ",CurrentMask[row][col]);
      }

      fprintf(outputfile, "\n");
    }

    //last line
    fprintf(outputfile, "#col    0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31");

    fclose(outputfile);

    cout<<"Output file: "<<newmask<<" contains "<<pixelsToMask<<" new masked pixels (total masked = "<<((double)(counterMasked+pixelsToMask)/1024)<<"%)"<<endl;

  }

}
