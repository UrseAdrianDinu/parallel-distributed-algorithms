#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "genetic_algorithm.h"
#define MIN(a, b) ((a) < (b)) ? (a) : (b)

int read_input(sack_object **objects, int *object_count, int *sack_capacity, int *generations_count, int *NUM_THREADS, int argc, char *argv[])
{
	FILE *fp;

	if (argc < 4)
	{
		fprintf(stderr, "Usage:\n\t./tema1 in_file generations_count num_threads\n");
		return 0;
	}

	fp = fopen(argv[1], "r");
	if (fp == NULL)
	{
		return 0;
	}

	if (fscanf(fp, "%d %d", object_count, sack_capacity) < 2)
	{
		fclose(fp);
		return 0;
	}

	if (*object_count % 10)
	{
		fclose(fp);
		return 0;
	}

	sack_object *tmp_objects = (sack_object *)calloc(*object_count, sizeof(sack_object));

	for (int i = 0; i < *object_count; ++i)
	{
		if (fscanf(fp, "%d %d", &tmp_objects[i].profit, &tmp_objects[i].weight) < 2)
		{
			free(objects);
			fclose(fp);
			return 0;
		}
	}

	fclose(fp);

	*generations_count = (int)strtol(argv[2], NULL, 10);

	if (*generations_count == 0)
	{
		free(tmp_objects);

		return 0;
	}

	*objects = tmp_objects;

	*NUM_THREADS = atoi(argv[3]);

	return 1;
}

void print_objects(const sack_object *objects, int object_count)
{
	for (int i = 0; i < object_count; ++i)
	{
		printf("%d %d\n", objects[i].weight, objects[i].profit);
	}
}

void print_generation(const individual *generation, int limit)
{
	for (int i = 0; i < limit; ++i)
	{
		for (int j = 0; j < generation[i].chromosome_length; ++j)
		{
			printf("%d ", generation[i].chromosomes[j]);
		}

		printf("\n%d - %d\n", i, generation[i].fitness);
	}
}

void print_best_fitness(const individual *generation)
{
	printf("%d\n", generation[0].fitness);
}

void parallel_compute_fitness_function(const sack_object *objects, individual *generation, int object_count, int sack_capacity, int start, int end)
{
	int weight;
	int profit;

	for (int i = start; i < end; ++i)
	{
		weight = 0;
		profit = 0;

		for (int j = 0; j < generation[i].chromosome_length; ++j)
		{
			if (generation[i].chromosomes[j])
			{
				weight += objects[j].weight;
				profit += objects[j].profit;
			}
		}
		generation[i].fitness = (weight <= sack_capacity) ? profit : 0;
	}
}

int compare(individual *first, individual *second)
{
	int i;
	int res = second->fitness - first->fitness; // decreasing by fitness
	if (res == 0)
	{
		int first_count = 0;
		int second_count = 0;

		for (i = 0; i < first->chromosome_length && i < second->chromosome_length; ++i)
		{
			first_count += first->chromosomes[i];
			second_count += second->chromosomes[i];
		}

		res = first_count - second_count; // increasing by number of objects in the sack
		if (res == 0)
		{
			return second->index - first->index;
		}
	}

	return res;
}

void mutate_bit_string_1(const individual *ind, int generation_index)
{
	int i, mutation_size;
	int step = 1 + generation_index % (ind->chromosome_length - 2);

	if (ind->index % 2 == 0)
	{
		// for even-indexed individuals, mutate the first 40% chromosomes by a given step
		mutation_size = ind->chromosome_length * 4 / 10;
		for (i = 0; i < mutation_size; i += step)
		{
			ind->chromosomes[i] = 1 - ind->chromosomes[i];
		}
	}
	else
	{
		// for even-indexed individuals, mutate the last 80% chromosomes by a given step
		mutation_size = ind->chromosome_length * 8 / 10;
		for (i = ind->chromosome_length - mutation_size; i < ind->chromosome_length; i += step)
		{
			ind->chromosomes[i] = 1 - ind->chromosomes[i];
		}
	}
}

void mutate_bit_string_2(const individual *ind, int generation_index)
{
	int step = 1 + generation_index % (ind->chromosome_length - 2);

	// mutate all chromosomes by a given step
	for (int i = 0; i < ind->chromosome_length; i += step)
	{
		ind->chromosomes[i] = 1 - ind->chromosomes[i];
	}
}

void crossover(individual *parent1, individual *child1, int generation_index)
{
	individual *parent2 = parent1 + 1;
	individual *child2 = child1 + 1;
	int count = 1 + generation_index % parent1->chromosome_length;

	memcpy(child1->chromosomes, parent1->chromosomes, count * sizeof(int));
	memcpy(child1->chromosomes + count, parent2->chromosomes + count, (parent1->chromosome_length - count) * sizeof(int));

	memcpy(child2->chromosomes, parent2->chromosomes, count * sizeof(int));
	memcpy(child2->chromosomes + count, parent1->chromosomes + count, (parent1->chromosome_length - count) * sizeof(int));
}

void copy_individual(const individual *from, const individual *to)
{
	memcpy(to->chromosomes, from->chromosomes, from->chromosome_length * sizeof(int));
}

void free_generation(individual *generation)
{
	int i;

	for (i = 0; i < generation->chromosome_length; ++i)
	{
		free(generation[i].chromosomes);
		generation[i].chromosomes = NULL;
		generation[i].fitness = 0;
	}
}

// Functie ce interclaseaza 2 vectori
void merge(individual *current_generation, int low, int mid, int high)
{
	int n1 = mid - low + 1;
	int n2 = high - mid;
	individual left[n1];
	individual right[n2];
	int i, j;
	int k = 0;

	for (i = 0; i < n1; i++)
	{
		left[i] = current_generation[i + low];
	}

	for (j = 0; j < n2; j++)
	{
		right[j] = current_generation[j + mid + 1];
	}

	i = 0;
	j = 0;
	while (i < n1 && j < n2)
	{
		if (compare(&left[i], &right[j]) <= 0)
		{
			current_generation[low + k] = left[i];
			i++;
		}
		else
		{
			current_generation[low + k] = right[j];
			j++;
		}
		k++;
	}

	while (i < n1)
	{
		current_generation[low + k] = left[i];
		k++;
		i++;
	}

	while (j < n2)
	{
		current_generation[low + k] = right[j];
		k++;
		j++;
	}
}

// Merge sort
void merge_sort(individual *current_generation, int low, int high)
{
	if (low < high)
	{
		int mid = low + (high - low) / 2;
		merge_sort(current_generation, low, mid);
		merge_sort(current_generation, mid + 1, high);
		merge(current_generation, low, mid, high);
	}
}

void main_merge_sort(individual *current_generation, int id, int N, int P, int thread_part)
{
	int start = id * thread_part;
	int end = (id + 1) * thread_part - 1;
	merge_sort(current_generation, start, end);
}

// Functie ce construieste vectorul sortat
void merge_final(individual *current_generation, int N, int P, int thread_part, int width)
{

	for (int i = 0; i < P; i = i + 2)
	{
		int left = i * thread_part * width;
		int right = ((i + 2) * thread_part * width) - 1;
		int middle = left + thread_part * width - 1;
		if (right >= N)
		{
			right = N - 1;
		}
		merge(current_generation, left, middle, right);
	}
	if (P / 2 >= 1)
	{
		merge_final(current_generation, N, P / 2, thread_part, width * 2);
	}
}

void *run_genetic_algorithm(void *arg)
{
	struct my_arg *data = (struct my_arg *)arg;
	int count, cursor;
	individual *tmp = NULL;

	// Impartim vectorul de indivizi in mod egal la data->P thread-uri
	int start = data->id * (double)data->N / data->P;
	int end = MIN((data->id + 1) * (double)data->N / data->P, data->N);

	// set initial generation (composed of object_count individuals with a single item in the sack)
	// Creare paralela a primei generatii
	for (int i = start; i < end; i++)
	{
		data->current_generation[i].fitness = 0;
		data->current_generation[i].chromosomes = (int *)calloc(data->N, sizeof(int));
		data->current_generation[i].chromosomes[i] = 1;
		data->current_generation[i].index = i;
		data->current_generation[i].chromosome_length = data->N;

		data->next_generation[i].fitness = 0;
		data->next_generation[i].chromosomes = (int *)calloc(data->N, sizeof(int));
		data->next_generation[i].index = i;
		data->next_generation[i].chromosome_length = data->N;
	}

	// iterate for each generation
	for (int k = 0; k < data->generations_count; ++k)
	{
		cursor = 0;

		// compute fitness and sort by it
		// Calculare fitness pentru fiecare individ in paralel
		parallel_compute_fitness_function(data->objects, data->current_generation, data->N, data->sack_capacity, start, end);

		// Sortare paralela a gemeratiei curente
		main_merge_sort(data->current_generation, data->id, data->N, data->P, data->thread_part);
		pthread_barrier_wait(data->barrier);

		// Constructie vector final sortat
		pthread_mutex_lock(data->mutex);
		if (data->sorted[k] == 0)
		{
			merge_final(data->current_generation, data->N, data->P, data->thread_part, 1);
			data->sorted[k] = 1;
		}
		pthread_mutex_unlock(data->mutex);

		// keep first 30% children (elite children selection)
		// Selectia elitei in paralel
		count = data->N * 3 / 10;
		int startcopy = data->id * (double)count / data->P;
		int endcopy = MIN((data->id + 1) * (double)count / data->P, count);
		for (int i = startcopy; i < endcopy; ++i)
		{
			copy_individual(data->current_generation + i, data->next_generation + i);
		}
		cursor = count;

		// mutate first 20% children with the first version of bit string mutation
		// Se aplica in paralel prima varianta de mutatie pe primii 20% de indivizi
		count = data->N * 2 / 10;
		startcopy = data->id * (double)count / data->P;
		endcopy = MIN((data->id + 1) * (double)count / data->P, count);
		for (int i = startcopy; i < endcopy; ++i)
		{
			copy_individual(data->current_generation + i, data->next_generation + cursor + i);
		}

		for (int i = startcopy; i < endcopy; ++i)
		{
			mutate_bit_string_1(data->next_generation + cursor + i, k);
		}
		cursor += count;

		// mutate next 20% children with the second version of bit string mutation
		// Se aplica in paralel a doua varianta de mutatie pe urmatorii 20% de indivizi
		count = data->N * 2 / 10;
		startcopy = data->id * (double)count / data->P;
		endcopy = MIN((data->id + 1) * (double)count / data->P, count);
		for (int i = startcopy; i < endcopy; ++i)
		{
			copy_individual(data->current_generation + i + count, data->next_generation + cursor + i);
		}

		for (int i = startcopy; i < endcopy; ++i)
		{
			mutate_bit_string_2(data->next_generation + cursor + i, k);
		}
		cursor += count;

		// crossover first 30% parents with one-point crossover
		// (if there is an odd number of parents, the last one is kept as such)

		// Se aplica in paralel crossover pe primii 30% de indivizi
		// In cazul in care numarul de parinti este impar, doar un thread copiaza ultimul individ
		count = data->N * 3 / 10;
		pthread_mutex_lock(data->mutex);
		if (count % 2 == 1 && data->odd[k] == 0)
		{
			copy_individual(data->current_generation + data->N - 1, data->next_generation + cursor + count - 1);
			data->odd[k] = 1;
			count--;
		}
		pthread_mutex_unlock(data->mutex);

		startcopy = data->id * (double)count / data->P;
		endcopy = MIN((data->id + 1) * (double)count / data->P, count) - 1;

		for (int i = startcopy; i < endcopy; i += 2)
		{
			crossover(data->current_generation + i, data->next_generation + cursor + i, k);
		}
		pthread_barrier_wait(data->barrier);

		// switch to new generation
		pthread_mutex_lock(data->mutex);
		tmp = data->current_generation;
		data->current_generation = data->next_generation;
		data->next_generation = tmp;
		pthread_mutex_unlock(data->mutex);

		for (int i = start; i < end; ++i)
		{
			data->current_generation[i].index = i;
		}

		if (k % 5 == 0)
		{
			pthread_mutex_lock(data->mutex);
			if (data->flagprint[k] == 0)
			{
				print_best_fitness(data->current_generation);
				data->flagprint[k] = 1;
			}
			pthread_mutex_unlock(data->mutex);
		}
	}

	// Calculare finala a fitness-ului pentru fiecare individ in paralel
	parallel_compute_fitness_function(data->objects, data->current_generation, data->N, data->sack_capacity, start, end);

	// Sortare finala a ultimei generatii in paralel
	main_merge_sort(data->current_generation, data->id, data->N, data->P, data->thread_part);
	pthread_barrier_wait(data->barrier);
	

	pthread_mutex_lock(data->mutex);
	if (*data->final == 0)
	{
		merge_final(data->current_generation, data->N, data->P, data->thread_part, 1);
		*data->final = 1;
		print_best_fitness(data->current_generation);
	}
	pthread_mutex_unlock(data->mutex);
	pthread_exit(NULL);
	return NULL;
}