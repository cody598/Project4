#!/bin/bash -l
#SBATCH --time=0:10:00
#SBATCH --mem=3G
#SBATCH --constraint=elves

module load OpenMPI
module load foss/2020a --quiet

echo MPI

time /homes/cody598/cis520/Project4/3way-mpi/mpi-500k
grep DATA *.out > 500kTimes.csv

