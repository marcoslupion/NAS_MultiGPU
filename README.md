# NAS - MultiGPU
This repository is linked to the paper _Accelerating neural network architecture search using multi-GPU high-performance computing_, submitted to The Journal of Supercomputing and authored by M. Lupión[^1], N. C. Cruz[^2], J. F. Sanjuan[^1], B. Paechter[^3] and P. M. Ortigosa[^1].
[^1]: Department of Informatics, University of Almerı́a, CEiA3, Sacramento Road, Almerı́a, 04120, Spain.
[^2]: Department of Computer Engineering, Automation and Robotics, University of Granada, Journalist Daniel Saucedo Aranda Street, Granada, 18071, Spain.
[^3]: School of Computing, Edinburgh Napier University, 10 Colinton Rd, EH10 5DT, Edinburgh, Scotland, UK.

It contains the core implementation of the parallelization strategy proposed in the referred work. The work deals with evaluating in parallel the solutions of population-based meta-heuristics making the following considerations:
- Each individual encodes a neural network architecture
- Not every architecture is feasible
- Feasible architectures need a GPU to be evaluated
- The evaluation time can significantly vary between different feasible candidate solutions.

The parallelization approach consists in developing a program, known as the oracle, that expects to receive as input a file containing a set of individuals to evaluate. It also needs the user to define an external neural network assessment script (e.g., with Python to interact with TensorFlow, as done in our paper). With this information, the oracle controls the evaluations managing multiple GPUs, even in different cluster nodes using MPI, and multiple cores for the master process through OpenMP. Finally, it writes the values to an output value for the optimizer to load them.

The oracle is implemented by following a master-worker design pattern. Its execution model expects a master process and a worker at least. There are two execution phases: The first one, computationally fast and regular, consists in identifying feasible and infeasible solutions (which remain evaluated at this point). It is launched by the master process using OpenMP. The second one, computationally demanding due to the necessity of building the candidate neural network architectures and training them, is iteratively divided by the master among the workers. Each worker is attached to a GPU, as working with the neural networks becomes intractable otherwise, and it performs the computation, sends the result back to the master, and waits for a new task.

In the referred paper, we use this parallelization module linked to the widespread Teaching-Learning-based Optimization ([TLBO](https://www.sciencedirect.com/science/article/pii/S0020025511004191)), but it can be attached to any other optimizer. The only requirement is to make the algorithm write the set of solutions to evaluate into a text file and wait for reading the resulting output. The expected format is as simple as possible, as can be seen in the examples included. Namely, each candidate solution must be in a row with its different components separated by blank spaces. The output is another file in which the number in line i corresponds to the value of the i-th solution in the input file.

## Usage
- **Compile**: *make* 
- **Run Optimizer**: *./simu_optimizer* <*number of individuals*> <*number of components per individual*>
- **Run Oracle**: *mpirun -np* <*num_processes*> *./MyOracle* <*name_evaluator_script*>

Example in the current repository: 
```
make
./simu_optimizer 10 3
mpirun -np 3 ./MyOracle simuEvaluator.py
```

> **_NOTE:_**  The system can be also launched using Slurm, as in the real execution. 
> - *execution_oracle.sbatch*: Launches the oracle. It uses two cluster nodes. One of them executes the first phase of the oracle and the master process in the second phase. The second node executes the workers of the MPI process. 
> - *execution_optimizer.sbatch*: Launches the optimizer in a cluster node.

## License
This software is provided 'as is', with the best intention but without any kind of warranty and responsibility of stability or correctness. It can be freely used, distributed and modified by anyone interested, but a reference to this source is required.

## Citing
For citing this software, please, cite the associated paper as follows: M. Lupión, N.C. Cruz, J.F. Sanjuan, B. Paechter and P.M. Ortigosa. Accelerating neural network architecture search using multi-GPU high-performance computing. _Journal of Supercomputing_, 2022.

## Contact
For any question or suggestion feel free to contact:

- Marcos Lupión: [marcoslupion@ual.es](marcoslupion@ual.es)
- N.C. Cruz: [ncalvocruz@ugr.es](ncalvocruz@ugr.es)
