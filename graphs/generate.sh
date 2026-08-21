#!/bin/bash

# Check arguments
if [ $# -ne 2 ]; then
    echo "Usage: $0 <k> <nauty_path>"
    exit 1
fi

k=$1
nauty_path=$2

n_min=$(expr $k + 3)
n_max=$(( 2*k ))

# for loop
for n in $(seq $n_min $n_max);
do
  # Set number of edges
  e_max=$(expr $n + $k)

  # Generate with nauty
  ${nauty_path}/geng -C -d3 $n  ${e_max}:${e_max} > ${n}_${k}.g6

  # Show them in human readable format
  ${nauty_path}/showg -t -A -q  ${n}_${k}.g6 > ${n}_${k}.txt
done
