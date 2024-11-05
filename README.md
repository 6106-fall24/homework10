# Memory-Level Parallelism

To run and plot the MLP detective, use the following command:

```
make
./build_python_env.sh
source homework10-venv/bin/activate
./run_mlp_detective.sh > mlp_detective.csv
python plot.py mlp_detective.csv --x_axis totalKB --y_axis nsPerAccess --group_by pattern,nextIndex,blockSize --output_path mlp_detective.png
```

Plot mark phase results:
```
./run_mark.sh > mark_phase.csv
python plot.py mark_phase.csv --x_axis totalKB --y_axis nsPerAccess --group_by prefetchDistance --output_path mark_phase.png
```

Plot sweep phase results:
```
./run_sweep.sh > sweep_phase.csv
python plot.py sweep_phase.csv --x_axis totalKB --y_axis nsPerAccess --group_by prefetchDistanceInBytes --output_path sweep_phase.png
```

Plot bulk prefetch results:
```
./run_bulk_prefetch.sh > bulk_prefetch.csv
python plot.py bulk_prefetch.csv --x_axis totalKB --y_axis nsPerAccess --group_by innerTotalAccesses,enablePrefetch --filter innerArrayBytes=<X> --output_path bulk_prefetch.png
```



Additional usage guide for plot.py:
```
usage: plot.py [-h] --x_axis X_AXIS --y_axis Y_AXIS [--group_by GROUP_BY] [--output_path OUTPUT_PATH] [--filter FILTER] csv_path

Plot memory access patterns performance

positional arguments:
  csv_path              Path to the CSV file

options:
  -h, --help            show this help message and exit
  --x_axis X_AXIS       Column name for x-axis
  --y_axis Y_AXIS       Column name for y-axis
  --group_by GROUP_BY   Columns to group by, separated by commas
  --output_path OUTPUT_PATH
                        Path to the output file
  --filter FILTER       Filter in format "column=value" (e.g. "size=64")
```