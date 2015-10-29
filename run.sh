#!/bin/bash

function max_bg_procs {
    if [[ $# -eq 0 ]] ; then
            echo "Usage: max_bg_procs NUM_PROCS.  Will wait until the number of background (&)"
            echo "           bash processes (as determined by 'jobs -p') falls below NUM_PROCS"
            return
    fi
    local max_number=$((0 + ${1:-0}))
    while true; do
            local current_number=$(jobs -p | wc -l)
            if [[ $current_number -lt $max_number ]]; then
                    break
            fi
            sleep 1
    done
}

dates_already_recorded=
if [ -f gfx-stats-all.csv ]
then
#  dates_already_recorded="`cat gfx-stats-all.csv|grep ^[0-9]\{4\}|cut -d ',' -f 1`"
dates_already_recorded="`cat gfx-stats-all.csv|grep '^[0-9]\{4\}'|sed 's/\([0-9]\{4\}\)-\([0-9]\{2\}\)-\([0-9]\{2\}\).*/\1\2\3/g'`"
fi

echo $dates_already_recorded

for f in `ls gfx-stats*.csv`
do
  sed -i.bu '/^2015/d' "$f"
done


for f in `ls *-pub-crashdata.csv.gz`
do
  #if [[ "$f" == 2014* ]]
  #then
    max_bg_procs 10
    date="`echo $f | cut -d '-' -f 1`"
    if [[ "$dates_already_recorded" != *$date* ]]
    then
      echo $f
      echo "gzip -dc $f | ./gfx-stats --stdin-with-date $f"
      gzip -dc $f | ./gfx-stats --stdin-with-date $f &
    fi
  #fi
done
