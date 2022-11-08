#!/bin/bash

#SBATCH --output=log_oracle.log
#SBATCH --nodelist=c22,c21
#SBATCH --ntasks=3

module load python/3.8.7

mpirun -np 3 --use-hwthread-cpus --oversubscribe MyOracle path/to/simuEvaluator.py "[1a2b]:a.name=c22 & b.name=c21"
