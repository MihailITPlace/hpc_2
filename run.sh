#!/bin/bash

for ((i = 1; i < 6; i++ )); do
  echo "$i" attempt:
  for ((j=16; j > 1; j--)) do
    echo run app for "${j-1}" threads...
    mpirun -n ${j} --oversubscribe  ./cmake-build-debug/hpc_2 > attempt_"$i"_threads_"${j-1}"
  done
done