#ifndef NetScopeStandaloneAnalyzer_HH
#define NetScopeStandaloneAnalyzer_HH
#define NetScope_CHANNELS 8
#define NetScope_TIMES 1
#define NetScope_SAMPLES 502
#define NetScope_F_SAMPLES 0
#define SCOPE_MEM_LENGTH_MAX 12500000
 
#include "DatAnalyzer.hh"
#include <assert.h>


// This is the class that should be used for parsing and analyzing
// NetScope data files in .root format produced by the python script.

class NetScopeStandaloneAnalyzer : public DatAnalyzer {
  public:
	  struct FTBFPixelEvent {
	      double xSlope;
	      double ySlope;
	      double xIntercept;
	      double yIntercept;
          double chi2;
          double xResidBack;
	      double yResidBack;
	      int trigger;
	      int runNumber;
	      int nPlanes;
          int numPixels;
          int numBackPlanes;
          Long64_t timestamp;
	      Long64_t bco;
	  };

    //Scope Tektronix DPO7254 ADC already in account in the binary conversion
    NetScopeStandaloneAnalyzer() : DatAnalyzer(NetScope_CHANNELS, NetScope_TIMES, NetScope_SAMPLES, 1, 1., NetScope_F_SAMPLES) {}

    void GetCommandLineArgs(int argc, char **argv);

    void InitLoop();

    int GetChannelsMeasurement(int i_aux);

    unsigned int GetTimeIndex(unsigned int n_ch) { return 0; }

    void Analyze();
    
  protected:
    vector<int> active_ch = {0,1,2,3};
    // Set by command line arguments or default
    TString pixel_input_file_path;
    bool skip_tracks;
    // Pixel events variables
    FTBFPixelEvent* pixel_event= nullptr;
    TFile *pixel_file = nullptr;
    TTree *pixel_tree = nullptr;

    unsigned long int idx_px_tree = 0;
    unsigned long int entries_px_tree = 0;

    float xIntercept=0;
    float yIntercept=0;
    float xSlope=0;
    float ySlope=0;
    vector<float> x_DUT;
    vector<float> y_DUT;
    float chi2=0;
    float xResidBack=0;
    float yResidBack=0;
    int ntracks=0;
    int nplanes=0;
    int npix=0;
    int nback=0;
};

#endif
