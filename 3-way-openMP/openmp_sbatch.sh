#!/bin/bash -l
##$ -l h_rt=0:00:01             # ask for 1 minute runtime
#SBATCH --constraint=elves     # only run on dwarves

module load OpenMPI

mpirun mpirun /homes/cody598/cis520/Project4/3way-openMP/openMP #change to match the path to your code
