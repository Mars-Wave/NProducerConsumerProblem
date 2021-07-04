#include <stdio.h>
#include <pthread.h>
#include <stddef.h>
#include <semaphore.h>
#include <math.h>
#include <time.h>
//gcc -lpthread -o m.o main.c

#define BUFFERSIZE 3
#define PRODUCERS 5
#define CONSUMERS 2
#define PRODUCERSPEED 0
#define CONSUMERSPEED 0

void *producer(void *);
void *consumer(void *);
int max(int a, int b){
	if (a>b)
	{
		return a;
	}
	return b;
}


sem_t sem_emptyslots, sem_fullslots; //controls buffer being completely empty or completely full
pthread_mutex_t mutexbuffer; //controls access to count and buffer

int buf[BUFFERSIZE];
int count;

int main(void)
{
	srand(time(NULL));

	pthread_mutex_init(&mutexbuffer, NULL);
	sem_init(&sem_emptyslots, 0, BUFFERSIZE); //All slots empty
	sem_init(&sem_fullslots, 0, 0);	//no slots full

	pthread_t ths[PRODUCERS + CONSUMERS]; //all threads

	//INIT OF THREADS
	int prods = 0;
	int cons = 0;
	for (int i = 0; i < max(PRODUCERS,CONSUMERS);)
	{
		if(prods<PRODUCERS){
			if(pthread_create(&ths[i], NULL, &producer, NULL)!=0){
				printf("FAILURE CREATING THREAD"); exit(-1);
			}
			i++;
			prods++;
		}
		if (cons<CONSUMERS)
		{
			if( pthread_create(&ths[i], NULL, &consumer, NULL)!=0){
				printf("FAILURE CREATING THREAD"); exit(-1);
			}	
			i++;
			cons++;
		}
	}


	//JOINING OF THREADS
	printf("Threads joining...\n\n");

	for (int i = 0; i < PRODUCERS+CONSUMERS; i++)
	{
		if (pthread_join(&ths[i], NULL )!=0)
		{
			printf("FAILURE JOINING THREAD"); exit(-1);
		}
		
	}
	

	pthread_mutex_destroy(&mutexbuffer);
	sem_destroy(&sem_emptyslots);
	sem_destroy(&sem_fullslots);
	
	
	exit(0);
}

void *producer(void *arg)
{
	while(1){
		int prod = rand()%1000;

		sem_wait(&sem_emptyslots); //Wait until there is at least one empty slot to fill
		pthread_mutex_lock(&mutexbuffer); //account for concurrent modification of buf and count variables
		
		buf[count] = prod; //LIFO implementation of the production for the buffer so some order (in this case LIFO) is respected as per statement, but FIFO can be achieved as well 
		count++;
		
		printf("produced %d\n", prod); //this way it is printable that the produced will be consumed in order
		
		sleep(PRODUCERSPEED);  //account for producing speed

		pthread_mutex_unlock(&mutexbuffer); 
		sem_post(&sem_fullslots); //notify that now there is at least one full slot
		
	}
}

void *consumer(void *arg)
{
	while(1){
		sem_wait(&sem_fullslots); //Wait until there is at least one full slot to consume
		pthread_mutex_lock(&mutexbuffer); //account for concurrent modification of buf and count variables
		
		int cons = buf[count-1]; //LIFO order of consumption 
		count--;

		printf("consumed %d\n", cons);

		sleep(CONSUMERSPEED); //account for speed of consumption
		pthread_mutex_unlock(&mutexbuffer); 
		sem_post(&sem_emptyslots); //notify that now there is at least one empty slot to fill
	}
}