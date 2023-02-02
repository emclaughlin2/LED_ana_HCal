
#include <iostream>
#include <pmonitor/pmonitor.h>
#include "saHCal.h"

#include <TH1.h>
#include <TH2.h>
#include <TF1.h>
#include <TMath.h>
#include <iostream>
#include <string>
#include <fstream>
#include <sstream> 


int init_done = 0;
int fit_init_done = 0;

using namespace std;


int threshold = 0;
int nevt = 0;

const int numPacket  = 8;
const int numChannel = 192;

TH1F *hsig[numPacket][numChannel];
double mean[numPacket][numChannel]={0.0};
double x2[numPacket][numChannel]={0.0};

// need to improve signal extraction but this is a fine place holder
double getSignal(Packet *p, const int channel)
{
  double baseline = 0;
  for ( int s = 0;  s< 3; s++) {
      baseline += p->iValue(s,channel);
    }
  baseline /= 3.;

  double signal = 0;
  float x = 0;
  for ( int s = 3;  s< p->iValue(0,"SAMPLES"); s++) {
      x++;
      signal += p->iValue(s,channel) - baseline;
    }
  signal /= x;

  return signal;
}


//return the index of the vertical tower
int vertidx(int idx){
  if(idx % 2  == 0) return idx + 1;

  if(idx % 2 == 1) return idx - 1;

  return -999;
}


int pinit()
{
  cout << "initiallizing" <<endl;
  char name[500];
  for(int s=0; s<numPacket; s++){
    for(int c=0; c<numChannel; c++){
      sprintf(name,"signal_sec%d_ch%d",s,c);
      hsig[s][c]=new TH1F(name,name,100,-100,100);
      hsig[s][c]->StatOverflows(1); 
    }
  }
  return 0;
}

// this runs for every event
int process_event (Event * e)
{

  if ( e->getEvtType() == 9)
    {
      return 0;
    }

  int returnval = 0;

  // data is ordered into packets, like looping over sectors
  for (int packet=8001; packet<=8009; packet++) {
    
    Packet *p = e->getPacket(packet);
    if (p)
      {
	int sect=packet-8001; //packet to sector mapping
	for ( int c = 0; c < p->iValue(0,"CHANNELS"); c++)
	  {
	    double signal = getSignal(p,c);
	    //cout << "c: " << c << " p: " << packet <<endl;    
	    hsig[sect][c]->Fill(signal);
	    mean[sect][c]=mean[sect][c]+signal;
	    x2[sect][c] += signal*signal;
	  } // channels
	delete p;
	nevt++;
	
      } // if p
  } // packet loop
  
  return returnval;
}

int pclose()
{

  cout << "writing variables" <<endl;
  cout << "entries: "<< hsig[3][3]->GetEntries()<<endl;
  cout << "nevt: "<< nevt <<endl;
  cout << "sector channel  mean" <<endl;

  double stdev[numPacket][numChannel]={0.0};

  for (int s=0; s<numPacket; s++){
    for(int c=0; c<numChannel; c++){
      mean[s][c] /= nevt;
      x2[s][c] /= nevt;
      stdev[s][c] = sqrt(x2[s][c] - pow(mean[s][c],2)); 
      cout << "packet " << s << "  channel "   << c << "  mean=" << mean[s][c] << " " <<  hsig[s][c]->GetMean() << endl;
      //cout << s << " " << c << "std  " << stdev[s][c] << endl;
    }
  }
  // we would also add some conditions here for raising warning
  // this is where we would right to the DB

  return 0;
}

