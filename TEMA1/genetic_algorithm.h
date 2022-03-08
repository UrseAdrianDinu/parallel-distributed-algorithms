#ifndef GENETIC_ALGORITHM_H
#define GENETIC_ALGORITHM_H

#include "sack_object.h"
#include "individual.h"
#include <pthread.h>
#include <math.h>

// Structura pentru a reprenzenta parametrul 
// primit de functia executata de fiecare thread  
typedef struct my_arg {
	int id; 							// id-ul thread-ului 
	int N;  							// object_count
	int P;  							// NUM_THREADS
	int generations_count; 				// numarul de generatii
	int sack_capacity;     				// capacitatea rucsacului
	sack_object *objects;  				// obiectele ce pot fi puse in rucsac
    individual *current_generation; 	// generatia curenta de indivizi				
	individual *next_generation;		// noua generatie de indivizi
    pthread_barrier_t *barrier;			// bariera partajata
	pthread_mutex_t *mutex;				// mutex partajat

	int *flagprint;						// vector folosit pentru a printa o singura data
										// best fitness 

	int *sorted;						// vector folosit pentru a aplica o singura data
										// merge-ul final pentru fiecare generatie
	
	int *final;							// flag folosit pentru a aplica o singura data
										// merge-ul final pentru ultima generatie
										// si pentru a afisa o singura data best fitness
										
	int thread_part;					// numarul de elemente alocate fiecarui thread

	int *odd;							// flag folosit pentru a copia o singura data
										// ultimul individ in cazul in care exista
										// un numar impar de parinti

	int *gen_switch;

}my_arg;

// reads input from a given file
int read_input(sack_object **objects, int *object_count, int *sack_capacity, int *generations_count, int *NUM_THREADS, int argc, char *argv[]);

// displays all the objects that can be placed in the sack
void print_objects(const sack_object *objects, int object_count);

// displays all or a part of the individuals in a generation
void print_generation(const individual *generation, int limit);

// displays the individual with the best fitness in a generation
void print_best_fitness(const individual *generation);

// computes the fitness function for each individual in a generation
void parallel_compute_fitness_function(const sack_object *objects, individual *generation, int object_count, int sack_capacity, int start ,int end);

// compares two individuals by fitness and then number of objects in the sack (to be used with qsort)
int cmpfunc(individual *first, individual *second);

// performs a variant of bit string mutation
void mutate_bit_string_1(const individual *ind, int generation_index);

// performs a different variant of bit string mutation
void mutate_bit_string_2(const individual *ind, int generation_index);

// performs one-point crossover
void crossover(individual *parent1, individual *child1, int generation_index);

// copies one individual
void copy_individual(const individual *from, const individual *to);

// deallocates a generation
void free_generation(individual *generation);

// runs the genetic algorithm
void *run_genetic_algorithm(void *arg);

// interclaseaza 2 vectori
void merge(individual *current_generation, int low, int mid, int high);

void merge_sort(individual *current_generation, int low, int high);

void main_merge_sort(individual *current_generation,int id, int N, int P, int part);

void merge_final(individual *current_generation,int N, int P, int part, int agr);

#endif