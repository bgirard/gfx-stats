gfx-stats
=========

Generates Mozilla graphics stats from crash data

A copy of the resulting stats is hosted there:

  http://people.mozilla.org/~bjacob/gfx_features_stats/

How to build and generate stats
===============================

Build gfx-stats.cpp as a standalone c++11 program, ensuring that the resulting executable is named `gfx-stats`. Enable optimization. For example:

```
  c++ gfx-stats.cpp -o gfx-stats --std=c++0x -O3 -DNDEBUG -Wall -Wextra -pedantic
```

Download crashdata files from Mozilla's `crash-analysis` archives, to the same directory:

  https://crash-analysis.mozilla.com/crash_analysis/

For example:

```
  wget --no-check-certificate https://crash-analysis.mozilla.com/crash_analysis/20140907/20140907-pub-crashdata.csv.gz
```

Download as many per-day `.csv.gz` files as you wish.

Then run the runner script, `gfx-stats.sh`, in BASH, in the same directory that contains the above `gfx-stat`s executable and downloaded `.csv.gz` files.

```
  ./gfx-stats.sh
```

This will look for the `.csv.gz` files with their specific filename format indicating the date that they correspond to, so don't rename these files. This will then produce `.csv` files describing one complete graph each, for example:

```
  gfx-stats-win7.csv
  gfx-stats-os-market-share.csv
  ...
```

Copy these `.csv` files and the `index.html` file to some directory and serve it over HTTP or HTTPS, not as a `file://` URL.

To subsequently update the graph, just repeat the operation of downloading new crashdata `.csv.gz` files, and just re-run:

```
  ./gfx-stats.sh
```

This will automatically find existing graphs .csv files in the current directory, and will directly start processing new `.csv.gz` files and appending to the existing `.csv` files.
