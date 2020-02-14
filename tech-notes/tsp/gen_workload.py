#!/usr/bin/env python3
import os
import argparse
import random
import csv

def generate(max_coord, cities):
    res = []
    for i in range(cities):
        res.append({
            "id": i,
            "x": random.randint(0, max_coord),
            "y": random.randint(0, max_coord)
        })
    return res

def parse_args():
    parser = argparse.ArgumentParser(
        "Generate random TSP workload"
    )
    parser.add_argument("--out", help="out file name", default="out")
    parser.add_argument("--num", help="number of cities", type=int)
    parser.add_argument("--max", help="maximum X/Y values", type=int)
    args = parser.parse_args()
    return args

if __name__ == "__main__":
    args = parse_args()
    res = generate(args.max, args.num)
    fieldnames = res[0].keys()

    with open(args.out, "w") as csvfile:
        writer = csv.DictWriter(csvfile, fieldnames=fieldnames)
        writer.writeheader()
        for row in res:
            writer.writerow(row)
    print("done")
