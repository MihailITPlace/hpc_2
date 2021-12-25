#!/bin/bash

for ((i = 1; i < 4; i++ )); do
  echo "$i" attempt:
  for ((j=4; j > 1; j--)) do
    echo run app for "$j" threads...
    mpirun -n ${j} --use-hwthread-cpus  ./cmake-build-debug/hpc_2 3000 > attempt_"$i"_threads_"$j"
  done
done