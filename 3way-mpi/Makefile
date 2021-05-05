all: mpi.c
	mpicc mpi-10k.c -o mpi-10k
	mpicc mpi-100k.c -o mpi-100k
	mpicc mpi-500k.c -o mpi-500k
	mpicc mpi-1mil.c -o mpi-1mil
clean:
	$(RM) mpi-10k
	$(RM) mpi-100k
	$(RM) mpi-500k
	$(RM) mpi-1mil