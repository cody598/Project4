#!/bin/bash
for i in 2 4 6 8 10 12 14 16
do
	echo "Nodes Used: 1, Tasks Per Node: $i"
	sbatch --constraint=elves --ntasks-per-node=$i --nodes=1 mpi_sbatch.sh
done