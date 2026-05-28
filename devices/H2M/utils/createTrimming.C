#include <vector>
#include "TFile.h"
#include "TTree.h"
#include "TGraph.h"
#include "string"
using namespace std;

void createTrimming(){
  int trimming_target = 63;
  // we need all the input files - add the trim dac to the data
  vector<pair<int, string>> files;
  for(int i=0; i<16;++i){
    string tmp = ("noise_scan_"+to_string(i)+".root");
    cout << "Adding: " << tmp << "at i = "<< i <<endl;
    files.push_back(make_pair(i,tmp));
  }
  // Need to create all lines - especially for pseudo fitting if we take two points
  vector<TGraph *> lines;
  for(int i=0; i<64*16;++i){
    lines.push_back(new TGraph(files.size()));
  }

  TCanvas *c1 = new TCanvas("c1","",1200,800);
  bool first = true;
  for( auto & f : files){
    cout << f.first <<"\t"<<f.second<<endl;
    TFile *file = new TFile(f.second.c_str(),"READ");
    if(!file->IsOpen()){
      cout << "ERROR - file cannot be opened: " << f.second<<" returning"<<endl;
      return;
    }
    TTree* tree = (TTree*) file->Get("trim_data");
    double tcol, trow, tmean, tsigma;
    tree->SetBranchAddress("col", &tcol);
    tree->SetBranchAddress("row", &trow);
    tree->SetBranchAddress("mean", &tmean);
    tree->SetBranchAddress("sigma", &tsigma);

    for (int i=0; i< tree->GetEntries();++i){
      tree->GetEntry(i);
      lines.at(tcol+64*trow)->SetPoint((f.first), (f.first), tmean);
    }
    if(first){
      first=false;
      tree->Draw("mean>>h(100,30,130)","col==1 && row==1");
    }else{
      tree->Draw("mean","col==1 && row==1","same");
    }
    //auto hist = (TH1D*) f.second->Get(mean);

  }
  
  lines.at(0)->Draw();
  lines.at(0)->GetYaxis()->SetRangeUser(30,100);
   for(auto & a: lines){

     a->Draw("same");
  }
    freopen("tuned.mask","w",stdout);

   cout << "# col row tb_enable mask tdac not top" <<endl;
   int idx=0;
   for(auto & l : lines){
     double vx =0, vy =0;
     double diff = 10000;
     int tune =0;
     for(int point =0; point < 16;point++){
       vx = l->GetPointX(point);
       vy = l->GetPointY(point);
       //       cout << vx << "\t" << vy << "\t" << diff <<endl;
       if(fabs(vy-trimming_target)<diff){
	 diff = fabs(vy-trimming_target);
	 tune = vx;
       }
     }
     cout << (idx%64) <<" " << (idx/64) << " 1 0 "<< tune << " 0 " <<endl;
     idx++;
   }
}
