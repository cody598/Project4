#!/bin/bash
for j in 1 2 3
do
	for i in 1 2 3 4 
	do
	if(($i == 4))
	then
		sleep 20
	fi

		echo "Nodes: $j, Tasks: $i"
		 sbatch --constraint=elves --ntasks-per-node=$i --nodes=$j mpi_sbatch.sh
		 #sbatch --ntasks-per-node=$i --nodes=1 mpi_sbatch.sh

	done
done