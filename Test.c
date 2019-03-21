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
	int inReadyQueue;
	int inCPUBurst;
	int inIO;
} Process;

typedef struct{
	int a;
	int* b;
} TestStruct;

int checkInt(char* input){
	for(int i = 0; i < strlen(input); i++){
		if(isdigit(input[i]) == 0){
			fprintf(stderr, "the error char is [%c]\n", input[i]);
			return -1;
		}
	}
	return atol(input);
}

double checkDouble(char* input){
	for(int i = 0; i < strlen(input); i++){
		if(isdigit(input[i]) == 0 && input[i] != '.'){
			fprintf(stderr, "the error char is [%c]\n", input[i]);
			return -1;
		}
	}
	return (double)atof(input);
}

double expRandom(double lambda, int threshold){
	
	int trigger = 0;
	double result = -1;
	while(trigger == 0){
		//this part of code is from lecture notes
		double r = drand48();
		result = (double)(-(log(r))) / lambda;
		if(result < threshold){ trigger = 1; }
	}
	
	return result;
}

void testfunct(double *input){
	
	*input = 10.0;
	printf("input inside function is [%f]\n", *input);
	
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

/*
int qadd(char* array, int* size, char info){
	
	if(array[*size] != NULL){return 1;}
	
	array[*size] = info;
	(*size) += 1;
	return 0;
	
}

char qpop(char* array, int* size){
	
	if(array[*size] == 0){return NULL;}
	
	char result = array[0];
	for(int i = 0; i < size-1; i++){
		array[i] = array[i+1];
	}
	array[*size-1] = NULL;
	*size -= 1;
	return result;
	
}
*/

int main(int argc, char** argv){
	
	int seed = 14;
	double lambda = 0.002;
	int threshold = 3000;
	int processNum = 5;
	int textSwitchTime = 132;
	double alpha = 0.25;
	int timeSlice = 15 ;
	char* addPosition = "END";
	char* nameString = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	
	srand48(seed);
	//printf("drand48 returns [%f]\n", (-log(drand48()))/ 0.002);
	//printf("The expRandom returns [%lf] \n", expRandom(lambda,threshold));
	// the following part is logic for random 
	
	Process* p = calloc(1,sizeof(Process));
	createProcess(p, 'C', lambda, threshold);
	
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
	
	freeProcess(p);
	
	/*
	TestStruct* t = calloc(1, sizeof(TestStruct));
	t->a = 10;
	t->b = calloc(t->a, sizeof(int));
	(t->b)[1] = 10;
	free(t->b);
	free(t);
	*/
	
	/*
	char* c = calloc(5, sizeof(char));
	for(int i =0; i < 5; i++){
		if(c[i] == NULL){
			printf("at position i in the array, the char is [%ld]\n", c[i]);
		}
		
	}
	
	free(c);
	*/
}