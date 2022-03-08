#include <stdio.h>
#include <stdlib.h>
#include "genetic_algorithm.h"


int main(int argc, char *argv[]) {
	// array with all the objects that can be placed in the sack
	sack_object *objects = NULL;

	// number of objects
	int object_count = 0;

	// maximum weight that can be carried in the sack
	int sack_capacity = 0;

	// number of generations
	int generations_count = 0;

	int NUM_THREADS = 0;
	int r;

	if (!read_input(&objects, &object_count, &sack_capacity, &generations_count, &NUM_THREADS, argc, argv)) {
		return 0;
	
	}

	pthread_t *threads;
	struct my_arg *arguments;
	threads = (pthread_t*) malloc(NUM_THREADS * sizeof(pthread_t));
	arguments = (struct my_arg*) malloc(NUM_THREADS * sizeof(struct my_arg));
	pthread_barrier_t barrier;
	pthread_mutex_t mutex;
	pthread_mutex_init(&mutex, NULL);
	pthread_barrier_init(&barrier,NULL,NUM_THREADS);
	individual *current_generation = (individual*) calloc(object_count, sizeof(individual));
	individual *next_generation = (individual*) calloc(object_count, sizeof(individual));


	int *flagprint = (int*) calloc(generations_count, sizeof(int));
	int *sorted = (int*)calloc(generations_count,sizeof(int));
	int *odd = (int*)calloc(generations_count,sizeof(int));
	int final = 0;

	for (int i = 0; i < NUM_THREADS; i++)
	{
		arguments[i].id = i;
		arguments[i].N = object_count;
		arguments[i].P = NUM_THREADS;
		arguments[i].objects = objects;
		arguments[i].sack_capacity = sack_capacity;
		arguments[i].generations_count = generations_count;
		arguments[i].current_generation = current_generation;
		arguments[i].next_generation = next_generation;
		arguments[i].mutex = &mutex;
		arguments[i].barrier = &barrier;
		arguments[i].flagprint = flagprint;
		arguments[i].sorted = sorted;
		arguments[i].final = &final;
		arguments[i].thread_part = object_count/NUM_THREADS;
		arguments[i].odd = odd;
		r = pthread_create(&threads[i], NULL, run_genetic_algorithm, &arguments[i]);

		if (r) {
			printf("Eroare la crearea thread-ului %d\n", i);
			exit(-1);
		}
	}

	for (int i = 0; i < NUM_THREADS; i++) {
		r = pthread_join(threads[i], NULL);

		if (r) {
			printf("Eroare la asteptarea thread-ului %d\n", i);
			exit(-1);
		}
	}
	
	//free resources for old generation
	free_generation(current_generation);
	free_generation(next_generation);

	// free resources
	free(current_generation);
	free(next_generation);
	free(objects);

	return 0;
}
