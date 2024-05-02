#! /bin/bash

FILE="$1"

if [ -z "$FILE" ] ; then

    root -l saHCal.C\(0\)
else
    sh output_LED_status.sh $FILE
    root -l saHCal.C\(\"$FILE\"\)
fi

