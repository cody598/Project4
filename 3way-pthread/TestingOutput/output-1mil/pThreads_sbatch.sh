#!/bin/bash -l
#SBATCH --time=0:10:00
#SBATCH --mem=100G
#SBATCH --constraint=elves

module load OpenMPI
module load foss/2020a --quiet

echo pThread

time /homes/cody598/cis520/Project4/3way-pthread/pThreads-1mil
grep DATA *.out > 1-milTimes.csv
