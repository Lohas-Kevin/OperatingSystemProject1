#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <ctype.h>

typedef struct{
	char name;
	int arriveTime;
	int burst;
	int pointer;
	int* burstTime;
	int* ioTime;
	int cpuTime;
	int turnaroundTime;
	int waitTime;
} Process;

//this function is used to check whether the input
//string is a valid positive int or 0;
int checkInt(char* input){
	for(int i = 0; i < strlen(input); i++){
		if(isdigit(input[i]) == 0){
			fprintf(stderr, "the error char is [%c]\n", input[i]);
			return -1;
		}
	}
	return atoi(input);
}

//this function is used to check whether the input
//string is a valid positive double or 0.0;
double checkDouble(char* input){
	for(int i = 0; i < strlen(input); i++){
		if(isdigit(input[i]) == 0 && input[i] != '.'){
			fprintf(stderr, "the error char is [%c]\n", input[i]);
			return -1.0;
		}
	}
	return atof(input);
}

//this function is used to generate a random 
//which does not exceed the threshold
double expRandom(double lambda, int threshold){
	
	int trigger = 0;
	double result = -1;
	while(trigger == 0){
		//this part of code is from lecture notes
		double r = drand48();
		result = -log(r) / lambda;
		if(result < threshold){ trigger = 1; }
		
	}
	
	return result;
}


void createProcess(Process* p, char nameIn, double lambda, int threshold){
	
	//count is used to determine the process name
	p->name = nameIn;
	p->arriveTime = (int)floor(expRandom(lambda, threshold));
	
	p->burst = ((int)(drand48()*100.0))+1;
	
	
	//dynamically allocate the memeory for cputime and iotime
	p->burstTime = calloc(p->burst, sizeof(int));
	p->ioTime = calloc(p->burst, sizeof(int));
	
	//this for loop will set up the cpu burst time and io burst time
	for(int i = 0; i < p->burst; i++){
		if(i != p->burst -1){
			(p->burstTime)[i] = (int)ceil(expRandom(lambda, threshold));
			(p->ioTime)[i] = (int)ceil(expRandom(lambda, threshold));
		}else{
			//at the last cpu burst, the io time is set to -1
			(p->burstTime)[i] = (int)ceil(expRandom(lambda, threshold));
			(p->ioTime)[i] = -1;
		}
	}
	
	//then set the remaning parts to 0
	
	
}


void freeProcess(Process* p){
	free(p->burstTime);
	free(p->ioTime);
	free(p);
}

void ProcessViewer(Process* p){
	printf("the name of p is [%c]\n", p->name);
	printf("the arriveTime of p is [%i]\n", p->arriveTime);
	printf("the burst of p is [%i]\n", p->burst);
	printf("the pointer of p is [%i]\n", p->pointer);
	printf("the cpuTime of p is [%i]\n", p->cpuTime);
	printf("the turnaroundTime of p is [%i]\n", p->turnaroundTime);
	printf("the waitTime of p is [%i]\n", p->waitTime);
	
	printf("The elements in burstTime are: \n");
	for(int i = 0; i < p->burst; i++){
		printf("%d:[%d]  ", i, (p->burstTime)[i]);
		if(i%7 == 0){
			printf("\n");
		}
	}
	printf("\n");
	
	printf("The elements in ioTime are: \n");
	for(int i = 0; i < p->burst; i++){
		printf("%d:[%d]  ", i, (p->ioTime)[i]);
		if(i%7 == 0){
			printf("\n");
		}
	}
	printf("\n");
	printf("\n");
}


int main( int argc, char* argv[]){
	
	//This part is for arguments variables;
	int seed;
	double lambda;
	int threshold;
	int processNum;
	int textSwitchTime;
	double alpha;
	int timeSlice;
	char* addPosition;
	char* nameString = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	
	//this part is for input check
	//FIRSTLY
	//we check the input arguments numbers
	if(argc != 8){
		perror("ERROR: Wrong amount of Inputs arguments\n");
		return EXIT_FAILURE;
	}
	
	//SECONDLY
	//we check the inputs
	for(int i = 1; i <= 7; i++){
		if(i == 2){
			//only special case is the second input
			//we need to check whether the input is 
			//a valid double which is bigger then 0.
			if(checkDouble(argv[i]) <= 0.0){
				fprintf(stderr,"ERROR: The %d argument is wrong\n", i);
				return EXIT_FAILURE;
			}else{
				lambda = checkDouble(argv[i]);
			}
		}else if( i == 6){
			
			//check the constant alpha for calculating tau
			if(checkDouble(argv[i]) < 0.0 || checkDouble(argv[i]) >= 1){
				fprintf(stderr,"ERROR: The %d argument is wrong\n", i);
				return EXIT_FAILURE;
			}else{
				alpha = checkDouble(argv[i]);
			}
			
		}else{
			//check the remaining inputs
			//these inputs should bigger than 0
			if(checkInt(argv[i]) <= 0){
				fprintf(stderr,"ERROR: The %d argument is wrong\n", i);
				return EXIT_FAILURE;
			}else{
				//hard-code part to assign the variables
				if(i == 1){ seed = checkInt(argv[i]);}
				else if(i == 3){threshold = checkInt(argv[i]);}
				else if(i == 4){processNum = checkInt(argv[i]);}
				else if(i == 5){textSwitchTime = checkInt(argv[i]);}
				else if(i == 7){timeSlice = checkInt(argv[i]);}
			}
		}
	}
	
	//over here we check the last argument,
	//if the argumnent is not "BEGINNING", we
	//set the addPosition to "END"
	if(strcmp("BEGINNING", argv[8]) != 0){
		addPosition = "END";
	}else{
		addPosition = "BEGINNING";
	}
	
	Process** ProcessArray = calloc(processNum, sizeof(Process*));
	srand48(seed);
	
	for(int i = 0; i < processNum; i++){
		ProcessArray[i] = calloc(1, sizeof(Process));
		createProcess(ProcessArray[i],nameString[i],lambda,threshold);
	}
	//
	//insert algo here
	//
	
	//free the memory
	for(int i = 0; i < processNum; i++){
		freeProcess(ProcessArray[i]);
	}
	
	return EXIT_SUCCESS;
}