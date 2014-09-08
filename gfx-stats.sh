#!/bin/bash

dates_already_recorded=
if [ -f gfx-stats-all.csv ]
then
#  dates_already_recorded="`cat gfx-stats-all.csv|grep ^[0-9]\{4\}|cut -d ',' -f 1`"
dates_already_recorded="`cat gfx-stats-all.csv|grep '^[0-9]\{4\}'|sed 's/\([0-9]\{4\}\)-\([0-9]\{2\}\)-\([0-9]\{2\}\).*/\1\2\3/g'`"
fi

for f in `ls *-pub-crashdata.csv.gz`
do
  date="`echo $f | cut -d '-' -f 1`"
  if [[ "$dates_already_recorded" != *$date* ]]
  then
    echo $f
    gzip -dc $f | ./gfx-stats --stdin-with-date $f
  fi
done
