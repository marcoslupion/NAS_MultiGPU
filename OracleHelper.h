#ifndef ORACLE_HELPER

#define ORACLE_HELPER

void waitForFile(char* fileName);

int loadPopulationFile(char* fileName, int* numIndividuals, int* numDimensions, double** individuals);

double processIndividual(int fullEvaluation, int individualID, double* startOfIndividual, int numDimensions, char* evalScriptName);

int writeEvaluationFile(double* values, int numIndividuals, char* fileName, char* signalFileName);

int lookForPending(int* startFocus, double* values, int numIndividuals, double upperThreshold);

#endif
