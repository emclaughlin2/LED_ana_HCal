# LED_ana_HCal
Analysis script for SPHENIX HCal LED running 

(Update 6/5/23)
To run led + pedestal runs + analysis on an seb machine using runcontrol for running both seb16 and seb17, one can just run 'bash led_pedestal_run.sh' from this directory.
To analyze previously taken runs, run 'sh output_LED_status.sh /path/to/file/prdf-file' and 'sh saHCal.sh /path/to/file/prdf-file' from this directory.
Data is logged in the hcal_led and hcal_pedestal tables of the daq database on db1 server.
