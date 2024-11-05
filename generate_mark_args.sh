#! /bin/bash
count=0
> mark_args.txt

for prefetch_distance in '0' '4' '16' '64' '256'; do
     for ((num_nodes = 1000; num_nodes <= 2048000; num_nodes *= 2)); do
         echo "$num_nodes 3 3 20 $prefetch_distance" >> mark_args.txt
         count=$((count + 1))
     done
done
echo "Number of experiments count: $count"
