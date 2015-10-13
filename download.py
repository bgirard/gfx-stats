#!/bin/bash

import datetime
import gzip
import os
import time

try:
    import requests
except ImportError:
    print "pip install requests"
    raise


CRASH_URL = "https://crash-stats.mozilla.com/graphics_report/"
# last known date when crash-analysis put out working dumps (plus 1 day)
START_DATE = datetime.date(2015, 9, 20)


def download(auth_token, date, destination):
    # with open(destination, 'w') as f:
    with gzip.open(destination, 'wb') as f:
        print "DOWNLOADING FOR", date
        t0 = time.time()
        r = requests.get(
            CRASH_URL,
            params={'date': date.strftime('%Y-%m-%d')},
            headers={
                'Auth-Token': auth_token,
                'Accept-Encoding': 'gzip'
            }
        )
        assert r.status_code == 200, r.status_code
        f.write(r.content)
        t1 = time.time()
        print "WROTE", destination, "TOOK", "%.1f seconds" % (t1 - t0)


def download_all(auth_token):
    date = START_DATE
    while date < datetime.datetime.utcnow().date():
        date += datetime.timedelta(days=1)
        destination = date.strftime("%Y%m%d-pub-crashdata.csv.gz")
        if not os.path.isfile(destination):
            download(auth_token, date, destination)
        else:
            print "ALREADY HAVE", destination
    return 0


if __name__ == '__main__':
    import sys
    args = sys.argv[1:]
    if '-h' in args or '--help' in args:
        print "python %s [MYAPITOKEN]" % __file__
        sys.exit(1)
    if args:
        auth_token, = args
    else:
        auth_token = raw_input('API Token: ').strip()
    sys.exit(download_all(auth_token))
