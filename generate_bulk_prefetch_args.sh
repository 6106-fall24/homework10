#!/bin/bash
> bulk_prefetch_args.txt
for ((bytesLog=27; bytesLog >= 15; bytesLog -= 3)); do
    for innerBytesLog in '8' '9' '10'; do
        for innerAccesses in '1' '4' '16'; do
            for enable_prefetch in '0' '1'; do
                echo "$bytesLog 1000000 $innerBytesLog $innerAccesses $enable_prefetch 10" >> bulk_prefetch_args.txt
            done
        done
    done
done
