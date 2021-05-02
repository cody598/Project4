#!/bin/bash
for j in 1 2 3 
do
	for i in 1 2 3 4
	do
		echo "Nodes Used: $j, Tasks Per Node: $i"
		sbatch --constraint=elves --ntasks-per-node=$i --nodes=$j mpi_sbatch.sh

	done
done