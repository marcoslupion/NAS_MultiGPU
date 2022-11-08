all:
	gcc -o fakeOptimizer simuOptimizer.c
	mpicc -o MyOracle Oracle.c OracleHelper.c -fopenmp
clean:
	rm fakeOptimizer MyOracle