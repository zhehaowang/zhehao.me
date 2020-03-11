#! /bin/bash

for i in {10..30}; do
    echo $i;
    ./gen_workload.py --out "out$i" --max 100 --num $i
done
