#include <bits/stdc++.h>
#include <omp.h>

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

int main(int argc, char *argv[]) {
	if (argc < 3) {
		printf("Usage: %s [size_of_test] [seed]\nExiting.\n", argv[0]);
		printf("if seed=0, all value will be 0;\nif seed=-1, all value will be desc.\n");
		exit(1);
	}
	int n, seed;
	sscanf(argv[1], "%d", &n);
	sscanf(argv[2], "%d", &seed);
	int *data = (int*)malloc(sizeof(int)*n);
	srand(seed);
	for (int i = 0; i < n; i++)
		if (0 == seed) data[i] = 0;
		else if (-1 == seed) data[i] = n - i;
		else data[i] = rand() % n + 1;	
	
	double tmp = omp_get_wtime();
	qsort(data, 0, n - 1);
	tmp = omp_get_wtime() - tmp;
	printf("%lf\n", tmp);
	
	return validate(data, n);
}
