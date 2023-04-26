//This simple file acts as an optimizer that creates a file with the individuals to evaluate
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "OracleConstants.h"

int generatePopulation(int nIndividuals, int nComponents){ // Simulating the creation of a population & writing the activation flag
	FILE* popFile = fopen(POPULATION_FILE, "w");
	if(popFile){//POPULATION (A matrix of vectors)
		int counter = 0;
		for(int i = 0; i<nIndividuals; i++){
			for(int j = 0; j<(nComponents-1); j++){
				fprintf(popFile, "%lf ", (double) counter);//i);
				counter++;
			}
			fprintf(popFile, "%lf\n", (double) counter);//i);
			counter++;
		}
	}else{
		printf("Error: It was not possible to create the file with the individuals\n");
		return 0;
	}
	fclose(popFile);
	// THE FLAG THAT WOULD LAUNCH THE ORACLE
	FILE* flagFile = fopen(REQUEST_EVAL_FILE, "w");
	if(!flagFile){
		printf("Error: It was not possible to create the flag file that activates the oracle\n");
		return 0;
	}else{
		fclose(flagFile);
	}
	return 1;
}

int waitAndLoadValues(int nIndividuals){ // Simulating an active wait and load of the evaluation results
	FILE* flagFile = 0;
	while(!flagFile){
		flagFile = fopen(FINISHED_EVAL_FILE, "r");
		sleep(1); // Sleep one second (negligible time when dealing within a NAS environment)
	}// Good place for a timeout and error	
	fclose(flagFile);
	double* popValues = malloc(sizeof(double)*nIndividuals);
	if(!popValues){
		printf("Error: It was not possible to get memory for the values of the individuals\n");
		return 0;
	}else{
		FILE* valFile = fopen(POPULATION_EVAL_FILE, "r");
		if(!valFile){
			printf("Error: The evaluation file is not available\n");
		}else{
			for(int i = 0; i<nIndividuals; i++){
				fscanf(valFile, "%lf\n", &(popValues[i]));
				printf("Value of the individual %d: %lf\n", i, popValues[i]);
			}
			fclose(valFile);
		}
	}
	free(popValues);
	return 1;
}

void simulatedOptimizer(int nIndividuals, int nComponents){
	int operationCheck = generatePopulation(nIndividuals, nComponents);
	if(operationCheck){
		operationCheck = waitAndLoadValues(nIndividuals);
	}
}

void cleanUp(){
	remove(POPULATION_FILE);
	remove(REQUEST_EVAL_FILE);
	remove(FINISHED_EVAL_FILE);
	remove(POPULATION_EVAL_FILE);
}

int main(int argc, char* argv[]){
	if(argc != 3){
		printf("Usage: ./simu_optimizer num_individuals components_per_individual\n");
		printf("Hint: If you want to see a sample population file, run this module alone and see Input_Output/\n");
	}else{
		int nIndividuals = atoi(argv[1]);//How many vectors of real numbers
		int nComponents = atoi(argv[2]);//The size of every vector
		if(nIndividuals<=0 || nComponents<=0){
			printf("Error: The number of individuals and their size as vectors must by greater than 0\n");	
		}else{
			simulatedOptimizer(nIndividuals, nComponents);
			cleanUp();
		}
	}
	return 0;
}
