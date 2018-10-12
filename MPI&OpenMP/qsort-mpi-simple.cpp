#include "mpi.h"
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <stdlib.h>
#include <iostream>

using namespace std;
typedef pair<int, int> P;

int validate(int *output, int num_elements)
{
	int i = 0;
	for (i = 0; i < num_elements - 1; i++)
	  {
		  if (output[i] > output[i + 1])
			{
				return -1;
			}
	  }
	return 0;
}

P partition(int a[], int l, int r) {
	if (l >= r) return P(l, r);
	int cur = l;
	int left = l;
	int right = r;
	int p = rand() % (r - l + 1);
	int aim = a[l + p];
	swap(a[l + p], a[l]);
	while (cur <= right) {
		if (a[cur] == aim) cur++;
		else if (a[cur] < aim) swap(a[cur++], a[left++]);
		else swap(a[cur], a[right--]);
	}
	return P(left, right);
}

void qsort(int a[], int l, int r) {
	if (l >= r) return;
	P ret = partition(a, l, r);
	qsort(a, l, ret.first - 1);
	qsort(a, ret.second + 1, r);
}

void mpi_sort(int a[], int l, int r, int id, int max_id, int deep) {
	MPI_Status status;
	int rson_id = id + pow(2, deep);
	if (rson_id > max_id) {
		qsort(a, l, r);
		return;
	}
	rson_id--;
	
	if (l > r) {
		MPI_Send(a + l, 0, MPI::INT, rson_id, 0, MPI_COMM_WORLD);
		MPI_Recv(a + l, 0, MPI::INT, rson_id, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
		return;
	}
	
	P ret = partition(a, l, r);
	int lsize = ret.first - l;
	int rsize = r - ret.second;
	if (lsize < rsize) {
		MPI_Send(a + l, lsize, MPI::INT, rson_id, l, MPI_COMM_WORLD);
		mpi_sort(a, ret.second + 1, r, id, max_id, deep + 1);
		MPI_Recv(a + l, lsize, MPI::INT, rson_id, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
	} else {
		MPI_Send(a + ret.second + 1, rsize, MPI::INT, rson_id, ret.second + 1, MPI_COMM_WORLD);
		mpi_sort(a, l, ret.first - 1, id, max_id, deep + 1);
		MPI_Recv(a + ret.second + 1, rsize, MPI::INT, rson_id, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
	}
}

int main(int argc, char *argv[]) {
	if (argc < 3) {
		printf("Usage: %s [size_of_test] [seed]\nExiting.\n", argv[0]);
		printf("if seed=0, all value will be 0;\nif seed=-1, all value will be desc.\n");
		exit(1);
	}
	int numprocs, myid, namelen;
	char processor_name[MPI_MAX_PROCESSOR_NAME];
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
	MPI_Comm_rank(MPI_COMM_WORLD, &myid);
	MPI_Get_processor_name(processor_name, &namelen);
	
	myid++;
	int retcode = 0;
	if (myid == 1) {
		int n, seed;
		sscanf(argv[1], "%d", &n);
		sscanf(argv[2], "%d", &seed);
		int *data = (int*)malloc(sizeof(int)*n);
		srand(seed);
		for (int i = 0; i < n; i++)
			if (0 == seed) data[i] = 0;
			else if (-1 == seed) data[i] = n - i;
			else data[i] = rand() % n + 1;
		double tmp = MPI_Wtime();
		mpi_sort(data, 0, n - 1, myid, numprocs, 0);
		tmp = MPI_Wtime() - tmp;
		//printf("Sort of %d ints took %lf secs(seed=%d,procs=%d)\n", n, tmp, seed, numprocs);
		printf("%lf\n", tmp);
		retcode = validate(data, n);
		free(data);
	} else {
		int *subarray = NULL;
		MPI_Status status;
		int sub_size = 0;
		int index = 0;
		int parent_id = 0;
		while (pow(2, index) < myid) index++;
		MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
		MPI_Get_count(&status, MPI::INT, &sub_size);
		parent_id = status.MPI_SOURCE;
		subarray = (int*)malloc(sub_size * sizeof(int));
		
		MPI_Recv(subarray, sub_size, MPI::INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
		mpi_sort(subarray, 0, sub_size - 1, myid, numprocs, index);
		MPI_Send(subarray, sub_size, MPI::INT, parent_id, parent_id,MPI_COMM_WORLD);
		
		free(subarray);
	}
	MPI_Finalize();
	return retcode;
}
