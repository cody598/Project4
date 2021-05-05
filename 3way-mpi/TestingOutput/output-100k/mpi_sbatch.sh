#!/bin/bash -l
#SBATCH --time=0:10:00
#SBATCH --mem=3G
#SBATCH --constraint=elves

module load OpenMPI
module load foss/2020a --quiet

echo MPI

mpirun /homes/cody598/cis520/Project4/3way-mpi/mpi-100k
grep DATA *.out > 100kTimes.csv

