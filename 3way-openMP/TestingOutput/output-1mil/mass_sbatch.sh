#!/bin/bash
#SBATCH --job-name=openMP
#SBATCH -o 3way-openMP-MASSBATCH-STATS.out
for i in 1 2 4 8 16
do
	echo "Tasks: $i"
	sbatch --constraint=elves --ntasks-per-node=$i --nodes=1 --job-name=openMP -o $i-core-1mil.out openmp_sbatch.sh
done
