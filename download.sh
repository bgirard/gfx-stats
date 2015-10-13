# This script has been replaced by download.py
# Kept around for legacy inspection.


CRASH_URL="https://crash-analysis.mozilla.com/crash_analysis/"

set -e

trap 'killall' INT

killall() {
    trap '' INT TERM     # ignore INT and TERM while shutting down
    kill -TERM 0         # fixed order, send TERM not INT
    wait
}

function max_bg_procs {
    if [[ $# -eq 0 ]] ; then
      exit
    fi
    local max_number=$((0 + ${1:-0}))
    while true; do
            local current_number=$(jobs | wc -l)
            if [[ $current_number -lt $max_number ]]; then
                    break
            fi
            sleep 1
    done
}

INDEX_HTML=`curl -s $CRASH_URL`

while read -r line; do
  if [[ "${line:0:9}" == "<a href=\"" ]] ;
  then
    CRASH_DATE=${line:9:8}
    CRASH_FILE=$CRASH_DATE-pub-crashdata.csv.gz
    ls $CRASH_FILE 2>&1 > /dev/null 2> /dev/null || {
      # Download the file
      echo "Fetching $CRASH_FILE"
      wget --no-check-certificate "$CRASH_URL$CRASH_DATE/$CRASH_FILE" 2> /dev/null > /dev/null || {
        echo "Skipping $CRASH_FILE"
      }
    }
  fi
done <<< "$INDEX_HTML"

echo "Crash file updated"
