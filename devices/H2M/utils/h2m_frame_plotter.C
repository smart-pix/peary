// draw hitmaps or event displays from h2m data obtained with printFrames

void h2m_frame_plotter(const char* filename, int i = -1) {

  // cosmetics
  gStyle->SetPalette(kNeon); // this is uggly, but min and max of the scale are visible on white BG

  // strip .txt
  string s_filename = filename;
  string name = s_filename.substr(0, s_filename.size() - 4);

  // ascii to tree
  TTree* frame_tree = new TTree("frame_tree", "tree containing 1 h2m frame");
  frame_tree->ReadFile(filename, "col/I:row/I:val/I:mode/I");

  // prepare histograms
  int n_col = 64;
  int n_row = 16;

  TH1D* h_col = new TH1D("h_col", ";col;", n_col, 0 - 0.5, n_col - 0.5);
  TH1D* h_row = new TH1D("h_row", ";row;", n_row, 0 - 0.5, n_row - 0.5);
  TH2D* h_colrow = new TH2D("h_colrow", ";col;row;", n_col, 0 - 0.5, n_col - 0.5, n_row, 0 - 0.5, n_row - 0.5);

  // make plots
  if(i == -1) { // hitmap

    cout << "Drawing hitmaps" << endl;

    TCanvas* c_col = new TCanvas("c_col", "c_col", 100, 100, 640, 640);
    frame_tree->Draw("col>>h_col");

    TCanvas* c_row = new TCanvas("c_row", "c_row", 200, 200, 640, 640);
    frame_tree->Draw("row>>h_row");

    TCanvas* c_colrow = new TCanvas("c_colrow", "c_colrow", 300, 300, 640, 640);
    frame_tree->Draw("row:col>>h_colrow", "", "COLZ");
  }
}
