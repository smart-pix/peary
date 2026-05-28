// Analysis macro for H2M dac-scans

void H2M_scanReader(string filename = "", int plotAt = 255) {

  size_t strip_from = filename.find_last_of(".");
  std::string rootFileName = Form("%s.root", filename.substr(0, strip_from).c_str());

  // H2M chip parameters
  const int n_col = 64;
  const int n_row = 16;
  const int n_dac = 256;
  const int n_tdc = 256;

  TH2D* h_curve_2D =
    new TH2D("curve_2D", ";amplitude;occupancy;entries", n_dac, 0 - 0.5, n_dac - 0.5, n_dac, 0 - 0.5, n_dac - 0.5);
  TH1D* h_curve_sum = new TH1D("curve_sum", ";amplitude;occupancy;entries", n_dac, 0 - 0.5, n_dac - 0.5);
  TH2D* occupancy = new TH2D("occupancy", "row; column; entries", n_col, 0 - 0.5, n_col - 0.5, n_row, 0 - 0.5, n_row - 0.5);
  TH2D* hitmap_for_rafael =
    new TH2D("hitmap_173", "hitmap;column;row; pixel TOT [TDC]", n_col, 0 - 0.5, n_col - 0.5, n_row, 0 - 0.5, n_row - 0.5);
  TH2D* occupancy_sum =
    new TH2D("occupancy_sum", "row; column; entries", n_col, 0 - 0.5, n_col - 0.5, n_row, 0 - 0.5, n_row - 0.5);
  TH1D* h_response_sum = new TH1D("response_sum", ";response [tdc];entries", n_tdc, 0 - 0.5, n_tdc - 0.5);

  TH1D* h_pixel_responses[n_col][n_row];
  for(int col = 0; col < n_col; col++) {
    for(int row = 0; row < n_row; row++) {
      h_pixel_responses[col][row] =
        new TH1D(Form("h_pixel_response_%i_%i", col, row), ";response [tdc];entries", n_tdc, 0 - 0.5, n_tdc - 0.5);
    }
  }

  // fill arrays in loops
  int i = 0;
  double occ_map_pulse[n_col][n_row][n_dac] = {0};
  std::vector<double> ampl;
  ampl.resize(n_dac, 0);

  // fit parameter
  float th[n_col][n_row];
  float noise[n_col][n_row];
  float chi[n_col][n_row];

  // open data file
  std::ifstream data_in;
  data_in.open(filename.c_str(), std::ios::ate | std::ios::binary);

  // output file to save histogram
  TFile* fout = new TFile(rootFileName.c_str(), "RECREATE");
  TDirectory* dir = fout->GetDirectory("");
  TDirectory* dir_single = dir->mkdir("single_pixel_plots");
  TTree* T = new TTree("trim_data", "pixel thresholds for trimming");
  T->SetAutoSave(0);
  double tcol, trow, tmean, tsigma;
  T->Branch("col", &tcol);
  T->Branch("row", &trow);
  T->Branch("mean", &tmean);
  T->Branch("sigma", &tsigma);

  std::cout << filename.c_str() << std::endl;

  if(data_in) {

    std::cout << filename.c_str() << std::endl;

    // get length of file:
    data_in.seekg(0, data_in.end);
    auto length = data_in.tellg();
    data_in.seekg(0, data_in.beg);
    std::cout << "Reading " << length << " bytes " << std::endl;

    // buffer for one block of testpulses
    std::streamsize blocksize = n_row * n_col + 6;
    uint16_t block[blocksize];

    // containers for the occupancy map and amplitude
    uint16_t occupancy_map[n_col][n_row] = {0};
    uint16_t amplitude = 0;
    uint16_t scan_dac_i = 0;
    uint16_t scan_start = 0;
    uint16_t scan_step = 0;
    uint16_t scan_end = 0;
    uint16_t scan_n = 0;

    // reading data from binary file
    int counts;
    while(data_in.read(reinterpret_cast<char*>(&block), 2 * blocksize)) {

      counts = 0;

      for(size_t col = 0; col < n_col; col++) {
        for(size_t row = 0; row < n_row; row++) {
          occupancy_map[col][row] = block[col + row * n_col];
        }
      }
      amplitude = block[n_col * n_row + 0];
      scan_start = block[n_col * n_row + 1];
      scan_step = block[n_col * n_row + 2];
      scan_end = block[n_col * n_row + 3];
      scan_n = block[n_col * n_row + 4];
      scan_dac_i = block[n_col * n_row + 5];

      for(int col = 0; col < n_col; col++) {
        for(int row = 0; row < n_row; row++) {

          // count
          if(occupancy_map[col][row] > 0)
            counts++;
          if(i == 173 - 1)
            hitmap_for_rafael->Fill(col, row, occupancy_map[col][row]);

          // fill containers and histograms for data analysis and plotting
          h_curve_2D->Fill(amplitude, 100.0 * occupancy_map[col][row] / scan_n);
          h_curve_sum->Fill(amplitude, 100.0 * occupancy_map[col][row] / scan_n);

          // this makes only sense for e.g. TOT spectra
          h_pixel_responses[col][row]->Fill(occupancy_map[col][row]);
          if(!((col == 28 && row == 7) || (col == 44 && row == 15))) { // exclude noisy
            h_response_sum->Fill(occupancy_map[col][row]);
          }

          // this makes only sense for dac_scans
          if(i < n_dac) {
            ampl[i] = amplitude;
            occ_map_pulse[col][row][i] = 100.0 * occupancy_map[col][row] / scan_n;
          }
        }
      }

      // this counts the number of block read (0== scan_start)
      i = i + 1;
      if(counts > 5)
        cout << "hooray, " << counts << " counts in event " << i << endl;
    }

    TF1* fitfunction;
    if(scan_dac_i == 0) {
      std::cout << "Assuming we have a test-pulse scan -- fitting Error function" << std::endl;
      // ToDo fix erf
      fitfunction = new TF1("fitfunction", "50*(1+erf((x-[0])/[1]))", scan_start, scan_end);
    } else if(scan_dac_i == 4 || scan_dac_i == 5) {
      std::cout << "Assuming we have a reference voltage scan -- fitting Gaussians" << std::endl;
      fitfunction = new TF1("fitfunction", "[2]*TMath::Gaus(x-[0],[1],1)", scan_start, scan_end);
    } else {
      std::cout << "I do not know this type of scan -- please tell me what to do" << std::endl;
    }

    TH1D* threshold = new TH1D("threshold", ";threshold;entries", n_dac * 10, 0 - 0.5, n_dac - 0.5);
    TH1D* noiseplot = new TH1D("noise", ";noise;entries", n_dac * 10, 0 - 0.5, n_dac - 0.5);

    for(int col = 0; col < n_col; col++) {
      for(int row = 0; row < n_row; row++) {

        // make one histogram per pixel
        TH1D* h_pixel_curve =
          new TH1D(Form("h_pixel_curve_%i_%i", col, row), ";amplitude;entries", n_dac, 0 - 0.5, n_dac - 0.5);

        for(int i = 0; i < n_dac; i++) {
          h_pixel_curve->SetBinContent(h_pixel_curve->FindBin(ampl[i]), occ_map_pulse[col][row][i]);
          occupancy_sum->Fill(col, row, occ_map_pulse[col][row][i]);
        }
        int map_index = find(ampl.begin(), ampl.end(), plotAt) - ampl.begin();
        occupancy->Fill(col, row, occ_map_pulse[col][row][map_index]);

        if(scan_dac_i == 0) {
          // ToDo implement proper parameter estimation
          fitfunction->SetParameters(50, 1);
          auto fit_result = h_pixel_curve->Fit("fitfunction", "RIQ");
          if(fit_result != 0) {
            std::cout << "FAILED FIT! col row: " << col << " " << row << std::endl;
          }
        } else if(scan_dac_i == 4 || scan_dac_i == 5) {
          fitfunction->SetParameters(h_pixel_curve->GetMean(), h_pixel_curve->GetRMS());
          auto fit_result = h_pixel_curve->Fit("fitfunction", "RIQ");
          if(fit_result != 0) {
            std::cout << "FAILED FIT! col row: " << col << " " << row << std::endl;
          }
        }

        dir_single->cd();
        h_pixel_curve->Write();
        h_pixel_responses[col][row]->Write();

        th[col][row] = fitfunction->GetParameter(0);
        noise[col][row] = fitfunction->GetParameter(1);
        chi[col][row] = fitfunction->GetChisquare() / fitfunction->GetNDF();

        dir->cd();
        tcol = col;
        trow = row;
        tmean = fitfunction->GetParameter(0);
        tsigma = fitfunction->GetParameter(1);
        T->Fill();
      }
    } // col

    for(int col = 0; col < n_col; col++) {
      for(int row = 0; row < n_row; row++) {

        threshold->Fill(th[col][row]);
        noiseplot->Fill(noise[col][row]);

        std::cout << "col: " << col << " row: " << row << std::endl;
        if(chi[col][row] > 5) {
          std::cout << "  WARNING: bad chi squared / ndof: " << chi[col][row] << std::endl;
        }
        std::cout << "  threshold " << th[col][row] << std::endl;
        std::cout << "  noise " << noise[col][row] << std::endl;
        std::cout << "  chi squared / ndof " << chi[col][row] << std::endl;
      }
    }

    h_curve_2D->Write();
    h_curve_sum->Write();
    h_response_sum->Write();
    occupancy->Write();
    occupancy_sum->Write();
    threshold->Write();
    noiseplot->Write();
    hitmap_for_rafael->Write();
    T->Write();
    fout->Close();

  } // if open
  else {
    std::cout << "Failed to open " << filename << std::endl;
  }
  gSystem->Exit(0);
}
