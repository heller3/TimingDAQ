#include "NetScopeStandaloneAnalyzer.hh"

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

std::string NetScopeStandaloneAnalyzer::split(const std::string& half, const std::string& s, const std::string& h) const
{
    if(s.find(h) != std::string::npos)
    {
        std::string token;
        if      ("first"==half) token = s.substr(0, s.find(h));
        else if ("last" ==half) token = s.substr(s.find(h) + h.length(), std::string::npos);
        return token;
    }
    else
    {
        return s;
    }
}

void NetScopeStandaloneAnalyzer::GetDim(TTree* const tree, const std::string& var, unsigned int& f, unsigned int& s)
{
    TBranch* branch = tree->GetBranch(var.c_str());
    TObjArray *lol = branch->GetListOfLeaves();
    TLeaf *leaf = (TLeaf*)lol->UncheckedAt(0);
    std::string title = leaf->GetTitle();
    std::string firstdim  = split("last", split("first", title, "]"), "[");
    std::string seconddim = split("first", split("last", title, "]["), "]");
    f = static_cast<unsigned int>(std::atoi(firstdim.c_str()));
    s = static_cast<unsigned int>(std::atoi(seconddim.c_str()));
}

void NetScopeStandaloneAnalyzer::InitLoop(){
  cout<<"Define numChannels, numTime, and numSamples from input TTree"<<endl;
  unsigned int numChannels, numTime, numSamples;
  GetDim(tree_in, "channel", numChannels, numSamples);
  GetDim(tree_in, "time", numTime, numSamples);
  setNumChannels(numChannels);
  setNumTimes(numTime);
  setNumSamples(numSamples);
  for(unsigned int i = 0; i < numChannels; i++){active_ch.emplace_back(i);}

  DatAnalyzer::InitLoop();
  cout<<"Finished datanalyzer InitLoop"<<endl;
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
    tree->Branch("npix", &npix, "npix/I");
    tree->Branch("nback", &nback, "nback/I");
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
    npix = 0;
    nback = 0;

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
          npix = pixel_event->numPixels;
          nback = pixel_event->numBackPlanes;
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
