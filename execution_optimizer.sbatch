#!/bin/bash

#SBATCH --output=log_optimizer.log
#SBATCH --partition=cpu_ivybridge

./fakeOptimizer 100 100
