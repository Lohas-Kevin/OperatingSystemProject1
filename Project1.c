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
	
	//set tau to default value 1/lambda
	p->tau = (int)(ceil(1.0/lambda));
	p->timer = p->arriveTime;
	p->status = 1;
	
	
}

int calculateTau(double alpha, int tau0, int time){
	double tau1 = (alpha*(double)time) + ((1-alpha)*(double)tau0);
	return (int)(ceil(tau1));
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
	printf("the burstPointer of p is [%i]\n", p->burstPointer);
	printf("the ioPointer of p is [%i]\n", p->ioPointer);
	printf("the cpuTime of p is [%i]\n", p->cpuTime);
	printf("the turnaroundTime of p is [%i]\n", p->turnaroundTime);
	printf("the waitTime of p is [%i]\n", p->waitTime);
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

int queueComparator(const void *p1, const void* p2){
	const Process * l = *((Process**) p1);
	const Process * r = *((Process**) p2);
	
	int e1 = 0;
	int e2 = 0;
	if(l->preemp == 1){
		e1 = l->burstTime[l->burstPointer] - l->preempTime;
	}
	if(r->preemp == 1){
		e2 = r->burstTime[r->burstPointer] - r->preempTime;
	}
	
	if(((l->tau) - e1 )== ((r->tau) - e2)){
		return (l->name - r->name);
	}else{
		return (((l->tau) - e1 ) - ((r->tau) - e2));
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
		if(remainingBurst > 1){
			printf("time %dms: Process %c completed a CPU burst; %d bursts to go ",totalTime, p->name, remainingBurst);
			printQueue(queue, *qCount);
		}else if(remainingBurst == 1){
			printf("time %dms: Process %c completed a CPU burst; %d burst to go ",totalTime, p->name, remainingBurst);
			printQueue(queue, *qCount);
		}
		
		if(sort == 1){
			//recalculate the tau
			printf("time %dms: Recalculated tau = %dms for process %c ", totalTime, p->tau, p->name);
			printQueue(queue, *qCount);
		}
		
		
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
		
		if(sort == 1){
			printf("time %dms: Process %c (tau %dms) completed I/O; added to ready queue ",totalTime,p->name,p->tau);
			printQueue(queue, *qCount);
		}else{
			printf("time %dms: Process %c completed I/O; added to ready queue ",totalTime,p->name);
			printQueue(queue, *qCount);
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
		if(sort == 1){
			printf("time %dms: Process %c (tau %dms) arrived; added to ready queue ",totalTime, p->name, p->tau);
			printQueue(queue, *qCount);
		}else{
			printf("time %dms: Process %c arrived; added to ready queue ",totalTime,p->name);
			printQueue(queue, *qCount);
		}
		
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

void printInfoWithPreemp(Process* p, Process** queue, int totalTime, int* qCount, int textSwitchTime, double alpha,Process* inCPU){
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
		if(remainingBurst > 1){
			printf("time %dms: Process %c completed a CPU burst; %d bursts to go ",totalTime, p->name, remainingBurst);
			printQueue(queue, *qCount);
		}else if(remainingBurst == 1){
			printf("time %dms: Process %c completed a CPU burst; %d burst to go ",totalTime, p->name, remainingBurst);
			printQueue(queue, *qCount);
		}
		
		
		//recalculate the tau
		printf("time %dms: Recalculated tau = %dms for process %c ", totalTime, p->tau, p->name);
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
		qsort(queue, *qCount, sizeof(Process*), queueComparator);
		
		
		//we should check the remaining time
		int r = checkPreemp(p, inCPU);
		
		if(r == 0){
			//if no preemp
			printf("time %dms: Process %c (tau %dms) completed I/O; added to ready queue ",totalTime,p->name,p->tau);
			printQueue(queue, *qCount);
		}else if(r == 1){
			//if preemp without wait
			inCPU->status = 4;
			//special case, if there is a io arrive
			//at the same time as the the cpu start working
			
			inCPU->preempTime = inCPU->timer;
			inCPU->preemp = 1;
			inCPU->timer = textSwitchTime/2;
			
			printf("time %dms: Process %c (tau %dms) completed I/O and will preempt %c ", totalTime, p->name, p->tau, inCPU->name);
			printQueue(queue, *qCount);
			//also, we need to add the old process to the queue
			//in switch out status
		}else if(r == 2){
			//if preemp with wait
			//leave the remaining action to
			//switch in end part
			//a process can be preempted for several times
			inCPU->preemp = 2;
			printf("time %dms: Process %c (tau %dms) completed I/O; added to ready queue ",totalTime,p->name,p->tau);
			printQueue(queue, *qCount);
		}
		
	}else if(v == 1){
		//arrive
		//do the change in function to avoid queue print bug
		//so we can add queue then print.
		p->status = 0;
		qadd(queue, qCount, p);
		qsort(queue, *qCount, sizeof(Process*), queueComparator);
		
		
		int r = checkPreemp(p, inCPU);
		if(r == 0){
			//no preemp
			//then print the infomation
			printf("time %dms: Process %c (tau %dms) arrived; added to ready queue ",totalTime, p->name, p->tau);
			printQueue(queue, *qCount);
		}else if(r == 1){
			//if preemp without wait
			inCPU->status = 4;
			inCPU->preempTime = inCPU->timer;
			inCPU->timer = textSwitchTime/2;
			inCPU->preemp = 1;
			printf("time %dms: Process %c (tau %dms) arrived and will preempt %c ", totalTime, p->name, p->tau, inCPU->name);
			printQueue(queue, *qCount);
		}else if(r == 2){
			//if preemp with wait
			//leave the remaining action to
			//switch in end part
			//a process can be preempted for several times
			inCPU->preemp = 2;
			printf("time %dms: Process %c (tau %dms) arrived; added to ready queue ",totalTime,p->name,p->tau);
			printQueue(queue, *qCount);
		}
		
	}else if(v == 5){
		//start using cpu
		if(p->preemp == 0){
			p->status = 3;
			p->timer = p->burstTime[p->burstPointer];
			p->preempTime = 0;
			printf("time %dms: Process %c started using the CPU for %dms burst ",totalTime, p->name, p->timer);
			printQueue(queue, *qCount);
		}else if(p->preemp == 2){
			//if we need to switch out once
			//the switch in finished
			p->status = 4;
			p->timer = textSwitchTime/2;
			if(p -> preempTime == 0 || (p->burstTime[p->burstPointer] == p->preempTime)){
				printf("time %dms: Process %c started using the CPU for %dms burst ",totalTime, p->name, p->burstTime[p->burstPointer]);
				printQueue(queue, *qCount);
			}else{
				printf("time %dms: Process %c started using the CPU with %dms remaining ",totalTime, p->name, p->preempTime);
				printQueue(queue, *qCount);
			}

			printf("time %dms: Process %c (tau %dms) will preempt %c ",totalTime, queue[0]->name, queue[0]->tau, p->name);
			printQueue(queue, *qCount);
		}else if(p->preemp == 1){
			//if the process resumes using CPU
			p->status = 3;
			p->timer = p->preempTime;
			p->preemp = 0;
			
			if(p->timer != p->burstTime[p->burstPointer]){
				printf("time %dms: Process %c started using the CPU with %dms remaining ",totalTime, p->name, p->timer);
				printQueue(queue, *qCount);
			}else{
				printf("time %dms: Process %c started using the CPU for %dms burst ",totalTime, p->name, p->burstTime[p->burstPointer]);
				printQueue(queue, *qCount);
			}
			
			p->preempTime = 0;
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
	//if sort is 0, then the algo is fcfs
	int tracker = 0;
	int totalTime = 0;
	Process** queue = calloc(processNum, sizeof(Process*));
	int queueLength = 0;
	Process* inCPU = NULL;
	if(sort == 1){
		printf("time 0ms: Simulator started for SJF [Q <empty>]\n");
	}else{
		printf("time 0ms: Simulator started for FCFS [Q <empty>]\n");
	}
	
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
				//in print info function
			}else if(temp == 2){
				//io end
				ProcessArray[i]->returnStatus = 2;
				tempArray[tempLength] = ProcessArray[i];
				tempLength += 1;
				
				//we directly add the process to ready queue
				//in print info function
				
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
					printInfo(ProcessArray[i], queue, totalTime, &queueLength, textSwitchTime, alpha, sort);
					//don't forget to move to context switch out
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
				
			}
		}
		//after all increament
		//check the print info
		//1. nothing happened
		if(tempLength == 0){free(tempArray);}
		else{
			qsort(tempArray, tempLength, sizeof(Process*), printComparator);
			for(int i = 0; i < tempLength; i++){
				printInfo(tempArray[i], queue, totalTime, &queueLength, textSwitchTime, alpha, sort);
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
	if(sort == 1){
		printf("time %dms: Simulator ended for SJF [Q <empty>]\n",totalTime);
	}
	else{
		printf("time %dms: Simulator ended for FCFS [Q <empty>]\n",totalTime);
	}
	free(queue);
	
}

void SRT(Process** ProcessArray, int processNum, int textSwitchTime, double alpha){
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
					printInfoWithPreemp(ProcessArray[i], queue, totalTime, &queueLength, textSwitchTime, alpha, inCPU);
					
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
					qsort(queue, queueLength, sizeof(Process*), queueComparator);
					inCPU = NULL;
				}else if(ProcessArray[i]->preemp == 2){
					//if switch out after switch in
					//two conditions for the process switch out
					//first is the process has been switched
					//out before, we set preemp = 1
					//second is the process has not been switched
					//out before, we set preemp = 0
					if(ProcessArray[i]->preempTime > 0){
						ProcessArray[i]->preemp = 1;
					}else{
						ProcessArray[i]->preemp = 0;
					}
					
					ProcessArray[i]->status = 0;
					qadd(queue, &queueLength, ProcessArray[i]);
					qsort(queue, queueLength, sizeof(Process*), queueComparator);
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
				printInfoWithPreemp(tempArray[i], queue, totalTime, &queueLength, textSwitchTime, alpha, inCPU);
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
	printf("time %dms: Simulator ended for SRT [Q <empty>]\n",totalTime);
	free(queue);
	
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
	if(argc != 9 && argc != 8){
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
	if(argc == 9 ){
		
		if(strcmp("BEGINNING", argv[8]) != 0){
			addPosition = "END";
		}else{
			addPosition = "BEGINNING";
		}
	}else{
			addPosition = "END";
	}
	
	Process** ProcessArray = calloc(processNum, sizeof(Process*));
	srand48(seed);
	
	
	for(int i = 0; i < processNum; i++){
		ProcessArray[i] = calloc(1, sizeof(Process));
		createProcess(ProcessArray[i],nameString[i],lambda,threshold);
		if(ProcessArray[i]->burst > 1){
			printf("Process %c [NEW] (arrival time %d ms) %d CPU bursts\n",
			ProcessArray[i]->name, ProcessArray[i]->arriveTime, ProcessArray[i]->burst);
		}else{
			printf("Process %c [NEW] (arrival time %d ms) %d CPU burst\n",
			ProcessArray[i]->name, ProcessArray[i]->arriveTime, ProcessArray[i]->burst);
		}
	}
	//
	//insert algo here
	//
	SJF(ProcessArray, processNum, textSwitchTime, alpha, 0);
	//free the memory
	for(int i = 0; i < processNum; i++){
		freeProcess(ProcessArray[i]);
	}
	
	
	return EXIT_SUCCESS;
}