all: openMP.c
	gcc -o openMP -fopenmp openMP.c
	gcc -o openMP-10k -fopenmp openMP-10k.c
	gcc -o openMP-100k -fopenmp openMP-100k.c
	gcc -o openMP-500k -fopenmp openMP-500k.c
	gcc -o openMP-1mil -fopenmp openMP-1mil.c
clean:
	$(RM) openMP
	$(RM) openMP-10k
	$(RM) openMP-100k
	$(RM) openMP-500k
	$(RM) openMP-1mil