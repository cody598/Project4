#!/bin/bash
#SBATCH --job-name=OpenMP
#SBATCH -o 3way-OpenMP-MASSBATCH-STATS.out
for j in 1 2 3
do
	for i in 1 2 4 8 
	do
	if(($i == 8))
	then
		sleep 20
	fi
		echo "Nodes: $j, Tasks: $i"
		sbatch --constraint=elves --ntasks-per-node=$i --nodes=$j --job-name=OpenMP -o $j-node-$i-core-100k.out mpi_sbatch.sh
	done
done	
