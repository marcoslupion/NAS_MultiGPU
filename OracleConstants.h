#ifndef MY_CONSTANTS

#define MY_CONSTANTS

#include <limits.h>

#define POPULATION_FILE "INPUT_OUTPUT/popFile.txt"

#define REQUEST_EVAL_FILE "INPUT_OUTPUT/request.txt"

#define FINISHED_EVAL_FILE "INPUT_OUTPUT/request_finished.txt"

#define POPULATION_EVAL_FILE "INPUT_OUTPUT/popEval.txt"

#define ORACLE_MAX_LINE 4096 //The maximum number that a single individual can have when printed

#define ORACLE_MASTER 0

#define MAX_SCRIPT_NAME 80

#define ORACLE_MAX_COMMAND 160

#define ORACLE_TMP_PATH "INPUT_OUTPUT/"

#define BAD_EVALUATION -1.0

#define ORACLE_DEAD_TAG INT_MAX

#define OPENMP_THREADS_MASTER 96

#define OPENMP_THREADS_WORKER 16

#endif
