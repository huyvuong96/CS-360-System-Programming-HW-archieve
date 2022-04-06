#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <math.h>

const int n_philosophers = 5; //number of philosophers 
const int max_time_eat = 100; //max time philosophers gonna eat

// mean and stdev, will be used in randomGaussian

const int mean_eat = 9;
const int stdev_eat = 3;
const int mean_think = 11;
const int stdev_think = 7;

//Global variable
#define N_MUTEX 5
static pthread_mutex_t chopstick[N_MUTEX];
int n_phil[5] = {0,1,2,3,4};

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
int pick_up_chopstick(int id){
	if(pthread_mutex_lock(&chopstick[id]) != 0){
		fprintf(stderr, "%s\n", strerror(errno));
		exit(1);
	}
	if(pthread_mutex_trylock(&chopstick[(id +1) %5]) == EBUSY){
		if(pthread_mutex_unlock(&chopstick[id]) != 0){
			fprintf(stderr, "%s\n", strerror(errno));
			exit(1);
		}
		return 1;
	}
	else{
		return 0;
	}
}

void *philosophers_loop(void *philo_id){
	int id = *((int *)philo_id);
	int join_table = 1;
	int seconds_eaten = 0;

	while(seconds_eaten < max_time_eat){
		while(join_table){
			join_table = pick_up_chopstick(id);
			sleep(1);
		}
		int eat_time = randomGaussian(mean_eat, stdev_eat);
		seconds_eaten += eat_time;
		printf("Philosopher %d begins eating now\n", id);
		fflush(stdout);
		sleep(eat_time);
		printf("Philosopher %d has eaten for %d seconds\n", id, eat_time);
		printf("Total time eaten for philosopher %d till now: %d\n", id, seconds_eaten);
		fflush(stdout);

		if(pthread_mutex_unlock(&chopstick[id]) != 0 || pthread_mutex_unlock(&chopstick[(id +1) %5]) != 0){
			fprintf(stderr, "%s\n", strerror(errno));
			exit(1);
		}

		int think_time = randomGaussian(mean_think, stdev_think);
		printf("Philosopher %d begins thinking now\n", id);
		fflush(stdout);
		sleep(think_time);

	}

	printf("Philosopher %d has eaten for 100 seconds, leave table now\n", id);
}


int main(){
	pthread_t philo[n_philosophers];

	//create mutex for 5 chopsticks
	for(int i = 0; i < n_philosophers; i++){
		if(pthread_mutex_init(&chopstick[i], NULL) != 0){
			fprintf(stderr, "%s\n", strerror(errno));
			exit(1);
		}
	}

	//create thread for 5 philosophers
	for(int j =0; j < n_philosophers; j++){
		if(pthread_create(&philo[j], NULL, philosophers_loop, &n_phil[j]) != 0){
			fprintf(stderr, "%s\n", strerror(errno));
			exit(1);
		}
	}

	//join and clean up thread after process is done
	for(int z = 0; z < n_philosophers; z++){
		pthread_join(philo[z], NULL);
	}

	printf("All philosophers have done eating\n");

	return 0;
}