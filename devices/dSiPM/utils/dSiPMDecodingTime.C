
#define TC_const 2.45098E-9

// extract name from path
std::vector<std::string> splitpath(const std::string& str, const std::set<char> delimiters) {
  std::vector<std::string> result;
  char const* pch = str.c_str();
  char const* start = pch;
  for(; *pch; ++pch) {
    if(delimiters.find(*pch) != delimiters.end()) {
      if(start != pch) {
        std::string str(start, pch);
        result.push_back(str);
      } else {
        result.push_back("");
      }
      start = pch + 1;
    }
  }
  result.push_back(start);
  return result;
}
//

void dSiPMDecodingTime(const char* filename,
                       float TFval_Q1 = 95.2E-12,
                       float TFval_Q2 = 95.2E-12,
                       float TFval_Q3 = 95.2E-12,
                       float TFval_Q4 = 95.2E-12,
                       bool save = 1) {

  TH1::AddDirectory(false);
  string s_filename = filename;
  std::set<char> delims{'/'};
  std::vector<std::string> path = splitpath(s_filename, delims);
  string base = path.back();
  string name = base.substr(0, base.size() - 4);

  FILE* infile = fopen(filename, "r");
  TFile* outfile = new TFile(Form("%s.root", name.c_str()), "RECREATE");

  int nframe = 0;
  long long int bc = 0;
  int col = 0;
  int row = 0;
  int hit = 0;
  int valid = 0;
  int TF = 0;
  int TC = 0;
  float ToA = 0;

  float ToA_Q1_att = 0;
  float ToA_Q2_att = 0;
  float ToA_Q3_att = 0;
  float ToA_Q4_att = 0;

  TTree* dsipm_tree = new TTree("dsipm_tree", "tree of dSiPM output data");
  dsipm_tree->Branch("nframe", &nframe);
  dsipm_tree->Branch("bc", &bc);
  dsipm_tree->Branch("col", &col);
  dsipm_tree->Branch("row", &row);
  dsipm_tree->Branch("hit", &hit);
  dsipm_tree->Branch("valid", &valid);
  dsipm_tree->Branch("TF", &TF);
  dsipm_tree->Branch("TC", &TC);
  dsipm_tree->Branch("ToA", &ToA);

  TH1F* TF_ALL = new TH1F("TF_ALL", "TF_ALL", 32, -0.5, 31.5);
  TH1F* TC_ALL = new TH1F("TC_ALL", "TC_ALL", 128, -0.5, 127.5);
  TH1F* TF_Q1 = new TH1F("TF_Q1", "TF_Q1", 32, -0.5, 31.5);
  TH1F* TC_Q1 = new TH1F("TC_Q1", "TC_Q1", 128, -0.5, 127.5);
  TH1F* TF_Q2 = new TH1F("TF_Q2", "TF_Q2", 32, -0.5, 31.5);
  TH1F* TC_Q2 = new TH1F("TC_Q2", "TC_Q2", 128, -0.5, 127.5);
  TH1F* TF_Q3 = new TH1F("TF_Q3", "TF_Q3", 32, -0.5, 31.5);
  TH1F* TC_Q3 = new TH1F("TC_Q3", "TC_Q3", 128, -0.5, 127.5);
  TH1F* TF_Q4 = new TH1F("TF_Q4", "TF_Q4", 32, -0.5, 31.5);
  TH1F* TC_Q4 = new TH1F("TC_Q4", "TC_Q4", 128, -0.5, 127.5);

  TH1F* TOA_Q1 = new TH1F("TOA_Q1", "TOA_Q1", 3000, -0.5e-9, 299.5e-9);
  TH1F* TOA_Q2 = new TH1F("TOA_Q2", "TOA_Q2", 3000, -0.5e-9, 299.5e-9);
  TH1F* TOA_Q3 = new TH1F("TOA_Q3", "TOA_Q3", 3000, -0.5e-9, 299.5e-9);
  TH1F* TOA_Q4 = new TH1F("TOA_Q4", "TOA_Q4", 3000, -0.5e-9, 299.5e-9);

  TH1F* TDIFF_12 = new TH1F("TDIFF_12", "TDIFF_12", 7500, -350.5e-9, 349.5e-9);
  TH1F* TDIFF_13 = new TH1F("TDIFF_13", "TDIFF_13", 7500, -350.5e-9, 349.5e-9);
  TH1F* TDIFF_14 = new TH1F("TDIFF_14", "TDIFF_14", 7500, -350.5e-9, 349.5e-9);
  TH1F* TDIFF_23 = new TH1F("TDIFF_23", "TDIFF_23", 7500, -350.5e-9, 349.5e-9);
  TH1F* TDIFF_24 = new TH1F("TDIFF_24", "TDIFF_24", 7500, -350.5e-9, 349.5e-9);
  TH1F* TDIFF_34 = new TH1F("TDIFF_34", "TDIFF_34", 7500, -350.5e-9, 349.5e-9);

  while(fscanf(infile, " %lli %i %i %i %i %i %i ", &bc, &col, &row, &hit, &valid, &TF, &TC) != EOF) {
    if(bc == -1 && valid == -1 && hit == -1) {
      if(nframe % 1000 == 0) {
        cout << "Analyzing frame " << nframe << endl;
      }
      nframe++;
      TOA_Q1->Fill(ToA_Q1_att);
      TOA_Q2->Fill(ToA_Q2_att);
      TOA_Q3->Fill(ToA_Q3_att);
      TOA_Q4->Fill(ToA_Q4_att);

      TDIFF_12->Fill(ToA_Q1_att - ToA_Q2_att);
      TDIFF_13->Fill(ToA_Q1_att - ToA_Q3_att);
      TDIFF_14->Fill(ToA_Q1_att - ToA_Q4_att);
      TDIFF_23->Fill(ToA_Q2_att - ToA_Q3_att);
      TDIFF_24->Fill(ToA_Q2_att - ToA_Q4_att);
      TDIFF_34->Fill(ToA_Q3_att - ToA_Q4_att);

      ToA_Q1_att = 0;
      ToA_Q2_att = 0;
      ToA_Q3_att = 0;
      ToA_Q4_att = 0;
    }
    dsipm_tree->Fill();

    if(col < 16 && row > 15) { // quad1
      TF_ALL->Fill(TF);
      TC_ALL->Fill(TC);
      TF_Q1->Fill(TF);
      TC_Q1->Fill(TC);
      if((TF * TFval_Q1 + TC * TC_const) > ToA_Q1_att) {
        ToA_Q1_att = (TF * TFval_Q1 + TC * TC_const);
      }
      ToA = ToA_Q1_att;
    }

    if(col > 15 && row > 15) { // quad2
      TF_ALL->Fill(TF);
      TC_ALL->Fill(TC);
      TF_Q2->Fill(TF);
      TC_Q2->Fill(TC);
      if((TF * TFval_Q2 + TC * TC_const) > ToA_Q2_att) {
        ToA_Q2_att = (TF * TFval_Q2 + TC * TC_const);
      }
      ToA = ToA_Q2_att;
    }

    if(col < 16 && row < 16) { // quad3
      TF_ALL->Fill(TF);
      TC_ALL->Fill(TC);
      TF_Q3->Fill(TF);
      TC_Q3->Fill(TC);
      if((TF * TFval_Q3 + TC * TC_const) > ToA_Q3_att) {
        ToA_Q3_att = (TF * TFval_Q3 + TC * TC_const);
      }
      ToA = ToA_Q3_att;
    }

    if(col > 15 && row < 16) { // quad4
      TF_ALL->Fill(TF);
      TC_ALL->Fill(TC);
      TF_Q4->Fill(TF);
      TC_Q4->Fill(TC);
      if((TF * TFval_Q4 + TC * TC_const) > ToA_Q4_att) {
        ToA_Q4_att = (TF * TFval_Q4 + TC * TC_const);
      }
      ToA = ToA_Q4_att;
    }
  }
  fclose(infile);

  TCanvas* c1 = new TCanvas("timestamps", "timestamps", 1500, 900);
  // gStyle->SetOptStat(0);
  c1->Divide(5, 2);

  c1->cd(1);
  TF_ALL->Draw();
  gPad->Modified();
  gPad->Update();
  c1->cd(6);
  TC_ALL->Draw();
  gPad->Modified();
  gPad->Update();
  c1->cd(2);
  TF_Q1->Draw();
  gPad->Modified();
  gPad->Update();
  c1->cd(7);
  TC_Q1->Draw();
  gPad->Modified();
  gPad->Update();
  c1->cd(3);
  TF_Q2->Draw();
  gPad->Modified();
  gPad->Update();
  c1->cd(8);
  TC_Q2->Draw();
  gPad->Modified();
  gPad->Update();
  c1->cd(4);
  TF_Q3->Draw();
  gPad->Modified();
  gPad->Update();
  c1->cd(9);
  TC_Q3->Draw();
  gPad->Modified();
  gPad->Update();
  c1->cd(5);
  TF_Q4->Draw();
  gPad->Modified();
  gPad->Update();
  c1->cd(10);
  TC_Q4->Draw();
  gPad->Modified();
  gPad->Update();

  TCanvas* c2 = new TCanvas("ToA", "ToA", 900, 900);
  // gStyle->SetOptStat(0);
  c2->Divide(2, 2);

  c2->cd(1);
  TOA_Q1->Draw();
  gPad->Modified();
  gPad->Update();
  c2->cd(2);
  TOA_Q2->Draw();
  gPad->Modified();
  gPad->Update();
  c2->cd(3);
  TOA_Q3->Draw();
  gPad->Modified();
  gPad->Update();
  c2->cd(4);
  TOA_Q4->Draw();
  gPad->Modified();
  gPad->Update();

  TCanvas* c3 = new TCanvas("Tdiff", "Tdiff", 1200, 900);
  // gStyle->SetOptStat(0);
  c3->Divide(3, 2);

  c3->cd(1);
  TDIFF_12->Draw();
  gPad->Modified();
  gPad->Update();
  c3->cd(2);
  TDIFF_13->Draw();
  gPad->Modified();
  gPad->Update();
  c3->cd(3);
  TDIFF_14->Draw();
  gPad->Modified();
  gPad->Update();
  c3->cd(4);
  TDIFF_23->Draw();
  gPad->Modified();
  gPad->Update();
  c3->cd(5);
  TDIFF_24->Draw();
  gPad->Modified();
  gPad->Update();
  c3->cd(6);
  TDIFF_34->Draw();
  gPad->Modified();
  gPad->Update();

  if(save == 1) {
    dsipm_tree->Write();
    TF_ALL->Write();
    TC_ALL->Write();
    TF_Q1->Write();
    TC_Q1->Write();
    TF_Q2->Write();
    TC_Q2->Write();
    TF_Q3->Write();
    TC_Q3->Write();
    TF_Q4->Write();
    TC_Q4->Write();
    TOA_Q1->Write();
    TOA_Q2->Write();
    TOA_Q3->Write();
    TOA_Q4->Write();
    TDIFF_12->Write();
    TDIFF_13->Write();
    TDIFF_14->Write();
    TDIFF_23->Write();
    TDIFF_24->Write();
    TDIFF_34->Write();
  }

  outfile->Close();
}
