#! /bin/bash
> sweep_args.txt
for expected_block_bytes in '32'; do
    for ((pow=14; pow <= 27; pow++)); do
        for prefetch_distance in '0' '64' '512' '4096'; do
            echo "$expected_block_bytes $pow 10 $prefetch_distance" >> sweep_args.txt
        done
    done
done