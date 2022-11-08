#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <omp.h>

#include "OracleConstants.h"
#include "OracleHelper.h"

void requestExit(int nProcs){
	for(int i = 1; i<nProcs; i++){
		MPI_Send(0, 0, MPI_DOUBLE, i, ORACLE_DEAD_TAG, MPI_COMM_WORLD);
	}
}

void masterStage_One(int numIndividuals, int numDimensions, double* individuals, double* values, char* evalScriptName){
	#pragma omp parallel for
	for(int i = 0; i<numIndividuals; i++){
		//Pass the information to a temporary file and launch the script to use it (using files for generality, e.g., to allow working with either text of binary representations if needed (after updating writing and reading appropriately))
		values[i] = processIndividual(0, i, &(individuals[i*numDimensions]), numDimensions, evalScriptName);
		//The first parameter, 0, is expected to tell the script not to finish the evaluation if it is feasible (in other words, stick to a CPU core, not a GPU)
	}//After this loop, every infeasible value should have been evaluated. Feasible ones, which require a GPU, should have a negative value
}

void masterStage_Two(int numIndividuals, int numDimensions, double* individuals, double* values, int nProcs){
	MPI_Status status;
	int sendPoints = 0;
	int processedPoints = 0;
	double bufferRec = 0.0;
	int pointFocus = 0;
	int pendingFound = 0;

	for(int i = 1; i<nProcs && pointFocus<numIndividuals; i++){//Dispatch the first load
		pendingFound = lookForPending(&pointFocus, values, numIndividuals, 0.0);
		if(pendingFound){
			MPI_Send(&(individuals[pointFocus*numDimensions]), numDimensions, MPI_DOUBLE, i, pointFocus, MPI_COMM_WORLD);
			pointFocus++;
			sendPoints++;
		}else{
			break;
		}
	}
	
	while(processedPoints < sendPoints){//Wait for the results and keep going if needed
		MPI_Recv(&bufferRec, 1, MPI_DOUBLE, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
		values[status.MPI_TAG] = bufferRec;
		processedPoints++;
		pendingFound = lookForPending(&pointFocus, values, numIndividuals, 0.0);
		if(pendingFound){
			MPI_Send(&(individuals[pointFocus*numDimensions]), numDimensions, MPI_DOUBLE, status.MPI_SOURCE, pointFocus, MPI_COMM_WORLD);//The one who finished: Keep working!
			pointFocus++;	
			sendPoints++;
		}
	}
}

void master(int nProcs, char* evalScriptName){//printf("[MASTER] I'm here to execute %s with OpenMP\n", evalScriptName);
	waitForFile(REQUEST_EVAL_FILE);//Wait for a launcher file to exist
	//Plain single-shot version, i.e., the Oracle runs once before shutting down. Put this logic into an infinite loop for make it accessible through a complete optimization process
	int numIndividuals = 0, numDimensions = 0;
	double *individuals = 0;
	int success = loadPopulationFile(POPULATION_FILE, &numIndividuals, &numDimensions, &individuals);//Load the individuals
	if(success){
		MPI_Bcast(&numDimensions, 1, MPI_INT, ORACLE_MASTER, MPI_COMM_WORLD);//Share the number of dimensions for the workers to get ready
		double* values = malloc(sizeof(double)*numIndividuals);// A evaluation cell per individual
		if(values){
			masterStage_One(numIndividuals, numDimensions, individuals, values, evalScriptName);//Updating the infeasible values <OpenMP @ CPU cores>
			masterStage_Two(numIndividuals, numDimensions, individuals, values, nProcs);//Updating the infeasible values <MPI workers @ GPUs>
			writeEvaluationFile(values, numIndividuals, POPULATION_EVAL_FILE, FINISHED_EVAL_FILE);//Save results to an output file!
			free(values);//After making the values loadable by the optimizer, we do not further need them
		}
		free(individuals);
	}
	requestExit(nProcs);//Or keep everything going... This is a single-shot demo implementation
}

void worker(int rank, char* evalScriptName){//printf("[WORKER %d] I'm here to execute %s with my GPU\n", rank, evalScriptName);
	int numDimensions = 0;
	MPI_Bcast(&numDimensions, 1, MPI_INT, ORACLE_MASTER, MPI_COMM_WORLD);//Get the number of dimensions and get ready (This could be done dynamic for variable-length strategies)
	double* pointBuffer = malloc(sizeof(double)*numDimensions);//Where to store any new point to evaluate
	MPI_Status status;
	while(1){
		MPI_Recv(pointBuffer, numDimensions, MPI_DOUBLE, ORACLE_MASTER, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
		if(status.MPI_TAG == ORACLE_DEAD_TAG){
			break;//Finishing!
		}else{
			int individualID = status.MPI_TAG;//The first parameter is now 1: Tell the script to compute full evaluations involving a GPU
			double evalResult = processIndividual(1, individualID, pointBuffer, numDimensions, evalScriptName);//SLOW GPU EVALUATION
			MPI_Send(&evalResult, 1, MPI_DOUBLE, ORACLE_MASTER, individualID, MPI_COMM_WORLD);//Keep track of the point that I evaluated!!
		}
	}
	free(pointBuffer);
}

int main(int argc, char* argv[]){
	MPI_Init(&argc, &argv);
	int rank, size;

	MPI_Comm_size(MPI_COMM_WORLD, &size);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	if(size < 2){
		if(rank == ORACLE_MASTER){
			printf("Error: Please, launch the oracle with 2 processes at least!\n");
		}
	}else{
		char scriptName[MAX_SCRIPT_NAME] = {'\0'};// End of string
		if(rank == ORACLE_MASTER){
			if(argc < 2){
				printf("[MASTER] Usage: mpirun -np 2 ./MyOracle name_evaluator_script\n");
			}else{
				strcpy(scriptName, argv[1]);
				printf("[MASTER] Evaluation script: %s\n", scriptName);
			}
		}
		MPI_Bcast(scriptName, MAX_SCRIPT_NAME, MPI_CHAR, ORACLE_MASTER, MPI_COMM_WORLD);//Sharing the script name
		if(strlen(scriptName)>0){
			if(rank == ORACLE_MASTER){
                omp_set_num_threads(OPENMP_THREADS_MASTER);
				master(size, scriptName);
			}else{   
                omp_set_num_threads(OPENMP_THREADS_WORKER);
				worker(rank, scriptName);
			}
		}
	}
	MPI_Finalize();
	return 0;
}
