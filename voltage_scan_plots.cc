// ~ This file really is C++! ~ //
// Daniel Richford & Evan Walker
// 2022-02-23
// CUNY/ISU/BNL
// script to fit all towers and look at varying the voltages

// Log files at /sphenix/data/data02/sphenix/hcal/912/test/[first_run_number].log for each set of scans (comprising many sets of 6 runs for each voltage setting

// 0|5  1|0  1|5  2|0  2|5  3|0  3|5  4|0  4|5  5|0  5|5 60|

// Include
#include <TH1.h>
#include <TStyle.h>
#include <TCanvas.h>
#include <TMath.h>
#include <TF1.h>
#include "TROOT.h"
#include <iostream> // std::cout
#include <iomanip>
#include <string> // std::string, std::stoi
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <typeinfo> // typeid().name()

// FUNCTION FORWARD-DECLARATIONS
void run_and_group_analysis_code(const std::string, const Int_t, const Int_t,const std::string,const std::string,const std::string);
std::string pad_number(int,int,char);
Int_t mapcanvas(Int_t);
std::string nameHistograms(int, std::string, std::string);
Int_t Which_LED(Int_t,Int_t);

// Colors
typedef struct Tincture{
  unsigned short color;
  unsigned short hatch;
} Tincture;
Tincture Argent  = {kWhite,         0}; // 0
Tincture Sable   = {kBlack,      1001}; // 1
Tincture Cendry  = {kGray+1,     3001}; // 2
Tincture Gules   = {kPink-4,     3007}; // 3
Tincture Rose    = {kMagenta-10, 3007}; // 4
Tincture Or      = {kOrange-4,   3002}; // 5
Tincture Buff    = {kYellow-10,  3002}; // 6
Tincture Vert    = {kTeal-5,     3005}; // 7
Tincture VertDLV = {kGreen-10,   3005}; // 8
Tincture Azure   = {kAzure-4,    3006}; // 9
Tincture Celeste = {kCyan-10,    3006}; // 10
Tincture Purpure = {kViolet+1,   3004}; // 11

// main function
int voltage_scan_plots
(
  std::string data_type, // "test" or "cosmics"
  Int_t       first_run, // 0123456789
  Int_t       last_run,  // 0123456789
  std::string hcal_component, // "inner," "i," "outer," or "o"
  std::string sector_number, // 1-32
  std::string plotdir
)
{
  try
    {
      run_and_group_analysis_code(
				  data_type,
				  first_run,
				  last_run,
				  hcal_component,
				  sector_number,
				  plotdir
				  );
    }

  catch (invalid_argument& e)
    {
      cerr << e.what() << endl;
      return 1;
    }
  catch (runtime_error& e)
    {
      cerr << e.what() << endl;
      return 1;
    }
  return 0;
}

void run_and_group_analysis_code
// arguments
(
  const std::string data_type, // "test" or "cosmics"
  const Int_t       first_run, // 0123456789
  const Int_t       last_run,  // 0123456789
  const std::string hcal_component, // "inner," "i," "outer," or "o"
  const std::string sector_number, // 1-32
  const std::string plotdir
 )
// function definition
{

      // Check arguments
      // std::cout << "Check args in module() function" << std::endl;
      // std::cout << "data_type " << data_type << " (" << typeid(data_type).name() << std::endl; 
      // std::cout << "first_run " << first_run << " (" << typeid(first_run).name() << std::endl; 
      // std::cout << " last_run " << last_run << " (" << typeid(last_run).name() << std::endl; 
      // std::cout << "hcal_cmpnt "<< hcal_component << " (" << typeid(hcal_component).name() << std::endl; 
      // std::cout << "sectr_num " << sector_number << " (" << typeid(sector_number).name() << std::endl; 

  // Argument Exceptions
  if(
     data_type != "test" &&
     data_type != "cosmics"
     )
    {
      throw invalid_argument("Argument Error: run_and_group_analysis_code: data type not 'test' or 'cosmics.' ");
    }
  if(
     hcal_component != "inner" &&
     hcal_component != "outer"
     )
    {
      throw invalid_argument("Argument Error: run_and_group_analysis_code: hcal_component not 'inner' or 'outer.' ");
    }
  if(
     first_run > last_run
     )
    {
      throw invalid_argument("Argument Error: run numbers should be [first] then [last], or the same");
    }
  if(
      sector_number.length() < 1 ||
      sector_number.length() > 2 ||
      std::stoi(sector_number) < 1 ||
     std::stoi(sector_number) > 32
     )
    {
      throw invalid_argument("Argument Error: run_and_group_analysis_code: sector number not integer 1-32");
    }
// 0|5  1|0  1|5  2|0  2|5  3|0  3|5  4|0  4|5  5|0  5|5 60|

  // Default Constants
  const Int_t i_default = -9;
  const Float_t f_default = -99.9;
  const Double_t d_default = -999.99;
  const std::string s_default = "default";

  // Canvas colors
  Int_t k_color_south = Celeste.color; // kCyan-10;
  Int_t k_color_middle = Buff.color; // kYellow-10;
  Int_t k_color_north = Rose.color; // kMagenta-10;

  // Canvas Frames
  Int_t k_frame_nbins = 2150;
  Double_t k_frame_xlo = 0.;
  Double_t k_frame_xhi = 2150.;
  Double_t k_frame_ylo = 0.8; // 0.;
  Double_t k_frame_yhi = 5.e4; 

  // Preliminary Constants
  static const Int_t nLEDs = 5; // # lamps themselves
  const Int_t n_LEDs = 6; // number LED-style files
  static const Int_t nTowers = 48;
  const Int_t k_rebin= 2; // Used to be 20
  const std::string histodir = "/sphenix/data/data02/sphenix/hcal/sectortest/histos/" + data_type;
  const std::string LED_names[nLEDs+1] = {"All(1F)","L0(01)","L1(02)","L2(04)","L3(08)","L4(10)"};

  // LED index
  Int_t led_index_stop = i_default;
  if(hcal_component == "outer") {led_index_stop = n_LEDs;}
  else if(hcal_component == "inner") {led_index_stop = n_LEDs-1;}
  else { throw invalid_argument("Argument Error: voltage_scan_plots: hcal_component not 'inner' or 'outer.' line 169");}

  //Determine voltages
  Int_t nVoltages = 13;
  Float_t voltage_settings[nVoltages];
  const Float_t old_OHC_offset = -2.5;
  if(hcal_component=="outer" && first_run<10000)
    {
      voltage_settings[0]  = -66.00-old_OHC_offset; // -68.50 Volts
      voltage_settings[1]  = -66.25-old_OHC_offset; // -68.75
      voltage_settings[2]  = -66.50-old_OHC_offset; // -69.00
      voltage_settings[3]  = -66.75-old_OHC_offset; // -69.25
      voltage_settings[4]  = -67.00-old_OHC_offset; // -69.50
      voltage_settings[5]  = -67.25-old_OHC_offset; // -69.75
      voltage_settings[6]  = -67.50-old_OHC_offset; // -70.00
      voltage_settings[7]  = -67.75-old_OHC_offset; // -70.25
      voltage_settings[8]  = -68.00-old_OHC_offset; // -70.50
      voltage_settings[9]  = -68.25-old_OHC_offset; // -70.75
      voltage_settings[10] = -68.50-old_OHC_offset; // -71.00
      voltage_settings[11] = -68.75-old_OHC_offset; // -71.25
      voltage_settings[12] = -69.00-old_OHC_offset; // -71.50
    }
  else if(first_run>10000)
    {
      voltage_settings[0]  = -66.00; // Volts
      voltage_settings[1]  = -66.25;
      voltage_settings[2]  = -66.50;
      voltage_settings[3]  = -66.75;
      voltage_settings[4]  = -67.00;
      voltage_settings[5]  = -67.25;
      voltage_settings[6]  = -67.50;
      voltage_settings[7]  = -67.75;
      voltage_settings[8]  = -68.00;
      voltage_settings[9]  = -68.25;
      voltage_settings[10] = -68.50;
      voltage_settings[11] = -68.75;
      voltage_settings[12] = -69.00;
    }
  else
    {
      throw runtime_error{"Runtime Error: run_and_group_analysis_code: voltage settings from invalid run number."};
    }

  //Build file lists
  const Int_t nFiles = last_run-first_run+1;
  Int_t fileno[nFiles]; for(int i=0;i<nFiles;++i){fileno[i]=first_run+i;}
  Double_t d_fileno[nFiles]; for(int i=0;i<nFiles;++i){d_fileno[i]=first_run+i;}

  // Extracting Histograms from Files
  TFile * f_in[nFiles]; for(int i=0;i<nFiles;++i){f_in[i] = NULL;}
  // pad file numbers to 8 digits
  std::string filenostr[nFiles]; for(int i=0;i<nFiles;++i){ filenostr[i] = pad_number(fileno[i],8,'0');}
  // make filename strings
  std::string filestring[nFiles]; for(int i=0;i<nFiles;++i){filestring[i] = Form("%s/%s_%s-0000_out_histos.root",histodir.c_str(),data_type.c_str(),filenostr[i].c_str());}
  // make empty histograms 5files x 48towers
  TH1D * h_ped[nFiles][nTowers]; // pedestal
  TH1D * h_raw[nFiles][nTowers]; // all triggering events
  for(int i=0;i<nFiles;++i){for(int j=0;j<nTowers;++j){h_ped[i][j] = NULL;h_raw[i][j] = NULL;}}

  // Open Files and get histograms
  // loop for files
  for(int i=0;i<nFiles;++i)
    {
      // open file
      f_in[i] = TFile::Open(filestring[i].c_str(),"READ");
      // Exception if not open
      if(!(f_in[i]->TFile::IsOpen())) { throw runtime_error("Runtime Error: run_and_group_analysis_code: ...histos_out.root file not open!"); }

      // loop to get histograms
      for(int j=0;j<nTowers;++j)
	{
	  //	  std::cout << Form("OF:3a,%i,%i:Get",i,j) << std::endl; 
	  // get histograms
	  h_ped[i][j] = (TH1D*)f_in[i]->Get(Form("h_ped_%i",j));
	  h_raw[i][j] = (TH1D*)f_in[i]->Get(Form("h_raw_%i",j));
	  h_ped[i][j]->SetFillStyle(0);
	  h_raw[i][j]->SetFillStyle(0);
	  h_raw[i][j]->SetMarkerColor(Sable.color);

	  // detach histograms from file
	  h_ped[i][j]->SetDirectory(gROOT);
	  h_raw[i][j]->SetDirectory(gROOT);
	} // close loop over towers
      // close file
      f_in[i]->Close();
    }// loop over files
  //std::cout << Form("Histograms Extracted From All Files!") << std::endl; 
  //gROOT->ls();

  // Rebinning
  // std::cout << Form("Rebinning HIstograms by %i",k_rebin) << std::endl; 
  for(int i=0;i<nFiles;++i)
    {
      for(int j=0;j<nTowers;++j)
	{
	  h_ped[i][j]; // pedestal
	  h_raw[i][j]->Rebin(k_rebin); // all triggering events
	}
    }


  // Fitting

  //Results from fit
  Double_t mu[nFiles][nTowers];
  Double_t mu_e[nFiles][nTowers];
  Double_t sigma[nFiles][nTowers];
  Double_t sigma_e[nFiles][nTowers];
  Double_t chisq[nFiles][nTowers];
  Int_t    ndf[nFiles][nTowers];

  Double_t fit_xlo = 0;
  Double_t fit_xhi = 2000;
  TFitResultPtr frp_fit[nFiles][nTowers]; // Don't use *; already a pointer
  Int_t fit_status[nFiles][nTowers];

  for(int i=0;i<nFiles;++i)
    {
      for(int j=0;j<nTowers;++j)
	{
	  mu[i][j]=d_default;
	  mu_e[i][j]=d_default;
	  sigma[i][j]=d_default;
	  sigma_e[i][j]=d_default;
	  chisq[i][j]=d_default;
	  ndf[i][j]=i_default;
	  // don't initialize TFitResultPtrs
	  fit_status[i][j]=i_default;
	}
    }

  std::string option = "SRQ0+";//"SR+";//"SR0+"; //Q
  TF1 *fit = new TF1
    (
     "fit",
     "gaus",
     fit_xlo,
     fit_xhi
     );
  fit->SetLineColor(Azure.color);

  // "Do it!" -- Emperor Palpatine
  //  std::cout << Form("fitting 2") << std::endl; 
  for(int i=0;i<nFiles;++i)
    {
      for(int j=0;j<nTowers;++j)
	{
	  frp_fit[i][j] =  h_raw[i][j]->Fit("fit",option.c_str(),"goff");
	  fit_status[i][j]=frp_fit[i][j];

	  if(fit_status[i][j]==0)
	    {
	      mu[i][j]=frp_fit[i][j]->Parameter(1);
	      mu_e[i][j]=frp_fit[i][j]->ParError(1);
	      sigma[i][j]=frp_fit[i][j]->Parameter(2);
	      sigma_e[i][j]=frp_fit[i][j]->ParError(1);
	      chisq[i][j]=frp_fit[i][j]->Chi2();
	      ndf[i][j]=frp_fit[i][j]->Ndf();

	      // std::cout << Form("Mu(%i,%i): %g +/- %g",i,j,mu[i][j],mu_e[i][j]) << std::endl; 
	      // std::cout << Form("Sg(%i,%i): %g +/- %g",i,j,sigma[i][j],sigma_e[i][j]) << std::endl; 
	      // std::cout << Form("ChiSq/NDF(%i,%i): %g / %d",i,j,chisq[i][j],ndf[i][j]) << std::endl; 

	    }
	}
    }

  // Test Plotting
  TCanvas * c_test_raw[nTowers];
  //  TCanvas * c_test_mu_raw[nTowers];
  //  TCanvas * c_test_sg_raw[nTowers];
  //  TCanvas * c_test_ch_raw[nTowers];
  TGraphErrors * ge_mu_raw[nTowers];
  TGraphErrors * ge_sigma_raw[nTowers];
  TGraph * g_chisq_over_ndf_raw[nTowers];
  double xerr_raw[nFiles]; for(int i=0;i<nFiles;++i){xerr_raw[i] = 0.;}
  double chisq_over_ndf_test[nFiles][nTowers]; for(int i=0;i<nFiles;++i){for(int j=0;j<nTowers;++j){chisq_over_ndf_test[i][j]=d_default;}}; for(int i=0;i<nFiles;++i){for(int j=0;j<nTowers;++j){if(ndf[i][j]!=0){chisq_over_ndf_test[i][j]=chisq[i][j]/ndf[i][j];}}}

  // Make the graphs
  for(int j = 0; j<nTowers;++j)
    {
      ge_mu_raw[j] = new TGraphErrors(nFiles,d_fileno,mu[j],xerr_raw,mu_e[j]);
      ge_mu_raw[j]->SetTitle(Form("Mean (LED Peak-Minus-Pedestal) Sector %s_%s vs Run-Number [%d,%d] Twr %i",hcal_component.c_str(),sector_number.c_str(),first_run,last_run,j));
      ge_mu_raw[j]->GetXaxis()->SetTitle("Run Number");
      ge_mu_raw[j]->GetYaxis()->SetTitle("#mu_{peak-minus-ped} (N_{ADC})");

      ge_sigma_raw[j] = new TGraphErrors(nFiles,d_fileno,sigma[j],xerr_raw,sigma_e[j]);
      ge_sigma_raw[j]->SetTitle(Form("Sigma (LED Peak-Minus-Pedestal) Sector %s_%s vs Run-Number [%d,%d] Twr %i",hcal_component.c_str(),sector_number.c_str(),first_run,last_run,j));
      ge_sigma_raw[j]->GetXaxis()->SetTitle("Run Number");
      ge_sigma_raw[j]->GetYaxis()->SetTitle("#sigma_{peak-minus-ped} (N_{ADC})");

      g_chisq_over_ndf_raw[j] = new TGraphErrors(nFiles,d_fileno,chisq_over_ndf_test[j]);
      g_chisq_over_ndf_raw[j]->SetTitle(Form("#frac{#chi^{2}}{NDF} of Fit (LED Peak-Minus-Pedestal) Sector %s_%s vs Run-Number [%d,%d] Twr %i",hcal_component.c_str(),sector_number.c_str(),first_run,last_run,j));
      g_chisq_over_ndf_raw[j]->GetXaxis()->SetTitle("Run Number");
      g_chisq_over_ndf_raw[j]->GetYaxis()->SetTitle("#frac{#chi^{2}}{NDF} (arb.)");
    }

  // Test Plotting
  // gROOT->SetBatch(1); // don't plot // doesn't work
  for(int j=0;j<nTowers;++j)
    //for(int j=0;j<0;++j) // temporarily, dont plot these
    {
      c_test_raw[j] = new TCanvas
	(
	 Form("c_test_raw_%i",j),
	 Form("c_test_raw_%i",j),
	 500,1500
	 );
      c_test_raw[j]->Divide(1,3);
      c_test_raw[j]->cd(1);
      ge_mu_raw[j]->Draw("AP");
      c_test_raw[j]->cd(2);
      ge_sigma_raw[j]->Draw("AP");
      c_test_raw[j]->cd(3);
      g_chisq_over_ndf_raw[j]->Draw("AP");
      c_test_raw[j]->SaveAs(Form("%s/%s.pdf",plotdir.c_str(),c_test_raw[j]->GetName()));
      //gROOT->SetBatch(0); // go back to plotting // doesn't work
    }

std::cout << Form("TEST") << std::endl; 

  // Test Plotting

  // Interlude: What do we want to look at?

  // 0. Everything:

  // We want to keep track of the run-index (every six runs, the voltage changes to the next setting)
  // ... we have 13 voltage settings: voltage_settings[13] ... so if current_run-first_run%6==0, ++voltage_index
  // ... we have 6 led settings: run_index%6==0 -> All, L0, etc.
  // ... ... and each setting gets 13 mus and sigmas from the overall arrays
  // ... ... ... first, we initialize mu[n_LEDs][nTowers][nVoltageSettings], etc.
  // ... ... ... then we go [run_index = 0, 1, 2, 3, ...]
  // ... ... ... ... then [if run_index%6==0, ++voltage_index, set voltage to voltage_setting[voltage_index] (set v_i=-1)]
  // ... ... ... ... then [led_index=run_index%6 (0, LA; 1, L0; 2, L1; ...)]
  // ... ... ... ... ... then we go [tower_index =- 0, 1, 2, 3, ...]
  // ... ... ... ... ... ... mu[led_index(0-6)][tower_index(0-47)][voltage_index(0-12)] = mu[run_index(0-77)][tower_index(0-47)] // voltage_index last because can leave it off for TGraphError instigation
  // ... ... ... ... ... ... etc.
  // ... we have 78 runs, total, for the 13-setting setup.
  // ... we have 48 towers
  // ... ... so we go [led_index = 0, 1, 2, 3, ...]
  // ... ... ... then we go [tower_index =- 0, 1, 2, 3, ...]
  // ... ... ... ... then ge_mu[led_index][tower_index] = new TGraphErrors(nVoltageSettings,voltage_settings,xerr,mu[led_index][tower_index],mu_e[led_index][tower_index]);

  // const Int_t nVoltages = 13; // above line 165
  //  const Int_t n_LEDs = 6;
  // static const Int_t nTowers = 48;
  Float_t voltage_setting_error[13] = {0.,0.,0.,0.,0.,0.,0.,0.,0.,0.,0.,0.,0.};

  Float_t v_mu[n_LEDs][nTowers][nVoltages];
  Float_t v_mu_e[n_LEDs][nTowers][nVoltages];
  Float_t v_sigma[n_LEDs][nTowers][nVoltages];
  Float_t v_sigma_e[n_LEDs][nTowers][nVoltages];
  Float_t v_chisq[n_LEDs][nTowers][nVoltages];
  Int_t    v_ndf[n_LEDs][nTowers][nVoltages];
  Float_t v_chisq_over_ndf[n_LEDs][nTowers][nVoltages];

  for(int i=0;i<n_LEDs;++i)
    {
      for(int j=0;j<nTowers;++j)
	{
	  for(int k=0;k<nVoltages;++k)
	    {
	      v_mu[n_LEDs][nTowers][nVoltages]=f_default;
	      v_mu_e[n_LEDs][nTowers][nVoltages]=f_default;
	      v_sigma[n_LEDs][nTowers][nVoltages]=f_default;
	      v_sigma_e[n_LEDs][nTowers][nVoltages]=f_default;
	      v_chisq[n_LEDs][nTowers][nVoltages]=f_default;
	      v_ndf[n_LEDs][nTowers][nVoltages]=f_default;
	      v_chisq_over_ndf[n_LEDs][nTowers][nVoltages]=f_default;
	    }
	}
    }

  Int_t voltage_index = -1;
  Int_t led_index = -1;
  Int_t tower_index =-1;

  for(int i=0;i<nFiles;++i)
    {
      if(i%6==0)
	{
	  voltage_index++;
	  //  std::cout << Form("Voltage Index: ") << voltage_index << std::endl; 
	}
      led_index=i%6;
      for(int j=0;j<nTowers;++j)
	{
	  v_mu[led_index][j][voltage_index]=mu[i][j];
	  v_mu_e[led_index][j][voltage_index]=mu_e[i][j];
	  v_sigma[led_index][j][voltage_index]=sigma[i][j];
	  v_sigma_e[led_index][j][voltage_index]=sigma_e[i][j];
	  v_chisq[led_index][j][voltage_index]=chisq[i][j];
	  v_ndf[led_index][j][voltage_index]=ndf[i][j];
	  if(v_ndf[led_index][j][voltage_index]>0){v_chisq_over_ndf[n_LEDs][nTowers][nVoltages]=v_chisq[led_index][j][voltage_index]/v_ndf[led_index][j][voltage_index];}

	  // std::cout << Form("-=-=-=-=-=-=-=-=-=-=-") << std::endl; 
	  // std::cout << Form("LED: (i) %i (%s)",i,LED_names[i%6].c_str()) << std::endl;
	  // std::cout << Form("... led_index = imod6 = %d",led_index) << std::endl; 
	  // std::cout << Form("Twr: (j) %i",j) << std::endl; 
	  // std::cout << Form("Vtg: (voltage_index) %i (%f)",voltage_index,voltage_settings[voltage_index]) << std::endl;
	  // std::cout << Form("... voltage_index++ if imod6 = 0: %d",voltage_index) << std::endl; 
	  // std::cout << Form("Mu: %g +/- %g", v_mu[led_index][j][voltage_index],v_mu_e[led_index][j][voltage_index]) << std::endl;
	  // std::cout << Form("Sigma: %g +/- %g", v_sigma[led_index][j][voltage_index],v_sigma_e[led_index][j][voltage_index]) << std::endl;
	  // std::cout << Form("Chs/Ndf: %g / %d = %g", v_chisq[led_index][j][voltage_index],v_ndf[led_index][j][voltage_index], v_chisq_over_ndf[n_LEDs][nTowers][nVoltages]) << std::endl;

	}
    }
  // Graphs & Histograms
  TGraphErrors * ge_mu[n_LEDs][nTowers];
  TGraphErrors * ge_sigma[n_LEDs][nTowers];
  TGraph * g_chisq_over_ndf[n_LEDs][nTowers];

  TH1F * h1d_p1_whole[n_LEDs]; // histo for results of fits
  // must account for error separately: (1./Sqrt(n_{successful-fits}))*Quadrature(err_{p_{1}})
  TH1F * h1d_p2_whole[n_LEDs]; // histo for results of fits
  // must account for error separately: (1./Sqrt(n_{successful-fits}))*Quadrature(err_{p_{2}})

  Int_t n_bins_fit = 7000; // p1 in range (700,6500) ADCs
  Int_t rebin_fit = 50; // 2 ADC/bin because error is in (10,50) ADCs
  Float_t xlo_fit = 0.;
  Float_t xhi_fit = 7000.;

  TH1F * h1d_p1_north[n_LEDs]; // histo for results of fits
  TH1F * h1d_p2_north[n_LEDs]; // histo for results of fits
  TH1F * h1d_p1_middle[n_LEDs]; // histo for results of fits
  TH1F * h1d_p2_middle[n_LEDs]; // histo for results of fits
  TH1F * h1d_p1_south[n_LEDs]; // histo for results of fits
  TH1F * h1d_p2_south[n_LEDs]; // histo for results of fits

  // Test Plotting
  TCanvas * c_test_whole[n_LEDs][nTowers];
  TCanvas * c_test_north[n_LEDs][nTowers];
  TCanvas * c_test_middle[n_LEDs][nTowers];
  TCanvas * c_test_south[n_LEDs][nTowers];

  // Fit Plots
  TCanvas * c_p1_whole[n_LEDs];
  TCanvas * c_p2_whole[n_LEDs];

  // Fitting stuff
  std::string fit_options = "SQ+";
  TFitResultPtr frp_mu_vs_vol[n_LEDs][nTowers]; // already a ptr
  TFitResultPtr frp_sigma_vs_vol[n_LEDs][nTowers]; // already a ptr
  int fit_status_mu_vs_vol[n_LEDs][nTowers];
  int fit_status_sigma_vs_vol[n_LEDs][nTowers];
  TPaveText * pt_mu_fit_result[n_LEDs][nTowers];
  TPaveText * pt_sigma_fit_result[n_LEDs][nTowers];
  // Test Plotting

  // container for values errors
  Float_t n_successful_fits_per_lamp = f_default;
  Float_t p1_err_fit_quad_sum = f_default;
  Float_t p2_err_fit_quad_sum = f_default;

  for(int i=0;i<led_index_stop;++i)
    {
      h1d_p1_whole[i] = new TH1F
	(
	 Form
	 (
	  "h1d_p1_whole_%i",
	  i
	  ),
	 Form
	 (
	  "linear coeff of fit (N_{ADC}/V) for LED %s (%i)",
	  LED_names[i].c_str(),
	  i
	  ),
	 n_bins_fit,
	 xlo_fit,
	 xhi_fit
	 );
      h1d_p1_whole[i]->GetYaxis()->SetTitle(Form("Linear Coefficient Value (N_{ADC}/V)"));
      h1d_p1_whole[i]->GetYaxis()->SetTitle(Form("N_{TWR}"));
      h1d_p2_whole[i] = new TH1F
	(
	 Form
	 (
	  "h1d_p2_whole_%i",
	  i
	  ),
	 Form
	 (
	  "quadratic coeff of fit (N_{ADC}/V^{2}) for LED %s %i",
	  LED_names[i].c_str(),
	  i
	  ),
	 n_bins_fit,
	 xlo_fit,
	 xhi_fit
	 );
      h1d_p2_whole[i]->GetYaxis()->SetTitle(Form("Quadratic Coefficient Value (N_{ADC}/V^{2})"));
      h1d_p2_whole[i]->GetYaxis()->SetTitle(Form("N_{TWR}"));
; // histo for results of fits

      n_successful_fits_per_lamp = 0.;
      p1_err_fit_quad_sum = 0.;
      p2_err_fit_quad_sum = 0.;
      for(int j=0;j<nTowers;++j)
	{

	  ge_mu[i][j] = new TGraphErrors(nVoltages,voltage_settings,v_mu[i][j],voltage_setting_error,v_mu_e[i][j]);
	  ge_mu[i][j]->SetTitle(Form("#mu_{peak-minus-ped} %i%s Twr%i",i,LED_names[i].c_str(),j));
	  ge_mu[i][j]->GetXaxis()->SetTitle(Form("Bias Voltage (V)"));
	  ge_mu[i][j]->GetYaxis()->SetTitle(Form("#mu_{peak-minus-ped} (N_{ADC})"));

	  ge_sigma[i][j] = new TGraphErrors(nVoltages,voltage_settings,v_sigma[i][j],voltage_setting_error,v_sigma_e[i][j]);
	  ge_sigma[i][j]->SetTitle(Form("#sigma_{peak-minus-ped} %i%s Twr%i",i,LED_names[i].c_str(),j));
	  ge_sigma[i][j]->GetXaxis()->SetTitle(Form("Bias Voltage (V)"));
	  ge_sigma[i][j]->GetYaxis()->SetTitle(Form("#sigma_{peak-minus-ped} (N_{ADC})"));

	  g_chisq_over_ndf[i][j] = new TGraph(nVoltages,voltage_settings,v_chisq_over_ndf[i][j]);
	  g_chisq_over_ndf[i][j]->SetTitle(Form("#frac{#chi^{2}}{ndf}_{peak-minus-ped} %i%s_Twr%i",i,LED_names[i].c_str(),j));
	  g_chisq_over_ndf[i][j]->GetXaxis()->SetTitle(Form("Bias Voltage (V)"));
	  g_chisq_over_ndf[i][j]->GetYaxis()->SetTitle(Form("#frac{chi^2}{ndf} (arb.)"));

	  //fiting
	  frp_mu_vs_vol[i][j] = ge_mu[i][j]->Fit("pol2",fit_options.c_str());
	  fit_status_mu_vs_vol[i][j] = frp_mu_vs_vol[i][j];

	  p1_err_fit_quad_sum = f_default;
	  p2_err_fit_quad_sum = f_default;

	  if(fit_status_mu_vs_vol[i][j]==0)
	    {
	      // Fill histograms
	      n_successful_fits_per_lamp++;
	      h1d_p1_whole[i]->Fill(frp_mu_vs_vol[i][j]->Parameter(1));
	      p1_err_fit_quad_sum = (frp_mu_vs_vol[i][j]->ParError(1))*(frp_mu_vs_vol[i][j]->ParError(1));
	      h1d_p2_whole[i]->Fill(frp_mu_vs_vol[i][j]->Parameter(2));
	      p2_err_fit_quad_sum = (frp_mu_vs_vol[i][j]->ParError(2))*(frp_mu_vs_vol[i][j]->ParError(2));

	      // Make TPaveText for Parameters on Plots
	      pt_mu_fit_result[i][j] = new TPaveText(0.5,0.5,0.9,0.9,"brNDC");
	      pt_mu_fit_result[i][j]->AddText(Form("Fit Result"));
	      pt_mu_fit_result[i][j]->AddText(Form("y(x) = p0 + p1 x + p2 x^{2}"));
	      pt_mu_fit_result[i][j]->AddText
		(
		 Form
		 (
		  "p0 = %g #pm %g N_{ADC}",
		  frp_mu_vs_vol[i][j]->Parameter(0),
		  frp_mu_vs_vol[i][j]->ParError(0)
		  )
		 );
	      pt_mu_fit_result[i][j]->AddText
		(
		 Form
		 (
		  "p1 = %g #pm %g N_{ADC}/V",
		  frp_mu_vs_vol[i][j]->Parameter(1),
		  frp_mu_vs_vol[i][j]->ParError(1)
		  )
		 );
	      pt_mu_fit_result[i][j]->AddText
		(
		 Form
		 (
		  "p2 = %g #pm %g N_{ADC}/V^{2}",
		  frp_mu_vs_vol[i][j]->Parameter(2),
		  frp_mu_vs_vol[i][j]->ParError(2)
		  )
		 );

	      pt_mu_fit_result[i][j]->AddText(Form("#chi^{2} = %g",
						   frp_mu_vs_vol[i][j]->Chi2()));
	      pt_mu_fit_result[i][j]->AddText(Form("ndf = %d",
						   frp_mu_vs_vol[i][j]->Ndf()));
	    } // closes if fit-status-mu is good

	    frp_sigma_vs_vol[i][j] = ge_sigma[i][j]->Fit("pol2",fit_options.c_str());
	    fit_status_sigma_vs_vol[i][j] = frp_sigma_vs_vol[i][j];

	    if(fit_status_sigma_vs_vol[i][j]==0)
	      {
		pt_sigma_fit_result[i][j] = new TPaveText(0.5,0.5,0.9,0.9,"brNDC");
		pt_sigma_fit_result[i][j]->AddText(Form("Fit Result"));
		pt_sigma_fit_result[i][j]->AddText(Form("y(x) = p0 + p1 x + p2 x^{2}"));
		pt_sigma_fit_result[i][j]->AddText
		  (
		   Form
		   (
		    "p0 = %g #pm %g N_{ADC}",
		    frp_sigma_vs_vol[i][j]->Parameter(0),
		    frp_sigma_vs_vol[i][j]->ParError(0)
		    )
		   );
		pt_sigma_fit_result[i][j]->AddText
		  (
		   Form
		   (
		    "p1 = %g #pm %g N_{ADC}/V",
		    frp_sigma_vs_vol[i][j]->Parameter(1),
		    frp_sigma_vs_vol[i][j]->ParError(1)
		    )
		   );
		pt_sigma_fit_result[i][j]->AddText
		  (
		   Form
		   (
		    "p2 = %g #pm %g N_{ADC}/V^{2}",
		    frp_sigma_vs_vol[i][j]->Parameter(2),
		    frp_sigma_vs_vol[i][j]->ParError(2)
		    )
		   );

		pt_sigma_fit_result[i][j]->AddText(Form("#chi^{2} = %g",
						     frp_sigma_vs_vol[i][j]->Chi2()));
		pt_sigma_fit_result[i][j]->AddText(Form("ndf = %d",
						     frp_sigma_vs_vol[i][j]->Ndf()));
	      } // closes if-fit-status-sigma is good

	  // Test Plotting
	    c_test_whole[i][j] = new TCanvas
	      (
	       Form
	       (
		"c_test_whole_%i_%i",
		i,
		j
		),
	       Form
	       (
		"c_test_whole_%i_%i: Sector %s%s Twr %i LED %s (Runs [%d,%d])",
		i, // %i
		j, // %i
		hcal_component.c_str(), // %s
		sector_number.c_str(), // %s
		j, // %i
		LED_names[i].c_str(), //%s
		first_run, //%d
		last_run //%d
		),
	       500,
	       1500
	       );
	    //	    c_test_whole[i][j]->Divide(1,3);
	    c_test_whole[i][j]->Divide(1,2);
	    c_test_whole[i][j]->cd(1);

	    ge_mu[i][j]->Draw("AP");

	    if(fit_status_mu_vs_vol[i][j]==0)
	      {
		pt_mu_fit_result[i][j]->Draw("SAME");
		c_test_whole[i][j]->Update();
	      }
	    c_test_whole[i][j]->cd(2);
	    ge_sigma[i][j]->Draw("AP");

	    if(fit_status_sigma_vs_vol[i][j]==0)
	      {
		pt_sigma_fit_result[i][j]->Draw("SAME");
		c_test_whole[i][j]->Update();
	      }
	    // c_test_whole[i][j]->cd(3);
	    // std::cout << Form("5") << std::endl; 
	    // g_chisq_over_ndf[i][j]->Draw("AP");
	    // std::cout << Form("6") << std::endl; 

	    //c_test_whole[i][j]->SaveAs(Form("%s/%s.pdf",plotdir.c_str(),c_test_whole[i][j]->GetName()));
	    c_test_whole[i][j]->SaveAs(Form("%s/%s.pdf",plotdir.c_str(),c_test_whole[j]->GetName()));
	} // closes loop over towers

      //fill histograms & calculate error
      p1_err_fit_quad_sum = (1./sqrt(n_successful_fits_per_lamp))*sqrt(p1_err_fit_quad_sum);
      h1d_p1_whole[i]->Rebin(rebin_fit);
      for(int k=1;k<h1d_p1_whole[i]->GetNbinsX();++k)
	{
	  h1d_p1_whole[i]->SetBinError(k,p1_err_fit_quad_sum);
	}
      p2_err_fit_quad_sum = (1./sqrt(n_successful_fits_per_lamp))*sqrt(p2_err_fit_quad_sum);
      h1d_p2_whole[i]->Rebin(rebin_fit);
      for(int k=1;k<h1d_p2_whole[i]->GetNbinsX();++k)
	{
	  h1d_p2_whole[i]->SetBinError(k,p2_err_fit_quad_sum);
	}

      c_p1_whole[i] = new TCanvas(Form("c_p1_whole_%i",i),Form("c_p1_whole_%i",i),1);
      h1d_p1_whole[i]->Draw("E");
      //h1d_p1_whole[i]->SaveAs(Form("%s/%s.pdf",plotdir.c_str(),h1d_p1_whole[i]->GetName()));
      c_p1_whole[i]->SaveAs(Form("%s/%s.pdf",plotdir.c_str(),c_p1_whole[j]->GetName()));


      c_p2_whole[i] = new TCanvas(Form("c_p2_whole_%i",i),Form("c_p2_whole_%i",i),1);
      h1d_p2_whole[i]->Draw("E");
      //h1d_p2_whole[i]->SaveAs(Form("%s/%s.pdf",plotdir.c_str(),h1d_p2_whole[i]->GetName()));
      c_p2_whole[i]->SaveAs(Form("%s/%s.pdf",plotdir.c_str(),c_p2_whole[j]->GetName()));
	  // temp correction

	  // PR correction

	
    }





  // TEst Plotting

  // I. For each sector, separately:

  // We want to look at mean & mean err as function of voltage for each tile-set in each tower
  // ... so one plot per tile-set, composed of the usual 4x12 tower plot, showing MPV on the y-axis (plus error) and V on the x-axis
  // ... we also want to fit that line and put the fit reesult (but not the fit-line) on the plot for each tower
  //
  // So, how shall we do this?
  // ... need two TGraphErrors for each tower: mu, sigma vs voltage
  // ... need one vertical line: Vop, used by each tower
  // ... need a TGraph for each tower: chisq/ndf vs voltage (diagnostic)
  // ... need two TF1 sfor each tower: mu, sigma vs voltage
  // ... need two TFitResultPtrs & integer status: mu, sigma vs voltage
  // ... neet one TPaveText for each tower: y=mx+b's m, b, chisq/ndf

  // II. For each sector, separately:

  // We want to look at average Mean vs voltage for North/Middle/South
  // ... so same above, just average the means and propagate their errors
  // We want to look at average Sigma vs voltage for North/Middle/South
  // ... so same above, just average the sigmas and propagate their errors
  // We want to look at whole-sector average mean vs voltage
  // ... so same above, just average the means and propagate their errors
  // We want to look at whole-sector average Sigma vs voltage h
  // ... so same above, just average the sigmas and propagate their errors

  // III. All sectors together, incl. IHCal & OHCal:

  // A. Verify 50 mV = 2.5%
  // ... We want to get that slope for all towers
  // ... We want to get that slope for all thirds
  // ... We want to get that slope for each sector
  // ... ... I expect that the slope vs sector plot will be flat,
  //         because the voltage should change the response of the
  //         tile-sipm in proportion to its value

  // B. Check Hanpu's Plot
  // ... Plot all means and sigmas together for each tower
  // ... Plot all averages of means and sigmas for each third
  // ... ... take ratio of the similar-VOP thirds
  //         (and propagate err)
  // ... Plot ratios of average means and sigmas for similar-VOP
  //     thirds (and propage error) (TLine&TBox below ...)
  // ... Plot all average of means and sigmas for each sector
  // ... ... take ratio of the similar-VOP  sectors
  //         (and propagate err)
  // ... Plot ratios of average means and sigmas for similar-VOP
  //     sectors(and propagate error) (TLine&TBox below ...)
  //
  // ... extract ratio of the sectors from Hanpu's plot and put a
  //     TLine in a TBox as a constant if the data is availible
  //
  // ... ... I expect (or hope?) that the ratio should match the
  //         ratio in Hanpu's plot, because the tile-SiPM response
  //         should not differ, tower-by-tower, between LED and
  //         cosmics, since the tile's response is the same
  //         physics mechanism (E+M)

  /* // (l. 591)

// Whole Detector & Single Runs
 std::cout << Form("Plot:1:canvas_init") << std::endl; 
 //TCanvas *c_individ[nFiles][nTowers];
 TCanvas *c_big[nFiles];
 // TLegend *l_individ[nFiles][nTowers];for(int i=0;i<nTowers;++i){for(int j=0;j<nTowers;++j){l_individ[i][j]=NULL;}}
 TH1D * h_frame[nFiles][nTowers];
 for(int i=0;i<nFiles;++i)
   {
     for(int j=0;j<nTowers;++j)
       {
	 h_frame[i][j]= new TH1D("h_frame","h_frame",k_frame_nbins,k_frame_xlo,k_frame_xhi);
	 h_frame[i][j]->GetYaxis()->SetRangeUser(k_frame_ylo,k_frame_yhi);
       }
   }
 std::cout << Form("Plot:2:loop") << std::endl; 
for(int i=0;i<nFiles;++i)
  {
    std::cout << Form("Plot:2-%i-0:file_loop",i) << std::endl; 
    c_big[i] = new TCanvas(Form("c_big_%i",i),Form("c_big_%i",fileno[i]),1);
    std::cout << Form("Plot:2-%i-1:manipulate and color canvas",i) << std::endl;
    c_big[i]->Divide(12,4,0,0);
    c_big[i]->cd(1)->SetFillColorAlpha(k_color_south,0.5);
    c_big[i]->cd(2)->SetFillColorAlpha(k_color_south,0.5);
    c_big[i]->cd(3)->SetFillColorAlpha(k_color_south,0.5);
    c_big[i]->cd(4)->SetFillColorAlpha(k_color_south,0.5);
    c_big[i]->cd(5)->SetFillColorAlpha(k_color_south,0.5);
    c_big[i]->cd(6)->SetFillColorAlpha(k_color_south,0.5);
    c_big[i]->cd(7)->SetFillColorAlpha(k_color_south,0.5);
    c_big[i]->cd(8)->SetFillColorAlpha(k_color_south,0.5);
    c_big[i]->cd(13)->SetFillColorAlpha(k_color_south,0.5);
    c_big[i]->cd(14)->SetFillColorAlpha(k_color_south,0.5);
    c_big[i]->cd(15)->SetFillColorAlpha(k_color_south,0.5);
    c_big[i]->cd(16)->SetFillColorAlpha(k_color_south,0.5);
    c_big[i]->cd(17)->SetFillColorAlpha(k_color_south,0.5);
    c_big[i]->cd(18)->SetFillColorAlpha(k_color_south,0.5);
    c_big[i]->cd(19)->SetFillColorAlpha(k_color_south,0.5);
    c_big[i]->cd(20)->SetFillColorAlpha(k_color_south,0.5);
    c_big[i]->cd(9)->SetFillColorAlpha(k_color_middle,0.5);
    c_big[i]->cd(10)->SetFillColorAlpha(k_color_middle,0.5);
    c_big[i]->cd(11)->SetFillColorAlpha(k_color_middle,0.5);
    c_big[i]->cd(12)->SetFillColorAlpha(k_color_middle,0.5);
    c_big[i]->cd(21)->SetFillColorAlpha(k_color_middle,0.5);
    c_big[i]->cd(22)->SetFillColorAlpha(k_color_middle,0.5);
    c_big[i]->cd(23)->SetFillColorAlpha(k_color_middle,0.5);
    c_big[i]->cd(24)->SetFillColorAlpha(k_color_middle,0.5);
    c_big[i]->cd(25)->SetFillColorAlpha(k_color_middle,0.5);
    c_big[i]->cd(26)->SetFillColorAlpha(k_color_middle,0.5);
    c_big[i]->cd(27)->SetFillColorAlpha(k_color_middle,0.5);
    c_big[i]->cd(28)->SetFillColorAlpha(k_color_middle,0.5);
    c_big[i]->cd(37)->SetFillColorAlpha(k_color_middle,0.5);
    c_big[i]->cd(38)->SetFillColorAlpha(k_color_middle,0.5);
    c_big[i]->cd(39)->SetFillColorAlpha(k_color_middle,0.5);
    c_big[i]->cd(40)->SetFillColorAlpha(k_color_middle,0.5);
    c_big[i]->cd(29)->SetFillColorAlpha(k_color_north,0.5);
    c_big[i]->cd(30)->SetFillColorAlpha(k_color_north,0.5);
    c_big[i]->cd(31)->SetFillColorAlpha(k_color_north,0.5);
    c_big[i]->cd(32)->SetFillColorAlpha(k_color_north,0.5);
    c_big[i]->cd(33)->SetFillColorAlpha(k_color_north,0.5);
    c_big[i]->cd(34)->SetFillColorAlpha(k_color_north,0.5);
    c_big[i]->cd(35)->SetFillColorAlpha(k_color_north,0.5);
    c_big[i]->cd(36)->SetFillColorAlpha(k_color_north,0.5);
    c_big[i]->cd(41)->SetFillColorAlpha(k_color_north,0.5);
    c_big[i]->cd(42)->SetFillColorAlpha(k_color_north,0.5);
    c_big[i]->cd(43)->SetFillColorAlpha(k_color_north,0.5);
    c_big[i]->cd(44)->SetFillColorAlpha(k_color_north,0.5);
    c_big[i]->cd(45)->SetFillColorAlpha(k_color_north,0.5);
    c_big[i]->cd(46)->SetFillColorAlpha(k_color_north,0.5);
    c_big[i]->cd(47)->SetFillColorAlpha(k_color_north,0.5);
    c_big[i]->cd(48)->SetFillColorAlpha(k_color_north,0.5);

    for(int j=0;j<nTowers;++j)
      {
	std::cout << Form("Filling canvases: %i",j) << std::endl;
	c_big[i]->cd(mapcanvas(j));
	std::cout << Form("a") << std::endl; 
	gPad->SetLogy();
	h_frame[i][j]->SetTitle(Form("Run %i: %s %s: %s DAC@%s",fileno[i],hcal_component.c_str(),nameHistograms(j,sector_number,hcal_component).c_str(),LED_names[i%6].c_str(),LED_voltage_settings[i%8].c_str())); // re-casts each plot to have name of lst-plotted histogram
	std::cout << Form("b") << std::endl; 
	h_frame[i][j]->SetStats(0);
	h_frame[i][j]->Draw();
	std::cout << Form("c") << std::endl; 
	h_ped[i][j]->Draw("SAME");
	std::cout << Form("c2") << std::endl; 
	h_raw[i][j]->Draw("SAMEE");
      } // close loop over towers
  }// close loop over files

// Save Plot
 for(int i=0;i<nFiles;++i)
   {
     c_big[i]->SaveAs(Form("%s/c_big_%i.pdf",plotdir.c_str(),fileno[i]));
   }

 // Write-Out Fit Parameters to file
 // Should have structure like this:
 // [file] << file_number << LED_intensity_setting << LED_name << tower_number << tower_string << mu << mu_err << sigma << sigma_err << fit_chisquare << fit_ndf

 // Another script will read in that file and plot the numbers (use C++/ROOT and use TNtuple/TTree)

 */ // (l. 491)

} // end of main function

// Utility functions
std::string pad_number
(
 int number,
 int n_pads,
 char padcharacter
 )
{
  std::ostringstream ss;
  ss << std::setw(n_pads) << std::setfill(padcharacter) << number;
  return ss.str();
}

Int_t mapcanvas(
		Int_t pre_amp_tower_number
		)
{
  Int_t canvas_number = 0;

  Int_t map_canvas[48]; // 48
  // SOUTH THIRD
  map_canvas[0] = 2; // 1;
  map_canvas[1] = 14; // 13;
  map_canvas[2] = 1; // 2;
  map_canvas[3] = 13; // 14;

  map_canvas[4] = 4; // 3;
  map_canvas[5] = 16; // 15;
  map_canvas[6] = 3; // 4;
  map_canvas[7] = 15; // 16;

  map_canvas[8] = 6; // 5;
  map_canvas[9] = 18; // 17;
  map_canvas[10] = 5; // 6;
  map_canvas[11] = 17; // 18;

  map_canvas[12] = 8; // 7;
  map_canvas[13] = 20; // 19;
  map_canvas[14] = 7; // 8;
  map_canvas[15] = 19; // 20;
  // MIDDLE THIRD
  map_canvas[16] = 10; //9;  // 16 // 64
  map_canvas[17] = 22; //21; // 17 // 65
  map_canvas[18] = 9; //10; // 18 // 66
  map_canvas[19] = 21; //22; // 19 // 67

  map_canvas[20] = 12; //11; // 20 // 68
  map_canvas[21] = 24; //23; // 21 // 69
  map_canvas[22] = 11; //12; // 22 // 70
  map_canvas[23] = 23; //24; // 23 // 71

  map_canvas[24] = 26; //25; // 24 // 72
  map_canvas[25] = 38; //37; // 25 // 73
  map_canvas[26] = 25; //26; // 26 // 74
  map_canvas[27] = 37; //38; // 27 // 75

  map_canvas[28] = 28; //27; // 28 // 76
  map_canvas[29] = 40; //39; // 29 // 77
  map_canvas[30] = 27; //28; // 30 // 78
  map_canvas[31] = 39; //40; // 31 // 79
  // NORTH THIRD
  map_canvas[32] = 30; //29; // 32 // 128
  map_canvas[33] = 42; //41; // 33 // 129
  map_canvas[34] = 29; //30; // 34 // 130
  map_canvas[35] = 41; //42; // 35 // 131

  map_canvas[36] = 32; //31; // 36 // 132
  map_canvas[37] = 44; //43; // 37 // 133
  map_canvas[38] = 31; //32; // 38 // 134
  map_canvas[39] = 43; //44; // 39 // 135

  map_canvas[40] = 34; //33; // 40 // 136
  map_canvas[41] = 46; //45; // 41 // 137
  map_canvas[42] = 33; //34; // 42 // 138
  map_canvas[43] = 45; //46; // 43 // 139

  map_canvas[44] = 36; //35; // 44 // 140
  map_canvas[45] = 48; //47; // 45 // 141
  map_canvas[46] = 35; //36; // 46 // 142
  map_canvas[47] = 47; //48; // 47 // 143

  canvas_number = map_canvas[pre_amp_tower_number];
  return canvas_number;
}

std::string nameHistograms(int index = 0, std::string sectornumber="0", std::string hcal_component="outer")
{
  // histo Names
  std::string hname[112];
  // const int s=0;  // south  third: min ADC No.
  // const int m=48; // middle third: min ADC No.
  // const int n=96;// north  third: min ADC No.
  const int s = 0; // other south histo conversion
  const int m = 16; // other middle histo conversion
  const int n = 32; // other north histo conv
  for(int i=0;i<112;++i){hname[i]="empty";}

  if(hcal_component=="inner")
    {
      hname[s+0]  = "S-Z11-U"; // ADC 0   // Pre-amp 2
      hname[s+1]  = "S-Z11-L"; // ADC 1   // Pre-amp 3
      hname[s+2]  = "S-Z12-U"; // ADC 2   // Pre-amp 0
      hname[s+3]  = "S-Z12-L"; // ADC 3   // Pre-amp 1
      hname[s+4]  = "S-Z09-U"; // ADC 4   // Pre-amp 6
      hname[s+5]  = "S-Z09-L"; // ADC 5   // Pre-amp 7
      hname[s+6]  = "S-Z10-U"; // ADC 6   // Pre-amp 4
      hname[s+7]  = "S-Z10-L"; // ADC 7   // Pre-amp 5
      hname[s+8]  = "S-Z07-U"; // ADC 8   // Pre-amp 10
      hname[s+9]  = "S-Z07-L"; // ADC 9   // Pre-amp 11
      hname[s+10] = "S-Z08-U"; // ADC 10  // Pre-amp 8
      hname[s+11] = "S-Z08-L"; // ADC 11  // Pre-amp 9
      hname[s+12] = "S-Z05-U"; // ADC 12  // Pre-amp 14
      hname[s+13] = "S-Z05-L"; // ADC 13  // Pre-amp 15
      hname[s+14] = "S-Z06-U"; // ADC 14  // Pre-amp 12
      hname[s+15] = "S-Z06-L"; // ADC 15  // Pre-amp 13
      hname[m+0]  = "S-Z03-U"; // ADC 64  // Pre-amp 18
      hname[m+1]  = "S-Z03-L"; // ADC 65  // Pre-amp 19
      hname[m+2]  = "S-Z04-U"; // ADC 66  // Pre-amp 16
      hname[m+3]  = "S-Z04-L"; // ADC 67  // Pre-amp 17
      hname[m+4]  = "S-Z01-U"; // ADC 68  // Pre-amp 22
      hname[m+5]  = "S-Z01-L"; // ADC 69  // Pre-amp 23
      hname[m+6]  = "S-Z02-U"; // ADC 70  // Pre-amp 20
      hname[m+7]  = "S-Z02-L"; // ADC 71  // Pre-amp 21
      hname[m+8]  = "N-Z02-U"; // ADC 72  // Pre-amp 26
      hname[m+9]  = "N-Z02-L"; // ADC 73  // Pre-amp 27
      hname[m+10] = "N-Z01-U"; // ADC 74  // Pre-amp 24
      hname[m+11] = "N-Z01-L"; // ADC 75  // Pre-amp 25
      hname[m+12] = "N-Z04-U"; // ADC 76  // Pre-amp 30
      hname[m+13] = "N-Z04-L"; // ADC 77  // Pre-amp 31
      hname[m+14] = "N-Z03-U"; // ADC 78  // Pre-amp 28
      hname[m+15] = "N-Z03-L"; // ADC 79  // Pre-amp 29
      hname[n+0]  = "N-Z06-U"; // ADC 128 // Pre-amp 34
      hname[n+1]  = "N-Z06-L"; // ADC 129 // Pre-amp 35
      hname[n+2]  = "N-Z05-U"; // ADC 130 // Pre-amp 32
      hname[n+3]  = "N-Z05-L"; // ADC 131 // Pre-amp 33
      hname[n+4]  = "N-Z08-U"; // ADC 132 // Pre-amp 38
      hname[n+5]  = "N-Z08-L"; // ADC 133 // Pre-amp 39
      hname[n+6]  = "N-Z07-U"; // ADC 134 // Pre-amp 36
      hname[n+7]  = "N-Z07-L"; // ADC 135 // Pre-amp 37
      hname[n+8]  = "N-Z10-U"; // ADC 136 // Pre-amp 42
      hname[n+9]  = "N-Z10-L"; // ADC 137 // Pre-amp 43
      hname[n+10] = "N-Z09-U"; // ADC 138 // Pre-amp 40
      hname[n+11] = "N-Z09-L"; // ADC 139 // Pre-amp 41
      hname[n+12] = "N-Z12-U"; // ADC 140 // Pre-amp 46
      hname[n+13] = "N-Z12-L"; // ADC 141 // Pre-amp 47
      hname[n+14] = "N-Z11-U"; // ADC 142 // Pre-amp 44
      hname[n+15] = "N-Z11-L"; // ADC 143 // Pre-amp 45
    }
  else if(hcal_component=="outer")
    {
      const int s = 0; // other south histo conversion
      const int m = 16; // other middle histo conversion
      const int n = 32; // other north histo conv
      for(int i=0;i<112;++i){hname[i]="empty";}
      if(
	 sectornumber=="30" ||
	 sectornumber=="31" ||
	 sectornumber=="32"
	 )
	{
	  hname[s+0]  = "S-M31-U"; // ADC 0   // Pre-amp 2
	  hname[s+1]  = "S-M31-L"; // ADC 1   // Pre-amp 3
	  hname[s+2]  = "S-M32-U"; // ADC 2   // Pre-amp 0
	  hname[s+3]  = "S-M32-L"; // ADC 3   // Pre-amp 1
	  hname[s+4]  = "S-M29-U"; // ADC 4   // Pre-amp 6
	  hname[s+5]  = "S-M29-L"; // ADC 5   // Pre-amp 7
	  hname[s+6]  = "S-M30-U"; // ADC 6   // Pre-amp 4
	  hname[s+7]  = "S-M30-L"; // ADC 7   // Pre-amp 5
	}
      else
	{
	  hname[s+0]  = "S-B31-U"; // ADC 0   // Pre-amp 2
	  hname[s+1]  = "S-B31-L"; // ADC 1   // Pre-amp 3
	  hname[s+2]  = "S-B32-U"; // ADC 2   // Pre-amp 0
	  hname[s+3]  = "S-B32-L"; // ADC 3   // Pre-amp 1
	  hname[s+4]  = "S-B29-U"; // ADC 4   // Pre-amp 6
	  hname[s+5]  = "S-B29-L"; // ADC 5   // Pre-amp 7
	  hname[s+6]  = "S-B30-U"; // ADC 6   // Pre-amp 4
	  hname[s+7]  = "S-B30-L"; // ADC 7   // Pre-amp 5
	}
      hname[s+8]  = "S-B27-U"; // ADC 8   // Pre-amp 10
      hname[s+9]  = "S-B27-L"; // ADC 9   // Pre-amp 11
      hname[s+10] = "S-B28-U"; // ADC 10  // Pre-amp 8
      hname[s+11] = "S-B28-L"; // ADC 11  // Pre-amp 9
      hname[s+12] = "S-B25-U"; // ADC 12  // Pre-amp 14
      hname[s+13] = "S-B25-L"; // ADC 13  // Pre-amp 15
      hname[s+14] = "S-B26-U"; // ADC 14  // Pre-amp 12
      hname[s+15] = "S-B26-L"; // ADC 15  // Pre-amp 13
      hname[m+0]  = "S-B23-U"; // ADC 64  // Pre-amp 18
      hname[m+1]  = "S-B23-L"; // ADC 65  // Pre-amp 19
      hname[m+2]  = "S-B24-U"; // ADC 66  // Pre-amp 16
      hname[m+3]  = "S-B24-L"; // ADC 67  // Pre-amp 17
      hname[m+4]  = "S-B21-U"; // ADC 68  // Pre-amp 22
      hname[m+5]  = "S-B21-L"; // ADC 69  // Pre-amp 23
      hname[m+6]  = "S-B22-U"; // ADC 70  // Pre-amp 20
      hname[m+7]  = "S-B22-L"; // ADC 71  // Pre-amp 21
      hname[m+8]  = "N-B22-U"; // ADC 72  // Pre-amp 26
      hname[m+9]  = "N-B22-L"; // ADC 73  // Pre-amp 27
      hname[m+10] = "N-B21-U"; // ADC 74  // Pre-amp 24
      hname[m+11] = "N-B21-L"; // ADC 75  // Pre-amp 25
      hname[m+12] = "N-B24-U"; // ADC 76  // Pre-amp 30
      hname[m+13] = "N-B24-L"; // ADC 77  // Pre-amp 31
      hname[m+14] = "N-B23-U"; // ADC 78  // Pre-amp 28
      hname[m+15] = "N-B23-L"; // ADC 79  // Pre-amp 29
      hname[n+0]  = "N-B26-U"; // ADC 128 // Pre-amp 34
      hname[n+1]  = "N-B26-L"; // ADC 129 // Pre-amp 35
      hname[n+2]  = "N-B25-U"; // ADC 130 // Pre-amp 32
      hname[n+3]  = "N-B25-L"; // ADC 131 // Pre-amp 33
      hname[n+4]  = "N-B28-U"; // ADC 132 // Pre-amp 38
      hname[n+5]  = "N-B28-L"; // ADC 133 // Pre-amp 39
      hname[n+6]  = "N-B27-U"; // ADC 134 // Pre-amp 36
      hname[n+7]  = "N-B27-L"; // ADC 135 // Pre-amp 37
      hname[n+8]  = "N-B30-U"; // ADC 136 // Pre-amp 42
      hname[n+9]  = "N-B30-L"; // ADC 137 // Pre-amp 43
      hname[n+10] = "N-B29-U"; // ADC 138 // Pre-amp 40
      hname[n+11] = "N-B29-L"; // ADC 139 // Pre-amp 41
      hname[n+12] = "N-B32-U"; // ADC 140 // Pre-amp 46
      hname[n+13] = "N-B32-L"; // ADC 141 // Pre-amp 47
      hname[n+14] = "N-B31-U"; // ADC 142 // Pre-amp 44
      hname[n+15] = "N-B31-L"; // ADC 143 // Pre-amp 45
    }
  else
    {
      throw runtime_error("Tower ID Error");
    }
  return sectornumber+"-"+hname[index];
}

Int_t Which_LED
(
 Int_t run_number,
 Int_t LEDAll_run
 )
{
  // The Cycle is always LA-L0-L1-L2-L3-L4;
  // so, if we use the lowest run-number as
  // the modulus, then doing run-number mod
  // first-run-number, we'll get the right
  // LED. However, after going 6 runs, we
  // have to update the lowest run-number
  // considered. LEDAll runs are index-mod-
  // 6, meaning that there are 8 of them for
  // the whole scan.

  return run_number % LEDAll_run;
}
