#!/bin/bash
> mlp_detective_args.txt
for ((size=13; size <= 27; size++)); do
    echo "a c r $size 0 10" >> mlp_detective_args.txt
    echo "a c c $size 0 10" >> mlp_detective_args.txt
    echo "a s c $size 0 10" >> mlp_detective_args.txt
    echo "a m c $size 0 10" >> mlp_detective_args.txt
    echo "a s c $size 4 10" >> mlp_detective_args.txt
done
