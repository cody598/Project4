#!/bin/bash -l
#SBATCH --time=0:10:00
#SBATCH --mem=100G
#SBATCH --constraint=elves

module load OpenMPI
module load foss/2020a --quiet

echo openMP

time /homes/cody598/cis520/Project4/3way-openMP/openMP-500k
grep DATA *.out > 500kTimes.csv