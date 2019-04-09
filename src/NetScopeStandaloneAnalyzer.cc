#include "NetScopeStandaloneAnalyzer.hh"
#define BUFSIZE 8192

using namespace std;

void NetScopeStandaloneAnalyzer::GetCommandLineArgs(int argc, char **argv){
  DatAnalyzer::GetCommandLineArgs(argc, argv);
  pixel_input_file_path = ParseCommandLine( argc, argv, "pixel_input_file" );
  if (pixel_input_file_path == ""){
    if (verbose) { cout << "Pixel input file not provided" << endl; }
  }
  else {
    if (verbose) { cout << "Pixel input file: " << pixel_input_file_path.Data() << endl; }
    pixel_file = new TFile( pixel_input_file_path.Data(),"READ");
    if (!pixel_file) {std::cout << "[ERROR]: Pixel file not found" << std::endl; exit(0);}
    TString tree_name = pixel_file->GetListOfKeys()->At(0)->GetName(); //Only works if it the tree is the first key
    pixel_tree = (TTree*)pixel_file->Get(tree_name);
    if (!pixel_tree) {cout << "[ERROR]: Pixel Tree not found\n"; exit(0);}
    entries_px_tree = pixel_tree->GetEntries();
  }

}


void NetScopeStandaloneAnalyzer::InitLoop(){
  DatAnalyzer::InitLoop();
  cout<<"finished datanalyzer InitLoop"<<endl;
  tree_in->SetBranchAddress("i_evt", &i_evt);
  tree_in->SetBranchAddress("channel", &(channel[0][0]));
  tree_in->SetBranchAddress("time", &(time[0][0]));


  cout<<"Trying to open pixel file"<<endl;
  if(pixel_input_file_path != ""){
    pixel_event = new FTBFPixelEvent;
    pixel_tree->SetBranchAddress("event", pixel_event);

    tree->Branch("xIntercept", &xIntercept, "xIntercept/F");
    tree->Branch("yIntercept", &yIntercept, "yIntercept/F");
    tree->Branch("xSlope", &xSlope, "xSlope/F");
    tree->Branch("ySlope", &ySlope, "ySlope/F");

    if(config->z_DUT.size()) {
      for(float zz : config->z_DUT) {
        x_DUT.push_back(0.);
        y_DUT.push_back(0.);
      }
      tree->Branch("x_dut", &(x_DUT[0]), Form("x_dut[%lu]/F", config->z_DUT.size()));
      tree->Branch("y_dut", &(y_DUT[0]), Form("y_dut[%lu]/F", config->z_DUT.size()));
    }
    tree->Branch("chi2", &chi2, "chi2/F");
    tree->Branch("ntracks", &ntracks, "ntracks/I");
    tree->Branch("nplanes", &nplanes, "nplanes/I");
    if (verbose) { cout << "   -->All pixel variables" << endl; }
    cout<<"Trying to get first entry"<<endl;
    pixel_tree->GetEntry(0);
    cout<<"Start of while loop"<<endl;
    cout<<"start_evt, pixel trigger"<<start_evt<<" "<<pixel_event->trigger<<endl;
    while(pixel_event->trigger <  start_evt && idx_px_tree < entries_px_tree-1) {
      cout<<"incrementing. "<<pixel_event->trigger<<endl;
      idx_px_tree++;
      pixel_tree->GetEntry( idx_px_tree );
    }
  }


}

// Fill tc, raw, time and amplitude
int NetScopeStandaloneAnalyzer::GetChannelsMeasurement(int i_aux) {
  //std::cout << "NNNN 2: " << tree_in->GetEntries() << std::endl;
  ResetAnalysisVariables();
  //cout<<"Reset variables"<<endl;
  //std::cout << "NNNN 3: " << tree_in->GetEntries() << std::endl;
  //cout<<"i_aux "<<i_aux<<endl;
  tree_in->GetEntry(i_aux);
//  cout<<"Got entry"<<endl;

  return 0;
}


/*
**************************************************
Include telescope data, then call main analyzer DatAnalyzer::Analyze()
**************************************************
*/
void NetScopeStandaloneAnalyzer::Analyze(){
	//cout<<"i_evt "<<i_evt<<endl;
  if(pixel_input_file_path != ""){
    xIntercept = -999;
    yIntercept = -999;
    xSlope = -999;
    ySlope = -999;
    for(unsigned int i = 0; i < config->z_DUT.size(); i++) {
      x_DUT[i] = -999;
      y_DUT[i] = -999;
    }
    chi2 = -999.;
    ntracks = 0;
    nplanes = 0;

    while (idx_px_tree < entries_px_tree && i_evt >= (pixel_event->trigger+0)) {
      pixel_tree->GetEntry(idx_px_tree);
      if ((pixel_event->trigger+0) == i_evt) {
        if(ntracks == 0) {
          xIntercept = 1e-3*pixel_event->xIntercept; //um to mm
          yIntercept = 1e-3*pixel_event->yIntercept;
          xSlope = pixel_event->xSlope;
          ySlope = pixel_event->ySlope;
          for(unsigned int i = 0; i < config->z_DUT.size(); i++) {
            x_DUT[i] = xIntercept + xSlope*(config->z_DUT[i]);
            y_DUT[i] = yIntercept + ySlope*(config->z_DUT[i]);
          }
          chi2 = pixel_event->chi2;
          nplanes = pixel_event->nPlanes;
        }
      	ntracks++;
        idx_px_tree++;
      }
      else if (i_evt > (pixel_event->trigger+0)) {
        cout << "[ERROR] Pixel tree not ordered" << endl;
        exit(0);
      }
    }

  }

  //calling main analyzer -- DatAnalyzer::Analyze() -- in DatAnalyzer.cc
  DatAnalyzer::Analyze();
}