#!/bin/bash -l
#SBATCH --time=0:01:00
#SBATCH --mem=4G
#SBATCH --constraint=elves

module load OpenMPI
module load foss/2020a --quiet

echo MPI

time /homes/cody598/cis520/Project4/3way-openMP/openMP-100k

