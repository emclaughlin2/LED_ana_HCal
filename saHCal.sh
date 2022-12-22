#! /bin/bash

FILE="$1"

if [ -z "$FILE" ] ; then

    root -l saHCal.C\(0\)
else

    root -l saHCal.C\(\"$FILE\"\)
fi

