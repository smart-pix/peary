// Draw hitmaps or event displays from dSiPM data obtained with printFrames
// Usage:
/*
root -l
gStyle->SetOptStat(0)                         # -> Switch off stat box (for maps)
.x dSiPMFramePlotter.C("frameData.txt")       # -> Draw hitmap
.x dSiPMFramePlotter.C("frameData.txt", 1)    # -> Draw hitmap, save as .png and exit root
.x dSiPMFramePlotter.C("frameData.txt", 0, 1) # -> Draw frame 11
*/

void dSiPMFramePlotter(const char* filename, bool save = 0, int i = -1) {

  // cosmetics
  gStyle->SetPalette(kNeon); // this is uggly, but min and max of the scale are visible on white BG

  // strip .txt
  string s_filename = filename;
  string name = s_filename.substr(0, s_filename.size() - 4);

  // ascii to tree
  TTree* dsipm_tree = new TTree("dsipm_tree", "tree of dSiPM output data");
  dsipm_tree->ReadFile(filename, "bunchCounter/I:col/I:row/I:hit/I:valid/I:TF/I:TC/I");

  // prepare histograms
  int N = 32;
  TH1D* h_col = new TH1D("h_col", ";col;", N, 0 - 0.5, N - 0.5);
  TH1D* h_row = new TH1D("h_row", ";row;", N, 0 - 0.5, N - 0.5);
  TH2D* h_colrow = new TH2D("h_colrow", ";col;row;", N, 0 - 0.5, N - 0.5, N, 0 - 0.5, N - 0.5);

  // make plots
  if(i == -1) { // hitmap
    cout << "Drawing hitmaps" << endl;

    TCanvas* c_col = new TCanvas("c_col", "c_col", 100, 100, 640, 640);
    dsipm_tree->Draw("col>>h_col");

    TCanvas* c_row = new TCanvas("c_row", "c_row", 200, 200, 640, 640);
    dsipm_tree->Draw("row>>h_row");

    TCanvas* c_colrow = new TCanvas("c_colrow", "c_colrow", 300, 300, 640, 640);
    dsipm_tree->Draw("row:col>>h_colrow", "", "COLZ");

    // save
    if(save) {
      c_colrow->SaveAs(Form("%s.png", name.c_str()));
    }

  } else { // event display
    cout << "Showing event " << i << endl;

    TCanvas* c_col = new TCanvas("c_col", "c_col", 100, 100, 640, 640);
    dsipm_tree->Draw("col>>h_col", Form("bunchCounter==%i", i));

    TCanvas* c_row = new TCanvas("c_row", "c_row", 200, 200, 640, 640);
    dsipm_tree->Draw("row>>h_row", Form("bunchCounter==%i", i));

    TCanvas* c_colrow = new TCanvas("c_colrow", "c_colrow", 300, 300, 640, 640);
    dsipm_tree->Draw("col:row>>h_colrow", Form("bunchCounter==%i", i), "COLZ");

    // save
    if(save) {
      c_colrow->SaveAs(Form("%s.png", name.c_str()));
    }
  }
  // if save, exit root for use in bash script
  if(save) {
    gSystem->Exit(0);
  }
}
