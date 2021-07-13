#!/bin/bash
# eos root://cmseos.fnal.gov rm -r /store/group/cmstestbeam/2020_02_CMSTiming/condor
# eos root://cmseos.fnal.gov mkdir /store/group/cmstestbeam/2020_02_CMSTiming/condor
# eos root://cmseos.fnal.gov mkdir /store/group/cmstestbeam/2020_02_CMSTiming/condor/config
xrdcp -f NetScopeStandaloneDat2Root root://cmseos.fnal.gov//store/group/cmstestbeam/2021_CMSTiming_ETL/condor/NetScopeStandaloneDat2Root502
xrdcp -f config/FNAL_TestBeam_1904/LecroyScope*.config root://cmseos.fnal.gov//store/group/cmstestbeam/2021_CMSTiming_ETL/condor/config/FNAL_TestBeam_1904/
#xrdcp -f /uscms/home/rheller/work/TestBeamReco/ETL_Agilent_MSO-X-92004A/Reconstruction/conversion_bin_fast.py root://cmseos.fnal.gov//store/group/cmstestbeam/2021_CMSTiming_ETL/condor/
