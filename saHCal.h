#ifndef __SAHCAL_H__
#define __SAHCAL_H__

#include <pmonitor/pmonitor.h>
#include <Event/Event.h>
#include <Event/EventTypes.h>

void fithisto();
void fitLED();
void fithistogaus();
void toymc(int nevents);
int process_event (Event *e); //++CINT 

void set_threshold ( const int t);
int get_threshold ();

std::vector<float> anaWaveform(Packet* p, const int channel);
//TH1F *hsig[48];


#endif /* __SAHCAL_H__ */
