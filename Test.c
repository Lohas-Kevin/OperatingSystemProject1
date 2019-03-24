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
	//number of bursts;
	int burst;
	//These two pointers indicates how many bursts or io
	//have been finished
	int burstPointer;
	int ioPointer;
	//the arrays to store the infomation
	int* burstTime;
	int* ioTime;
	//The three counter to track the total time;
	int cpuTime;
	int turnaroundTime;
	int waitTime;
	//over here, -1 is finishd, 0 is in ready queue
	//1 is not arrived, 2 is in io ;
	//3 is in cpu burst, 4 is switch out
	//5 is switch in
	int status;
	//timer is used for decreament
	//basically, we copy the info here to decreament.
	int timer;
	int tau;
	int returnStatus;
	int preemp;
	int preempTime;
} Process;




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
	
	//set tau to default value 1/lambda
	p->tau = (int)(ceil(1.0/lambda));
	p->timer = p->arriveTime;
	p->status = 1;
	
	
	
}


void freeProcess(Process* p){
	free(p->burstTime);
	free(p->ioTime);
	free(p);
}

int calculateTau(double alpha, int tau0, int time){
	double tau1 = (alpha*(double)time) + ((1-alpha)*(double)tau0);
	return (int)(ceil(tau1));
}

int decreament(Process* p){
	if(p->status == -1){
		//if the process has ended
		return -1;
	}
	//check the state
	if(p->status == 0){
		//the only situation we don't need to do
		//the decreament is in ready queue
		//so we will only add the turnaroundTime and Wait Time
		(p->waitTime) += 1;
		(p->turnaroundTime) += 1;
		//return 0 means no status change
		return 0;
	}else{
		//in other situations we need to decreament the TImer
		
		if(p->status == 1){
			//if status == 1, not arrived
			p->timer -= 1;
			if(p->timer == 0){
				//if after decreament, timer == 0, that means 
				//this process has arrived and we need to take some actions;
				//return 1 means the process has arrived
				return 1;
			}
			//if timer doesn't equals 0
			//return no status changes
			return 0;
		}
		if(p->status == 3){
			//if the in cpu burst status equals 1
			//then the process is now ran by the cpu
			p->timer -= 1;
			p->turnaroundTime += 1;
			p->cpuTime += 1;
			if(p->timer == 0){
				//over here, return 3 indicates that 
				//cpu burst has ended;
				return 3;
			}
			
			//there is no status change
			return 0;
		}
		if(p->status == 2){
			//if the in io burst status equals 1
			//then the process is now in io
			p->timer -= 1;
			p->turnaroundTime += 1;
			if(p->timer == 0){
				//over here, return 2 indicates that 
				//io has ended;
				return 2;
			}
			
			//there is no status change
			return 0;
		}
		if(p->status == 4){
			//context switch out
			p->timer -= 1;
			p->turnaroundTime += 1;
			if(p-> timer == 0){
				//switch out finished
				return 4;
			}
			
			//there is no status change
			return 0;
		}
		if(p->status == 5){
			p->timer -= 1;
			p->turnaroundTime +=1;
			if(p->timer == 0){
				//switch in finished;
				return 5;
			}
			
			return 0;
		}
		
		
	}
	return -10;
}



int qadd(Process** array, int* size, Process* info){
	
	if(array[*size] != NULL){return 1;}
	
	array[*size] = info;
	(*size) += 1;
	return 0;
	
}

Process* qpop(Process** array, int* size){
	
	if(array[0] == NULL){return NULL;}
	
	Process* result = array[0];
	for(int i = 0; i < (*size)-1; i++){
		array[i] = array[i+1];
	}
	array[*size-1] = NULL;
	*size -= 1;
	return result;
	
}

void ProcessViewer(Process* p){
	printf("the name of p is [%c]\n", p->name);
	printf("the arriveTime of p is [%i]\n", p->arriveTime);
	printf("the burst of p is [%i]\n", p->burst);
	printf("the burstPointer of p is [%i]\n", p->burstPointer);
	printf("the ioPointer of p is [%i]\n", p->ioPointer);
	printf("the cpuTime of p is [%i]\n", p->cpuTime);
	printf("the turnaroundTime of p is [%i]\n", p->turnaroundTime);
	printf("the Status of p is [%i]\n", p->status);
	printf("the timer of p is [%i]\n", p->timer);
	printf("the tau of p is [%i]\n", p->tau);
	
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
}

int queueComparator(const void *p1, const void* p2){
	const Process * l = *((Process**) p1);
	const Process * r = *((Process**) p2);
	
	if(l->tau == r->tau){
		return (l->name - r->name);
	}else{
		return (l->tau - r->tau);
	}
}

int printComparator(const void *p1, const void*p2){
	int l = (*((Process**) p1))->returnStatus;
	int r = (*((Process**) p2))->returnStatus;
	
	if(l > r){
		return -1;
	}else if(l == r){
		char lc = (*((Process**) p1))->name;
		char rc = (*((Process**) p2))->name;
		if(lc < rc){return -1;}
		if(lc == rc){return 0;}
		if(lc > rc){return 1;}
	}
	return 1;
}

void printQueue(Process** queue, int qCount){
	
	if(qCount == 0 ){
		//queue is empty
		printf("[Q <empty>]\n");
	}else{
		printf("[Q");
		for(int i = 0; i < qCount; i++){
			printf(" %c",queue[i]->name);
		}
		printf("]\n");
	}
	
}

void printInfo(Process* p, Process** queue, int totalTime, int* qCount, int textSwitchTime, double alpha, int sort){
	int v = p->returnStatus;
	if(v == 3){
		//cpu burst end
		//if this in not the final burst
		p->status = 4;
		p->timer = textSwitchTime/2;
		p->tau = calculateTau(alpha, p->tau, p->burstTime[p->burstPointer]);
		p->burstPointer += 1;
		
		//print info to note that finished the cpu burst
		int remainingBurst = (p->burst) - (p->burstPointer);
		printf("time %dms: Process %c completed a CPU Burst; %d bursts to go ",totalTime, p->name, remainingBurst);
		printQueue(queue, *qCount);
		
		//recalculate the tau
		printf("time %dms: Recalculate tau = %dms for Process %c ", totalTime, p->tau, p->name);
		printQueue(queue, *qCount);
		
		//print switch out infomation
		int finishTime = p->timer + totalTime + p->ioTime[p->ioPointer];
		printf("time %dms: Process %c switching out of CPU; will block on I/O until time %dms ",totalTime, p->name, finishTime);
		printQueue(queue, *qCount);
		
	}else if(v == 2){
		//io end
		p->status = 0;
		qadd(queue, qCount, p);
		p->ioPointer += 1;
		if(sort == 1){
			qsort(queue, *qCount, sizeof(Process*), queueComparator);
		}
		
		//
		
		printf("time %dms: Process %c (tau %dms) completed I/O; added to ready queue",totalTime,p->name,p->tau);
		printQueue(queue, *qCount);
	}else if(v == 1){
		//arrive
		//do the change in function to avoid queue print bug
		//so we can add queue then print.
		p->status = 0;
		qadd(queue, qCount, p);
		if(sort == 1){
			qsort(queue, *qCount, sizeof(Process*), queueComparator);
		}
		//then print the infomation
		printf("time %dms: Process %c (tau %dms) arrived; added to ready queue ",totalTime, p->name, p->tau);
		printQueue(queue, *qCount);
	}else if(v == 5){
		//start using cpu
		p->status = 3;
		p->timer = p->burstTime[p->burstPointer];
		
		printf("time %dms: Process %c started using the CPU for %dms burst ",totalTime, p->name, p->timer);
		printQueue(queue, *qCount);
	}else if(v == -1){
		//Process end
		p->status = 4;
		p->timer = textSwitchTime/2;
		printf("time %dms: Process %c terminated ",totalTime, p->name);
		printQueue(queue, *qCount);
	}
}

int checkPreemp(Process* p, Process* inCPU){
	//0 for not preempt
	//1 for preempt now
	//2 for preempt after context switch
	if(inCPU == NULL){
		return 0;
	}else{
		int e1 = 0;
		int e2 = 0;
		if(p->preemp == 1){
			e1 = p->burstTime[p->burstPointer] - p->preempTime;
		}
		
		
		
		//check the status of inCPU process
		if(inCPU->status == 3){
			//if inCPU is using CPU
			e2 = inCPU->burstTime[inCPU->burstPointer] - inCPU->timer;
			if(((p->tau)-e1) < ((inCPU->tau)-e2)){
				return 1;
			}else{
				return 0;
			}
		}if(inCPU->status == 5){
			//if inCPU is switching in
			if(inCPU->preemp == 1){
				e2 = inCPU->burstTime[inCPU->burstPointer]-inCPU->preempTime;
			}
			if(((p->tau)-e1) < ((inCPU->tau)-e2)){
				return 2;
			}else{
				return 0;
			}
		}
	}
	//the remaining conditions should return 0
	return 0;
}

void printInfoWithPreemp(Process* p, Process** queue, int totalTime, int* qCount, int textSwitchTime, double alpha, int sort, Process* inCPU){
	int v = p->returnStatus;
	if(v == 3){
		//cpu burst end
		//if this in not the final burst
		p->status = 4;
		p->timer = textSwitchTime/2;
		p->tau = calculateTau(alpha, p->tau, p->burstTime[p->burstPointer]);
		p->burstPointer += 1;
		
		//print info to note that finished the cpu burst
		int remainingBurst = (p->burst) - (p->burstPointer);
		printf("time %dms: Process %c completed a CPU Burst; %d bursts to go ",totalTime, p->name, remainingBurst);
		printQueue(queue, *qCount);
		
		//recalculate the tau
		printf("time %dms: Recalculate tau = %dms for Process %c ", totalTime, p->tau, p->name);
		printQueue(queue, *qCount);
		
		//print switch out infomation
		int finishTime = p->timer + totalTime + p->ioTime[p->ioPointer];
		printf("time %dms: Process %c switching out of CPU; will block on I/O until time %dms ",totalTime, p->name, finishTime);
		printQueue(queue, *qCount);
		
	}else if(v == 2){
		//io end
		p->status = 0;
		qadd(queue, qCount, p);
		p->ioPointer += 1;
		if(sort == 1){
			qsort(queue, *qCount, sizeof(Process*), queueComparator);
		}
		
		//we should check the remaining time
		int r = checkPreemp(p, inCPU);
		if(totalTime<410 && totalTime>402){
			printf("r = %d\n",r); 
		}
		if(r == 0){
			//if no preemp
			printf("time %dms: Process %c (tau %dms) completed I/O; added to ready queue ",totalTime,p->name,p->tau);
			printQueue(queue, *qCount);
		}else if(r == 1){
			//if preemp without wait
			inCPU->status = 4;
			inCPU->preempTime = inCPU->timer;
			inCPU->timer = textSwitchTime/2;
			inCPU->preemp = 1;
			printf("time %dms: Process %c (tau %dms) completed I/O and will preempt %c ", totalTime, p->name, p->tau, inCPU->name);
			printQueue(queue, *qCount);
			//also, we need to add the old process to the queue
			//in switch out status
		}else if(r == 2){
			//if preemp with wait
			//leave the remaining action to
			//switch in end part
			inCPU->preemp = 2;
		}
		
	}else if(v == 1){
		//arrive
		//do the change in function to avoid queue print bug
		//so we can add queue then print.
		p->status = 0;
		qadd(queue, qCount, p);
		if(sort == 1){
			qsort(queue, *qCount, sizeof(Process*), queueComparator);
		}
		
		//then print the infomation
		printf("time %dms: Process %c (tau %dms) arrived; added to ready queue ",totalTime, p->name, p->tau);
		printQueue(queue, *qCount);
	}else if(v == 5){
		//start using cpu
		if(p->preemp == 0){
			p->status = 3;
			p->timer = p->burstTime[p->burstPointer];
		
			printf("time %dms: Process %c started using the CPU for %dms burst ",totalTime, p->name, p->timer);
			printQueue(queue, *qCount);
		}else if(p->preemp == 2){
			//if we need to switch out once
			//the switch in finished
			p->status = 4;
			inCPU->preempTime = p->burstTime[p->burstPointer];
			inCPU->timer = inCPU->preempTime;
			
			printf("time %dms: Process %c started using the CPU for %dms burst ",totalTime, p->name, p->timer);
			printQueue(queue, *qCount);
			inCPU->timer = textSwitchTime/2;
			printf("time %dms: Process %c (tau %dms) will preempt %c ",totalTime, queue[0]->name, queue[0]->tau, p->name);
			printQueue(queue, *qCount);
		}else if(p->preemp == 1){
			//if the process resumes using CPU
			p->status = 3;
			p->timer = p->preempTime;
			p->preemp = 0;
		
			printf("time %dms: Process %c started using the CPU with %dms remaining ",totalTime, p->name, p->timer);
			printQueue(queue, *qCount);
		}
		
	}else if(v == -1){
		//Process end
		p->status = 4;
		p->timer = textSwitchTime/2;
		printf("time %dms: Process %c terminated ",totalTime, p->name);
		printQueue(queue, *qCount);
	}
}

void SJF(Process** ProcessArray, int processNum, int textSwitchTime, double alpha, int sort){
	//the following part is for SJF
	int tracker = 0;
	int totalTime = 0;
	Process** queue = calloc(processNum, sizeof(Process*));
	int queueLength = 0;
	Process* inCPU = NULL;
	
	printf("time 0ms: Simulator started for SJF [Q <empty>]\n");
	while(tracker != processNum){
		totalTime++;
		Process** tempArray = calloc(processNum, sizeof(Process*));
		int tempLength = 0;
		for(int i = 0; i < processNum; i++){
			//handle the decreament signal here
			int temp = decreament(ProcessArray[i]);
			if(temp == 1){
				//process arrived
				ProcessArray[i]->returnStatus = 1;
				tempArray[tempLength] = ProcessArray[i];
				tempLength += 1;
				
				//add the process direcly to ready queue
				ProcessArray[i]->status = 0;
				qadd(queue, &queueLength, ProcessArray[i]);
				
			}else if(temp == 2){
				//io end
				ProcessArray[i]->returnStatus = 2;
				tempArray[tempLength] = ProcessArray[i];
				tempLength += 1;
				
				//we directly add the process to ready queue
				ProcessArray[i]->status = 0;
				qadd(queue, &queueLength, ProcessArray[i]);
				ProcessArray[i]->ioPointer += 1;
				
			}else if(temp == 3){
				//cpu burst end
				if(ProcessArray[i]->burstPointer < ((ProcessArray[i]->burst) - 1)){
					ProcessArray[i]->returnStatus = 3;
					tempArray[tempLength] = ProcessArray[i];
					tempLength += 1;
				
					//we start switch out
					ProcessArray[i]->status = 4;
					ProcessArray[i]->timer = textSwitchTime/2;
					//recalculate tau
					ProcessArray[i]->tau = calculateTau(alpha, ProcessArray[i]->tau, 
						ProcessArray[i]->burstTime[ProcessArray[i]->burstPointer]);
					ProcessArray[i]->burstPointer += 1;
				}
				//add a special case, the process ends the last burst
				else{
					ProcessArray[i]->status = -1;
					//print the end info here
					ProcessArray[i]->returnStatus = -1;
					printInfo(ProcessArray[i], queue, totalTime, &queueLength, textSwitchTime, alpha, 1);
					//don't forget to move to context switch out
					ProcessArray[i]->status = 4;
					ProcessArray[i]->timer = textSwitchTime/2;
				}
				
				
				
			}else if(temp == 4){
				//switch out end
				//we don't need to print any info here
				//we perform the io
				//also set the cpu to not be used
				//PROBLEM HERE 
				if(ProcessArray[i] -> returnStatus == 3){
					//if the process has not ended
					ProcessArray[i]->status = 2;
					ProcessArray[i]->timer = ProcessArray[i]->ioTime[ProcessArray[i]->ioPointer];
					inCPU = NULL;
				}else if(ProcessArray[i] -> returnStatus == -1){
					//if the process has ended
					ProcessArray[i]->status = -1;
					inCPU = NULL;
					ProcessArray[i]->timer = -1;
					tracker += 1;
				}
				
				
				
			}else if(temp == 5){
				//switch in end
				ProcessArray[i]->returnStatus = 5;
				tempArray[tempLength] = ProcessArray[i];
				tempLength += 1;
				
				//perform cpu burst
				ProcessArray[i]->status = 3;
				ProcessArray[i]->timer = ProcessArray[i]->burstTime[ProcessArray[i]->burstPointer];
				
			}
		}
		//after all increament
		//check the print info
		//1. nothing happened
		if(tempLength == 0){free(tempArray);}
		else{
			qsort(tempArray, tempLength, sizeof(Process*), printComparator);
			for(int i = 0; i < tempLength; i++){
				printInfo(tempArray[i], queue, totalTime, &queueLength, textSwitchTime, alpha, 1);
			}
		}
		
		//After printing out every info
		//we choose the next process in cpu
		if(queueLength > 0 && inCPU == NULL){
			inCPU = qpop(queue, &queueLength);
			//set the process we chosed to context switch in state
			inCPU->status = 5;
			inCPU->timer = (textSwitchTime / 2);
		}
	}
	printf("time %dms: Simulator ended for SJF [Q <empty>]\n",totalTime);
	
	
}


int main(int argc, char** argv){
	
	int seed = 2;
	double lambda = 0.01;
	int threshold = 200;
	int processNum = 2;
	int textSwitchTime = 4;
	double alpha = 0.5;
	int timeSlice = 120 ;
	char* addPosition = "END";
	char* nameString = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	
	
	//printf("drand48 returns [%f]\n", (-log(drand48()))/ 0.002);
	//printf("The expRandom returns [%lf] \n", expRandom(lambda,threshold));
	// the following part is logic for random 
	Process** ProcessArray = calloc(processNum, sizeof(Process*));
	srand48(seed);
	
	//Process** test = calloc(processNum, sizeof(Process*))
	for(int i = 0; i < processNum; i++){
		ProcessArray[i] = calloc(1, sizeof(Process));
		createProcess(ProcessArray[i],nameString[i],lambda,threshold);
		printf("Process %c [NEW] (arrival time %d ms) %d CPU bursts\n",
		ProcessArray[i]->name, ProcessArray[i]->arriveTime, ProcessArray[i]->burst);
		//ProcessViewer(ProcessArray[i]);
		
	}
	
	//SJF(ProcessArray, processNum, textSwitchTime, alpha);
	
	//the following part is for SJF
	int tracker = 0;
	int totalTime = 0;
	Process** queue = calloc(processNum, sizeof(Process*));
	int queueLength = 0;
	Process* inCPU = NULL;
	int sort = 1;
	
	printf("time 0ms: Simulator started for SJF [Q <empty>]\n");
	while(tracker != processNum){
		totalTime++;
		Process** tempArray = calloc(processNum, sizeof(Process*));
		int tempLength = 0;
		for(int i = 0; i < processNum; i++){
			//handle the decreament signal here
			int temp = decreament(ProcessArray[i]);
			if(temp == 1){
				//process arrived
				ProcessArray[i]->returnStatus = 1;
				tempArray[tempLength] = ProcessArray[i];
				tempLength += 1;
			}else if(temp == 2){
				//io end
				ProcessArray[i]->returnStatus = 2;
				tempArray[tempLength] = ProcessArray[i];
				tempLength += 1;	
			}else if(temp == 3){
				//cpu burst end
				if(ProcessArray[i]->burstPointer < ((ProcessArray[i]->burst) - 1)){
					ProcessArray[i]->returnStatus = 3;
					tempArray[tempLength] = ProcessArray[i];
					tempLength += 1;
				
				}
				//add a special case, the process ends the last burst
				else{
					ProcessArray[i]->status = -1;
					//print the end info here
					ProcessArray[i]->returnStatus = -1;
					//special case, we call printinfo once the cpu burst ends.
					printInfoWithPreemp(ProcessArray[i], queue, totalTime, &queueLength, textSwitchTime, alpha, 1, inCPU);
					
				}
			}else if(temp == 4){
				//switch out end
				//we don't need to print any info here
				//we perform the io
				//also set the cpu to not be used
				if(ProcessArray[i]->preemp == 0){
					//if the process finish its burst
					if(ProcessArray[i] -> returnStatus == 3){
						//if the process has not ended
						ProcessArray[i]->status = 2;
						ProcessArray[i]->timer = ProcessArray[i]->ioTime[ProcessArray[i]->ioPointer];
						inCPU = NULL;
					}else if(ProcessArray[i] -> returnStatus == -1){
						//if the process has ended
						ProcessArray[i]->status = -1;
						inCPU = NULL;
						ProcessArray[i]->timer = -1;
						tracker += 1;
					}
				}else if(ProcessArray[i]->preemp == 1){
					//if switch out immediately
					ProcessArray[i]->status = 0;
					qadd(queue, &queueLength, ProcessArray[i]);
					if(sort == 1){
						qsort(queue, queueLength, sizeof(Process*), queueComparator);
					}
					inCPU = NULL;
				}else if(ProcessArray[i]->preemp == 2){
					//if switch out after switch in
					//set the preemp to 1 in order to
					//run the process normally 
					ProcessArray[i]->preemp = 0;
					ProcessArray[i]->status = 0;
					qadd(queue, &queueLength, ProcessArray[i]);
					if(sort == 1){
						qsort(queue, queueLength, sizeof(Process*), queueComparator);
					}
					inCPU = NULL;
				}
				
			}else if(temp == 5){
				//switch in end
				ProcessArray[i]->returnStatus = 5;
				tempArray[tempLength] = ProcessArray[i];
				tempLength += 1;	
			}
		}

		
		//after all increament
		//check the print info
		//1. nothing happened
		if(tempLength == 0){free(tempArray);}
		else{
			qsort(tempArray, tempLength, sizeof(Process*), printComparator);
			for(int i = 0; i < tempLength; i++){
				printInfoWithPreemp(tempArray[i], queue, totalTime, &queueLength, textSwitchTime, alpha, 1, inCPU);
			}
		}
		
		//After printing out every info
		//we choose the next process in cpu
		if(queueLength > 0 && inCPU == NULL){
			inCPU = qpop(queue, &queueLength);
			//set the process we chosed to context switch in state
			inCPU->status = 5;
			inCPU->timer = (textSwitchTime / 2);
		}
	}
	printf("time %dms: Simulator ended for SJF [Q <empty>]\n",totalTime);
	
	
	
	for(int i = 0; i < processNum; i++){
		freeProcess(ProcessArray[i]);
	}
	free(queue);
	free(ProcessArray);
	
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