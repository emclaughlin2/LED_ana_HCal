#!/bin/bash

#ddump -t 9 -p 912 "$1" > led_info.txt
#ddump -t 9 -p 913 "$1" > pindiode_info.txt
echo $(basename "$1") > run_type_info.txt
