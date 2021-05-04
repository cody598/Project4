#!/bin/bash
#SBATCH --job-name=openMP
#SBATCH -o 3way-openMP-MASSBATCH-STATS.out

for j in 1
do
	for i in 1 2 4 8 16
	do
	if(($i == 8))
	then
		sleep 20
	fi
		echo "Nodes: $j, Tasks: $i"
		sbatch --constraint=elves --ntasks-per-node=$i --nodes=$j --job-name=openMP -o $j-node-$i-core-10k.out open_sbatch.sh
	done
done