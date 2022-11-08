#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "OracleConstants.h"

void waitForFile(char* fileName){
	FILE* flagFile = 0;
	while(!flagFile){
		flagFile = fopen(fileName, "r");
		sleep(1);//Change this function (and value) as required (nanoseconds maybe...)
	}
	fclose(flagFile);
}

//Internal auxiliary function
int countDimensions(char* fileName){
	int dimensions = 0;
	FILE* populationFile = fopen(fileName, "r");
	if(populationFile){
		char buffer[ORACLE_MAX_LINE];	
		char* chk = fgets(buffer, ORACLE_MAX_LINE, populationFile);
		if(chk){
			const char delim[] = " ";
			char* token = strtok(buffer, delim);
			while(token){
				token = strtok(0, delim);
				dimensions++;
   			}
		}
		fclose(populationFile);
	}// The number of dimensions remains 0 otherwise
	return dimensions;
}

//Internal auxiliary function
int countIndividuals(char* fileName){
	int numIndividuals = 0;
	FILE* populationFile = fopen(fileName, "r");
	if(populationFile){
		char buffer[ORACLE_MAX_LINE];
		while (fgets(buffer, ORACLE_MAX_LINE, populationFile)){
			numIndividuals++;
    		}
		fclose(populationFile);
	}
	return numIndividuals;
}

//Internal auxiliary function
int parseIndividuals(FILE* popFile, int numIndividuals, int numDimensions, double* individuals){
	int success = 1, chkDim = 0, focus = 0;
	char buffer[ORACLE_MAX_LINE];
	const char delim[] = " ";
	char* token = 0;
	for(int i = 0; i<numIndividuals; i++){
		focus = i*numDimensions;
		chkDim = 0;
		fgets(buffer, ORACLE_MAX_LINE, popFile);		
		token = strtok(buffer, delim);
		individuals[i*numDimensions + chkDim] = atof(token);
		chkDim++;
		while(chkDim<numDimensions){
			token = strtok(0, delim);
			individuals[i*numDimensions + chkDim] = atof(token);
			chkDim++;
		}
	}
	return success;
}

int loadPopulationFile(char* fileName, int* numIndividuals, int* numDimensions, double** individuals){
	int success = 1;	
	FILE* populationFile = fopen(fileName, "r");
	if(!populationFile){
		printf("<OracleHelper> File %s not found\n", fileName);
		success = 0;
	}else{
		int nDim = countDimensions(fileName);
		int nInd = countIndividuals(fileName);
		if(nDim > 0 && nInd > 0){
			*numDimensions = nDim;
			*numIndividuals = nInd;
			*individuals = malloc(sizeof(double)*nDim*nInd);
			if(*individuals){
				success = parseIndividuals(populationFile, nInd, nDim, *individuals);
				if(!success){
					free(*individuals);
					printf("<OracleHelper> It was not possible to parse the files of individuals properly\n");
				}
			}else{
				success = 0;
				printf("<OracleHelper> Failed to allocate memory for the population\n");
			}
		}else{
			success = 0;
			printf("<OracleHelper> Impossible number of dimensions (%d) and/or individuals (%d)\n", nDim, nInd);
		}
		fclose(populationFile);
	}
	return success;
}

//Internal auxiliary function
int writeIndividual(char* fileName, double* startOfIndividual, int numDimensions){
	FILE* tmpFile = fopen(fileName, "w");
	if(tmpFile){
		for(int i = 0; i<(numDimensions-1); i++){
			fprintf(tmpFile, "%lf ",  startOfIndividual[i]);
		}
		fprintf(tmpFile, "%lf\n",  startOfIndividual[numDimensions-1]);
		fclose(tmpFile);
		return 1;//Success
	}
	return 0;//Failure
}

double processIndividual(int fullEvaluation, int individualID, double* startOfIndividual, int numDimensions, char* evalScriptName){
	double value = BAD_EVALUATION;
	char tmpFileName[MAX_SCRIPT_NAME] = {'\0'};
	sprintf(tmpFileName, "%s%d.txt", ORACLE_TMP_PATH, individualID);
	int chk = writeIndividual(tmpFileName, startOfIndividual, numDimensions);
	if(chk){
		char command[ORACLE_MAX_COMMAND] = {'\0'};
		sprintf(command, "%s %d %s", evalScriptName, fullEvaluation, tmpFileName);
		
		//EXECUTION AND OUTPUT GETTING <Direct output for conciseness, i.e., no output writing to a file>
		char buffer[MAX_SCRIPT_NAME];		
		FILE *fp = popen(command, "r");
		if(!fp){
			printf("<OracleHelper> Error opening a pipe for evaluating individual #%d\n", individualID);
		}else{
			value = atof(fgets(buffer, MAX_SCRIPT_NAME, fp));//IMPORTANT: Make your evaluation script just print its output in this version
		}

		//Write output and the expected signal file
		remove(tmpFileName);
	}
	return value;	
}

int writeEvaluationFile(double* values, int numIndividuals, char* fileName, char* signalFileName){
	int success = 1;	
	FILE* outputFile = fopen(fileName, "w");
	if(outputFile){
		for(int i = 0; i<numIndividuals; i++){
			fprintf(outputFile, "%lf\n", values[i]);
		}
		fclose(outputFile);
		FILE* signalFile = fopen(signalFileName, "w");
		if(!signalFile){
			success = 0;
			printf("<OracleHelper> Error when trying to write the signal file!\n");
		}else{
			fclose(signalFile);
		}
	}else{
		success = 0;
		printf("<OracleHelper> Error when trying to write the evaluation results to a file!\n");
	}
	return success;
}

int lookForPending(int* startFocus, double* values, int numIndividuals, double upperThreshold){
	int pendingFound = 0;
	int focus = *startFocus;//local copy
	while(focus < numIndividuals){
		if(values[focus] < upperThreshold){
			pendingFound = 1;//True
			break;
		}
		focus++;
	}
	*startFocus = focus;//Update the original value
	return pendingFound;
}
