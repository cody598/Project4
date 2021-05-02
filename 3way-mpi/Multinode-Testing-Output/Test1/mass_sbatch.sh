#!/bin/bash
for i in 1 2 3 4 5 6 7
do
	echo "Nodes Used: 1, Tasks Per Node: $i"
	sbatch --constraint=elves --ntasks-per-node=$i --nodes=1 mpi_sbatch.sh
done