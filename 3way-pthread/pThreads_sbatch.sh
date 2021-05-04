#!/bin/bash -l
##$ -l h_rt=0:01:00		# ask for 1 hour runtime
#SBATCH --constraint=elves     # only run on dwarves

module load OpenMPI

mpirun /homes/cody598/cis520/Project4/3way-pthread/pThreads #change to match the path to your code
