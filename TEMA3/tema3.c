#include <mpi.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CONVERGENCE_COEF 100
#define min(X, Y) (((X) < (Y)) ? (X) : (Y))

// Matrice ce reprezinta clusterele
// Linia i contine rangurile workerilor coordonatorului i
int *clusters[3];
// Vector ce reprezinta cati workeri are fiecare coordonator
int workers_per_cluster[3]; //

// Fiecare coordonator citeste fisierul de intrare
void read_workers(int rank)
{
	FILE *fp;
	char file_name[15];
	sprintf(file_name, "./cluster%d.txt", rank);

	fp = fopen(file_name, "r");
	fscanf(fp, "%d", &workers_per_cluster[rank]);

	clusters[rank] = malloc(sizeof(int) * workers_per_cluster[rank]);
	if (clusters[rank] == NULL)
	{
		printf("malloc error\n");
		exit(-1);
	}
	for (size_t i = 0; i < workers_per_cluster[rank]; i++)
		fscanf(fp, "%d", &clusters[rank][i]);
}

int main(int argc, char *argv[])
{
	int rank, nProcesses, leader;

	int *v;
	int N;
	int total_workers = 0;

	int eroare_comunicatie = atoi(argv[2]);

	MPI_Init(&argc, &argv);
	MPI_Request request;

	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &nProcesses);

	// Stabilirea topologiei
	if (rank == 0 || rank == 1 || rank == 2)
	{
		// Fiecare coordonator citeste fisierul de intrare corespunzator
		read_workers(rank);
		leader = rank;

		// Fiecare coordonator isi informeaza workerii
		// cine este coordonatorul lor, trimitandu-le rankul lui
		for (int i = 0; i < workers_per_cluster[rank]; i++)
		{
			MPI_Send(&leader, 1, MPI_INT, clusters[rank][i], 0, MPI_COMM_WORLD);
			printf("M(%d,%d)\n", rank, clusters[rank][i]);
		}

		// In cazul in care nu exista o eroare de comunicatie,
		// fiecare coordonator trimite celorlalti coordonatori
		// rangurile workerilor pe care ii coordoneaza
		if (eroare_comunicatie == 0)
		{
			for (int i = 0; i < 3; i++)
			{
				if (i != rank)
				{
					// Trimite mai intai numarul de workeri si dupa vectorul
					MPI_Send(&workers_per_cluster[rank], 1, MPI_INT, i, 0, MPI_COMM_WORLD);
					printf("M(%d,%d)\n", rank, i);
					MPI_Send(clusters[rank], workers_per_cluster[rank], MPI_INT, i, 0, MPI_COMM_WORLD);
					printf("M(%d,%d)\n", rank, i);
				}
			}
		}
		else
		{
			// In cazul in care exista o eroare de comunicatie,
			// coordonatorii 0 si 1 ii trimit coordonatorului 2
			// rangurile workerilor pe care ii coordoneaza,
			// la fel si coordonatorul 2 le trimite celorlalti
			// coordonatori rangurile workerilor
			switch (rank)
			{
			case 0:
				// Trimite mai intai numarul de workeri si dupa vectorul
				MPI_Send(&workers_per_cluster[rank], 1, MPI_INT, 2, rank, MPI_COMM_WORLD);
				printf("M(%d,%d)\n", rank, 2);
				MPI_Send(clusters[rank], workers_per_cluster[rank], MPI_INT, 2, rank, MPI_COMM_WORLD);
				printf("M(%d,%d)\n", rank, 2);
				break;

			case 1:
				// Trimite mai intai numarul de workeri si dupa vectorul
				MPI_Send(&workers_per_cluster[rank], 1, MPI_INT, 2, rank, MPI_COMM_WORLD);
				printf("M(%d,%d)\n", rank, 2);
				MPI_Send(clusters[rank], workers_per_cluster[rank], MPI_INT, 2, rank,
						 MPI_COMM_WORLD);
				printf("M(%d,%d)\n", rank, 2);
				break;

			case 2:
				// Trimite mai intai numarul de workeri si dupa vectorul
				MPI_Send(&workers_per_cluster[rank], 1, MPI_INT, 0, rank, MPI_COMM_WORLD);
				printf("M(%d,%d)\n", rank, 0);
				MPI_Send(clusters[rank], workers_per_cluster[rank], MPI_INT, 0, rank, MPI_COMM_WORLD);
				printf("M(%d,%d)\n", rank, 0);

				// Trimite mai intai numarul de workeri si dupa vectorul
				MPI_Send(&workers_per_cluster[rank], 1, MPI_INT, 1, rank, MPI_COMM_WORLD);
				printf("M(%d,%d)\n", rank, 1);
				MPI_Send(clusters[rank], workers_per_cluster[rank], MPI_INT, 1, rank, MPI_COMM_WORLD);
				printf("M(%d,%d)\n", rank, 1);
				break;

			default:
				break;
			}
		}

		// In cazul in care nu exista o eroare de comunicatie,
		// fiecare coordonator primeste vectorul de ranguri ale workerilor
		// de la ceilalti coordonatori si afiseaza topologia
		if (eroare_comunicatie == 0)
		{
			for (int i = 0; i < 3; i++)
			{
				if (i != rank)
				{
					int nr;
					// Primeste mai intai numarul de workeri
					// aloca memorie si dupa vectorul
					MPI_Recv(&nr, 1, MPI_INT, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
					workers_per_cluster[i] = nr;
					clusters[i] = malloc(nr * sizeof(int));
					if (clusters[i] == NULL)
					{
						printf("malloc error\n");
						exit(-1);
					}
					MPI_Recv(clusters[i], nr, MPI_INT, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
				}
			}

			// Afisare topologie
			printf("%d -> ", rank);
			for (int i = 0; i < 3; i++)
			{
				printf("%d:", i);
				for (int j = 0; j < workers_per_cluster[i] - 1; j++)
				{
					printf("%d,", clusters[i][j]);
				}
				printf("%d ", clusters[i][workers_per_cluster[i] - 1]);
			}
			printf("\n");
		}
		else
		{
			// In cazul in care exista o eroare de comunicatie,
			// coordonatorul 2 primeste vectorul de ranguri de
			// la ceilalti 2 coordonatori, afiseaza topologia, ii
			// trimite coordonatorului 1 vectorul de workeri al
			// coordonatorului 0, ii trimite coordonatorului 0
			// vectorul de workeri al coordonatorului 1
			// Coordonatori 0 si 1, primesc vectorii de workeri
			// de la coordonatorul 2 si afiseaza topologia

			int nr;
			switch (rank)
			{
			case 0:
				MPI_Recv(&nr, 1, MPI_INT, 2, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
				workers_per_cluster[2] = nr;
				clusters[2] = malloc(nr * sizeof(int));
				if (clusters[2] == NULL)
				{
					printf("malloc error\n");
					exit(-1);
				}
				MPI_Recv(clusters[2], nr, MPI_INT, 2, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

				MPI_Recv(&nr, 1, MPI_INT, 2, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
				workers_per_cluster[1] = nr;
				clusters[1] = malloc(nr * sizeof(int));
				if (clusters[1] == NULL)
				{
					printf("malloc error\n");
					exit(-1);
				}
				MPI_Recv(clusters[1], nr, MPI_INT, 2, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
				break;

			case 1:
				MPI_Recv(&nr, 1, MPI_INT, 2, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
				workers_per_cluster[2] = nr;
				clusters[2] = malloc(nr * sizeof(int));
				if (clusters[2] == NULL)
				{
					printf("malloc error\n");
					exit(-1);
				}
				MPI_Recv(clusters[2], nr, MPI_INT, 2, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
				MPI_Recv(&nr, 1, MPI_INT, 2, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
				workers_per_cluster[0] = nr;
				clusters[0] = malloc(nr * sizeof(int));
				if (clusters[0] == NULL)
				{
					printf("malloc error\n");
					exit(-1);
				}
				MPI_Recv(clusters[0], nr, MPI_INT, 2, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
				break;

			case 2:
				MPI_Recv(&nr, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
				workers_per_cluster[0] = nr;
				clusters[0] = malloc(nr * sizeof(int));
				if (clusters[0] == NULL)
				{
					printf("malloc error\n");
					exit(-1);
				}
				MPI_Recv(clusters[0], nr, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

				MPI_Recv(&nr, 1, MPI_INT, 1, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
				workers_per_cluster[1] = nr;
				clusters[1] = malloc(nr * sizeof(int));
				if (clusters[1] == NULL)
				{
					printf("malloc error\n");
					exit(-1);
				}
				MPI_Recv(clusters[1], nr, MPI_INT, 1, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

				// Afisare topologie
				printf("%d -> ", rank);
				for (int i = 0; i < 3; i++)
				{
					printf("%d:", i);
					for (int j = 0; j < workers_per_cluster[i] - 1; j++)
					{
						printf("%d,", clusters[i][j]);
					}
					printf("%d ", clusters[i][workers_per_cluster[i] - 1]);
				}
				printf("\n");

				MPI_Send(&workers_per_cluster[1], 1, MPI_INT, 0, 1, MPI_COMM_WORLD);
				printf("M(%d,%d)\n", rank, 0);
				MPI_Send(clusters[1], workers_per_cluster[1], MPI_INT, 0, 1, MPI_COMM_WORLD);
				printf("M(%d,%d)\n", rank, 0);

				MPI_Send(&workers_per_cluster[0], 1, MPI_INT, 1, 0, MPI_COMM_WORLD);
				printf("M(%d,%d)\n", rank, 1);
				MPI_Send(clusters[0], workers_per_cluster[0], MPI_INT, 1, 0, MPI_COMM_WORLD);
				printf("M(%d,%d)\n", rank, 1);

				break;

			default:
				break;
			}

			if (rank != 2)
			{
				// Afisare topologie
				printf("%d -> ", rank);
				for (int i = 0; i < 3; i++)
				{
					printf("%d:", i);
					for (int j = 0; j < workers_per_cluster[i] - 1; j++)
					{
						printf("%d,", clusters[i][j]);
					}
					printf("%d ", clusters[i][workers_per_cluster[i] - 1]);
				}
				printf("\n");
			}
		}

		// Fiecare coordonator trimite topologia fiecarui worker pe care
		// il coordoneaza
		for (int i = 0; i < workers_per_cluster[rank]; i++)
		{
			for (int j = 0; j < 3; j++)
			{
				MPI_Send(&workers_per_cluster[j], 1, MPI_INT, clusters[rank][i], j, MPI_COMM_WORLD);
				printf("M(%d,%d)\n", rank, clusters[rank][i]);
				MPI_Send(clusters[j], workers_per_cluster[j], MPI_INT, clusters[rank][i], j, MPI_COMM_WORLD);
				printf("M(%d,%d)\n", rank, clusters[rank][i]);
			}
		}
	}
	else
	{
		// Fiecare worker primeste rank-ul liderului si isi seteaza liderul
		// Primeste topologia de la lider si o afiseaza
		MPI_Recv(&leader, 1, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		for (int i = 0; i < 3; i++)
		{
			MPI_Recv(&workers_per_cluster[i], 1, MPI_INT, leader, i, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			clusters[i] = malloc(workers_per_cluster[i] * sizeof(int));
			if (clusters[i] == NULL)
			{
				printf("malloc error\n");
				exit(-1);
			}
			MPI_Recv(clusters[i], workers_per_cluster[i], MPI_INT, leader, i, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		}

		// Afisare topologie
		printf("%d -> ", rank);
		for (int i = 0; i < 3; i++)
		{
			printf("%d:", i);
			for (int j = 0; j < workers_per_cluster[i] - 1; j++)
			{
				printf("%d,", clusters[i][j]);
			}
			printf("%d ", clusters[i][workers_per_cluster[i] - 1]);
		}
		printf("\n");
	}

	// Realizarea calculelor
	// Coordonatorul 0 genereaza vectorul V
	if (rank == 0)
	{
		N = atoi(argv[1]);
		v = malloc(N * sizeof(int));

		if (v == NULL)
		{
			printf("malloc error\n");
			exit(-1);
		}

		for (int i = 0; i < N; i++)
		{
			v[i] = i;
		}

		// Calculeaza numarul total de workeri
		for (int i = 0; i < 3; i++)
		{
			total_workers += workers_per_cluster[i];
		}

		// Coordonatorul 0 calculeaza partile de procesat pentru workerii fiecarui coordonator
		// Ex. N = 12
		// Primul cluster are intervalul 0 - 4
		// Al doilea cluster are intervalul 4 - 10
		// Al treilea cluster are intervalul 10 - 12
		// Ex. N = 6000
		// Primul cluster are intervalul 0 - 2000
		// Al doilea cluster are intervalul 2000 - 5000
		// Al treilea cluster are intervalul 5000 - 6000
		int start1 = workers_per_cluster[0] * ((double)N / total_workers);
		int end1 = min((workers_per_cluster[0] + workers_per_cluster[1]) * (double)N / total_workers, N);
		int start2 = (workers_per_cluster[0] + workers_per_cluster[1]) * (double)N / total_workers;
		int end2 = min((workers_per_cluster[2] + workers_per_cluster[0] + workers_per_cluster[1]) * (double)N / total_workers, N);

		int l1 = end1 - start1;
		int l2 = end2 - start2;

		// In cazul in care nu exista o eroare de comunicatie,
		// coordonatorul 0 trimite partile corespunzatoare fiecarui coordonator
		// din vectorul generat
		if (eroare_comunicatie == 0)
		{
			MPI_Send(&l1, 1, MPI_INT, 1, 0, MPI_COMM_WORLD);
			printf("M(%d,%d)\n", rank, 1);
			MPI_Send(v + start1, l1, MPI_INT, 1, 0, MPI_COMM_WORLD);
			printf("M(%d,%d)\n", rank, 1);

			MPI_Send(&l2, 1, MPI_INT, 2, 0, MPI_COMM_WORLD);
			printf("M(%d,%d)\n", rank, 2);
			MPI_Send(v + start2, l2, MPI_INT, 2, 0, MPI_COMM_WORLD);
			printf("M(%d,%d)\n", rank, 2);
		}
		else
		{
			// In cazul in care exista o eroare de comunicatie,
			// coordonatorul 0 trimite coordonatorului 2,
			// partile corespunzatoare coordonatorilor 2 si 1
			MPI_Send(&l2, 1, MPI_INT, 2, 0, MPI_COMM_WORLD);
			printf("M(%d,%d)\n", rank, 2);
			MPI_Send(v + start2, l2, MPI_INT, 2, 0, MPI_COMM_WORLD);
			printf("M(%d,%d)\n", rank, 2);

			MPI_Send(&l1, 1, MPI_INT, 2, 0, MPI_COMM_WORLD);
			printf("M(%d,%d)\n", rank, 2);
			MPI_Send(v + start1, l1, MPI_INT, 2, 0, MPI_COMM_WORLD);
			printf("M(%d,%d)\n", rank, 2);
		}

		int *start = malloc(workers_per_cluster[0] * sizeof(int));
		int *end = malloc(workers_per_cluster[0] * sizeof(int));
		if (start == NULL || end == NULL)
		{
			printf("malloc error\n");
			exit(-1);
		}

		// Coordonatorul 0 stabileste intervalele pentru workerii lui
		// si le trimite
		for (int i = 0; i < workers_per_cluster[0]; i++)
		{
			start[i] = i * (double)N / total_workers;
			end[i] = min((i + 1) * (double)N / total_workers, N);
			MPI_Send(&start[i], 1, MPI_INT, clusters[0][i], 0, MPI_COMM_WORLD);
			printf("M(%d,%d)\n", rank, clusters[0][i]);
			MPI_Send(&end[i], 1, MPI_INT, clusters[0][i], 0, MPI_COMM_WORLD);
			printf("M(%d,%d)\n", rank, clusters[0][i]);
			MPI_Send(v + start[i], end[i] - start[i], MPI_INT, clusters[0][i], 0, MPI_COMM_WORLD);
			printf("M(%d,%d)\n", rank, clusters[0][i]);
		}

		// Primeste rezultatele calculate de fiecare worker
		for (int i = 0; i < workers_per_cluster[0]; i++)
		{
			MPI_Recv(v + start[i], end[i] - start[i], MPI_INT, clusters[0][i], 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		}

		// In cazul in care nu exista o eroare de comunicatie,
		// coordonatorul 0 primeste rezultatele workerilor
		// coordonatorilor 1 si 2
		if (eroare_comunicatie == 0)
		{
			MPI_Recv(v + start1, l1, MPI_INT, 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			MPI_Recv(v + start2, l2, MPI_INT, 2, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		}
		else
		{
			// In cazul in care exista o eroare de comunicatie,
			// coordonatorul 0 primeste rezultatele workerilor
			// coordonatorilor 1 si 2 de la coordonatorul 2
			MPI_Recv(v + start2, l2, MPI_INT, 2, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			MPI_Recv(v + start1, l1, MPI_INT, 2, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		}

		// Eliberarea memoriei
		free(start);
		free(end);
	}
	else
	{
		if (rank == 1)
		{
			// In cazul in care nu exista o eroare de comunicatie,
			// coordonatorul 1 primeste vectorul de procesat de la
			// cordonatorul 0, in caz contrar il primeste de la 2
			if (eroare_comunicatie == 0)
			{
				MPI_Recv(&N, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
				v = malloc(N * sizeof(int));
				if (v == NULL)
				{
					printf("malloc error\n");
					exit(-1);
				}
				MPI_Recv(v, N, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			}
			else
			{
				MPI_Recv(&N, 1, MPI_INT, 2, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
				v = malloc(N * sizeof(int));
				if (v == NULL)
				{
					printf("malloc error\n");
					exit(-1);
				}
				MPI_Recv(v, N, MPI_INT, 2, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			}

			int *start = malloc(workers_per_cluster[1] * sizeof(int));
			int *end = malloc(workers_per_cluster[1] * sizeof(int));
			if (start == NULL || end == NULL)
			{
				printf("malloc error\n");
				exit(-1);
			}

			// Coordonatorul 1 stabileste intervalele pentru workerii lui
			// si trimite fiecarui worker capetele intervalului si portiunea din vector
			for (int i = 0; i < workers_per_cluster[1]; i++)
			{
				start[i] = i * (double)N / workers_per_cluster[1];
				end[i] = min((i + 1) * (double)N / workers_per_cluster[1], N);
				MPI_Send(&start[i], 1, MPI_INT, clusters[1][i], 0, MPI_COMM_WORLD);
				printf("M(%d,%d)\n", rank, clusters[1][i]);
				MPI_Send(&end[i], 1, MPI_INT, clusters[1][i], 0, MPI_COMM_WORLD);
				printf("M(%d,%d)\n", rank, clusters[1][i]);
				MPI_Send(v + start[i], end[i] - start[i], MPI_INT, clusters[1][i], 0, MPI_COMM_WORLD);
				printf("M(%d,%d)\n", rank, clusters[1][i]);
			}

			// Primeste rezultatele calculate de fiecare worker
			for (int i = 0; i < workers_per_cluster[1]; i++)
			{
				MPI_Recv(v + start[i], end[i] - start[i], MPI_INT, clusters[1][i], 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			}

			// In cazul in care nu exista o eroare de comunicatie,
			// coordonatorul 1 trimite rezultatele calculate de workerii lui
			// coordonatorului 0, in caz contrar le trimite coordonatorului 2
			if (eroare_comunicatie == 0)
			{
				MPI_Send(v, N, MPI_INT, 0, 0, MPI_COMM_WORLD);
				printf("M(%d,%d)\n", rank, 0);
			}
			else
			{
				MPI_Send(&N, 1, MPI_INT, 2, 0, MPI_COMM_WORLD);
				printf("M(%d,%d)\n", rank, 2);
				MPI_Send(v, N, MPI_INT, 2, 0, MPI_COMM_WORLD);
				printf("M(%d,%d)\n", rank, 2);
			}

			// Eliberarea memoriei
			free(start);
			free(end);
		}
		else
		{
			// Coordonatorul 2 primeste vectorul de procesat de la
			// cordonatorul 0
			if (rank == 2)
			{
				MPI_Recv(&N, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
				v = malloc(N * sizeof(int));
				if (v == NULL)
				{
					printf("malloc error\n");
					exit(-1);
				}
				MPI_Recv(v, N, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

				// In cazul in care exista o eroare de comunicatie,
				// coordonatorul 2 primeste vectorul de procesat al coordonatorului 1,
				// si il trimite
				if (eroare_comunicatie == 1)
				{
					int l1;
					MPI_Recv(&l1, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
					int *v1 = malloc(l1 * sizeof(int));
					if (v == NULL)
					{
						printf("malloc error\n");
						exit(-1);
					}
					MPI_Recv(v1, l1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
					MPI_Send(&l1, 1, MPI_INT, 1, 0, MPI_COMM_WORLD);
					printf("M(%d,%d)\n", rank, 1);
					MPI_Send(v1, l1, MPI_INT, 1, 0, MPI_COMM_WORLD);
					printf("M(%d,%d)\n", rank, 1);
					free(v1);
				}

				int *start = malloc(workers_per_cluster[2] * sizeof(int));
				int *end = malloc(workers_per_cluster[2] * sizeof(int));
				if (start == NULL || end == NULL)
				{
					printf("malloc error\n");
					exit(-1);
				}

				// Coordonatorul 2 stabileste intervalele pentru workerii lui
				// si trimite fiecarui worker capetele intervalului si portiunea din vector
				for (int i = 0; i < workers_per_cluster[2]; i++)
				{
					start[i] = i * (double)N / workers_per_cluster[2];
					end[i] = min((i + 1) * (double)N / workers_per_cluster[2], N);
					MPI_Send(&start[i], 1, MPI_INT, clusters[2][i], 0, MPI_COMM_WORLD);
					printf("M(%d,%d)\n", rank, clusters[2][i]);
					MPI_Send(&end[i], 1, MPI_INT, clusters[2][i], 0, MPI_COMM_WORLD);
					printf("M(%d,%d)\n", rank, clusters[2][i]);
					MPI_Send(v + start[i], end[i] - start[i], MPI_INT, clusters[2][i], 0, MPI_COMM_WORLD);
					printf("M(%d,%d)\n", rank, clusters[2][i]);
				}

				// Primeste rezultate calculate de fiecare worker
				for (int i = 0; i < workers_per_cluster[2]; i++)
				{
					MPI_Recv(v + start[i], end[i] - start[i], MPI_INT, clusters[2][i], 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
				}

				// In cazul in care nu exista o eroare de comunicatie,
				// coordonatorul 2 trimite rezultatele calculate de workerii lui
				// coordonatorului 0
				if (eroare_comunicatie == 0)
				{
					MPI_Send(v, N, MPI_INT, 0, 0, MPI_COMM_WORLD);
					printf("M(%d,%d)\n", rank, 0);
				}
				else
				{
					// In cazul in care exista o eroare de comunicatie,
					// coordonatorul 2 primeste rezultatele calculate de workerii
					// coordonatorului 1
					// Dupa trimite rezulatele calculate de workerii lui si
					// si rezulatele calculate de workerii coordonatorului 1,
					// coordonatorului 0
					int l1;
					MPI_Recv(&l1, 1, MPI_INT, 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
					int *v1 = malloc(l1 * sizeof(int));
					if (v1 == NULL)
					{
						printf("malloc error\n");
						exit(-1);
					}
					MPI_Recv(v1, l1, MPI_INT, 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
					MPI_Send(v, N, MPI_INT, 0, 2, MPI_COMM_WORLD);
					printf("M(%d,%d)\n", rank, 0);
					MPI_Send(v1, l1, MPI_INT, 0, 1, MPI_COMM_WORLD);
					printf("M(%d,%d)\n", rank, 0);
					free(v1);
				}
				free(start);
				free(end);
			}
			else
			{
				// Procesele worker primesc capetele intervalului de procesat
				// si vectorul, inmultesc elementele cu 2 si trimit vectorul
				// inapoi liderului
				int start, end;
				MPI_Recv(&start, 1, MPI_INT, leader, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
				MPI_Recv(&end, 1, MPI_INT, leader, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
				v = malloc((end - start) * sizeof(int));
				if (v == NULL)
				{
					printf("malloc error\n");
					exit(-1);
				}
				MPI_Recv(v, end - start, MPI_INT, leader, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
				for (int i = 0; i < end - start; i++)
				{
					v[i] *= 2;
				}
				MPI_Send(v, end - start, MPI_INT, leader, 0, MPI_COMM_WORLD);
				printf("M(%d,%d)\n", rank, leader);
			}
		}
	}

	// Pentru a afisa la sfarsit rezultatul, am folosit o bariera
	MPI_Barrier(MPI_COMM_WORLD);

	// Afisarea rezultatului
	if (rank == 0)
	{
		printf("Rezultat: ");
		for (int i = 0; i < N; i++)
		{
			printf("%d ", v[i]);
		}
		printf("\n");
	}

	// Eliberarea memoriei
	for (int i = 0; i < 3; i++)
	{
		free(clusters[i]);
	}
	free(v);

	MPI_Finalize();
	return 0;
}