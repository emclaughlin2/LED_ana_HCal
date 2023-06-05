#!/bin/bash

cat led_pedestal_run.sh > /home/phnxrc/operations/HCal/run_led_pedestal_script.txt

cd ~/operations/HCal

export GTMHOST=gtm.sphenix.bnl.gov
opc0="opc0.sphenix.bnl.gov"
easthost="seb17.sphenix.bnl.gov"
westhost="seb16.sphenix.bnl.gov"

echo "Setup led runtype and scheduler"
export RCDAQHOST=$westhost
rcdaq_client daq_set_runtype led 
RCDAQHOST=$easthost
rcdaq_client daq_set_runtype led
gl1_gtm_client gtm_stop 9 
gl1_gtm_client gtm_load_modebits 9 schedulers/led_running.scheduler
gl1_gtm_client gtm_enable 9 
gl1_gtm_client gtm_set_accept_l1 9 0 

echo "Setting detector settings for led run. Please wait"
ssh opc0 "python3 drichf1/control/pulser.py all 100; \
    python3 drichf1/control/gain.py both all n; \
    python3 drichf1/control/test_pulse.py both all off; \
    python3 drichf1/control/led.py both all on; \
    python3 drichf1/control/channel_specific_bias.py both all zero; \
    sleep 5; \
    python3 drichf1/control/channel_specific_bias.py both all nominal"

echo "Reading out HCal configuration into a device-file."
ssh $opc0 "sh drichf1/control/generate_daq_device_file_led_pin.sh"

echo "Starting led run now."
rc_client rc_begin
sleep 2
readarray -t led_run_number < <(rc_client rc_status)
IFS=' ' read -ra led_run_number_east <<< "${led_run_number[0]}"
IFS=' ' read -ra led_run_number_west <<< "${led_run_number[1]}"
if [ ${led_run_number_east[1]} -eq ${led_run_number_west[1]} ]; then
    led_run_number=${led_run_number_east[1]}
else
    echo "Run numbers for west and east side are different. Ending run."
    rc_client rc_end
    rc_client rc_close
    exit
fi
sleep 18
rc_client rc_end
rc_client rc_close

echo "Led run finished. Beginning pedestal run setup now"
gl1_gtm_client gtm_stop 9
gl1_gtm_client gtm_load_modebits 9 schedulers/silas_pedestal_new_test.scheduler
gl1_gtm_client gtm_enable 9
gl1_gtm_client gtm_set_accept_l1 9 0 

echo "Setting detector settings for pedestal run"
ssh $opc0 "python3 drichf1/control/test_pulse.py both all off; \
    python3 drichf1/control/led.py both all off"

echo "Reading out HCal configuration into a device-file."
ssh $opc0 "sh drichf1/control/generate_daq_device_file_led_pin.sh"

echo "Starting pedestal run now."
rc_client rc_begin
sleep 2
readarray -t pedestal_run_number < <(rc_client rc_status)
IFS=' ' read -ra pedestal_run_number_east <<< "${pedestal_run_number[0]}"
IFS=' ' read -ra pedestal_run_number_west <<< "${pedestal_run_number[1]}"
if [ ${led_run_number_east[1]} -eq ${pedestal_run_number_west[1]} ]; then
    pedestal_run_number=${pedestal_run_number_east[1]}
else
    echo "Run numbers for west and east side are different. Ending run."
    rc_client rc_end
    rc_client rc_close
    exit
fi
sleep 18
rc_client rc_end
rc_client rc_close

get_file() {
    local n=$1
    local type=$2
    local side=$3
    printf -v n "%08d" $n
    echo "/bbox/commissioning/HCal/$type/$type_$side-$n-0000.prdf"
}

echo "Analyzing led run and logging in database"
cd ~/bseidlitz/LED_ana_HCal/
west_file=get_file(led_run_number,led,West)
sh saHCal.sh west_file
east_file=get_file(led_run_number,led,East)
sh saHCal.sh east_file

echo "Analyzing pedestal run and logging in database"
west_file=get_file(pedestal_run_number,pedestal,West)
sh saHCal.sh west_file
east_file=get_file(pedestal_run_number,pedestal,East)
sh saHCal.sh east_file


