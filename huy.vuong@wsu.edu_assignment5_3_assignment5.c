#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/sem.h>

const int n_philosophers = 5; //number of philosophers 
const int max_time_eat = 100; //max time philosophers gonna eat

// mean and stdev, will be used in randomGaussian
const int mean_eat = 9;
const int stdev_eat = 3;
const int mean_think = 11;
const int stdev_think = 7;

//id for semaphore
int chopsticks;
//used to keep track if all philosophers were initiated
//start process when this int set to 0
int not_at_table;


//function provided in handout
int randomGaussian(int mean, int stddev) {
	double mu = 0.5 + (double) mean;
	double sigma = fabs((double) stddev);
	double f1 = sqrt(-2.0 * log((double) rand() / (double) RAND_MAX));
	double f2 = 2.0 * 3.14159265359 * (double) rand() / (double) RAND_MAX;
	if (rand() & (1 << 5))
		return (int) floor(mu + sigma * cos(f2) * f1);
	else
		return (int) floor(mu + sigma * sin(f2) * f1);
}

//code for child process
int philosophers(int n){
	int i, j, first, second;

	//used to keep track of time eats
	//make sure a philosopher don't eat more than 100 seconds
	int seconds_eaten = 0;

	struct sembuf op;
	op.sem_flg = 0;
	srand(n); // seed random number 

	//2 chopsticks 
	first = (n < n_philosophers) ? n : 0;
	second = (n < n_philosophers) ? n+1 : n_philosophers-1;

	//philosopher showed up at table
	op.sem_op = -1;
	op.sem_num = 0;
	semop(not_at_table, &op, 1);
	printf("Philosopher %d showed up at table\n", n);
	fflush(stdout);

	//wait for other philosophers
	op.sem_op = 0;
	op.sem_num = 0;

	semop(not_at_table, &op, 1);

	//Loop for eating and thinking cycle
	while(seconds_eaten <= max_time_eat){
		int eat_time = randomGaussian(mean_eat, stdev_eat);

		printf("Philosopher %d is thinking...\n", n);
		fflush(stdout);
		sleep(randomGaussian(mean_think, stdev_think));


		//Pick up chopsticks
		op.sem_op = -1;
		op.sem_num = first;
		semop(chopsticks, &op, 1);
		op.sem_op = -1;
		op.sem_num = second;
		semop(chopsticks, &op, 1);
		seconds_eaten += eat_time;

		printf("Philosopher %d begins eating now...\n", n);
		sleep(eat_time);
		fflush(stdout);
		printf("Philosopher %d has eaten for %d seconds\n", n, eat_time);
		printf("Philosopher %d total eat_time til now: %d\n", n, seconds_eaten);
		fflush(stdout);
		//Put down chopsticks
		op.sem_op = +1;
		op.sem_num = first;
		semop(chopsticks, &op, 1);
		op.sem_op = +1;
		op.sem_num = second;
		semop(chopsticks, &op, 1);

	}
	printf("Philosopher %d has eaten for 100 seconds, left the table now\n", n);
	exit(n);

}

int main(int argc, char *argv[]){
	/* code */
	int i, status;
	int ctlerr;
	pid_t phil[n_philosophers];

	//allocate chopsticks, 5 chopsticks in array
	chopsticks = semget(IPC_PRIVATE, 1, IPC_CREAT | 0600);

	for(i = 0; i < n_philosophers; i++){
		semctl(chopsticks, i, SETVAL, 1);
		
	}
	not_at_table = semget(IPC_PRIVATE, 1, IPC_CREAT | 0600);

	//check for semget error
	if(chopsticks == -1 || not_at_table == -1){
		fprintf(stderr, "%s\n", strerror(errno));
		exit(1);
	}

	//prevent child process from starting
	semctl(not_at_table,0, SETVAL, 5);
	

	for(i = 0; i < n_philosophers; i++){
		int pid = fork();
		if(pid == 0){
			//child act as philosophers
			int ret = philosophers(i);
			exit(ret);
		}
		else{
			//parent track child
			phil[i] = pid;
		}
	}
	//wait for all child to finish
	for(i = 0; i < n_philosophers; i++){
		waitpid(phil[i], &status,0);
	}

	//clean up and check for errors
	ctlerr = semctl(chopsticks, 0, IPC_RMID, 0);
	if(ctlerr == -1){
		fprintf(stderr, "%s\n", strerror(errno));
		exit(1);
	}
	
	ctlerr = semctl(not_at_table, 0, IPC_RMID,0);
	if(ctlerr == -1){
		fprintf(stderr, "%s\n", strerror(errno));
		exit(1);
	}
	

	return 0;
}
