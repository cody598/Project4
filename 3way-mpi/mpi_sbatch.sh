#!/bin/bash -l
##$ -l h_rt=0:01:00		# ask for 1 hour runtime
#SBATCH --constraint=elves     # only run on elves

module load OpenMPI
module load foss/2020a --quiet

mpirun /homes/cody598/cis520/Project4/3way-mpi/mpi #change to match the path to your code
