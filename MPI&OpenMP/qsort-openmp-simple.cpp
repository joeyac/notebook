#include<bits/stdc++.h>
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
	// a[l] - a[r] 双闭区间
	if (l >= r) return P(l, r);
	// 三分
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
	int left = ret.first, right = ret.second;
//	printf("%d : a[%d, %d]\n", omp_get_thread_num(), l, r);
	{
		#pragma omp task firstprivate(a, l, left)
		{
			qsort(a, l, left - 1);
		}
		#pragma omp task firstprivate(a, right, r)
		{
			qsort(a, right + 1, r);
		}
	}
}

int main(int argc, char *argv[]) {
	if (argc < 3) {
		printf("Usage: %s [size_of_test] [seed] [num_of_threads]\nExiting.\n", argv[0]);
		printf("if seed=0, all value will be 0;\nif seed=-1, all value will be desc.\n");
		exit(1);
	}
	int n, seed, num_of_threads;
	sscanf(argv[1], "%d", &n);
	sscanf(argv[2], "%d", &seed);
	sscanf(argv[3], "%d", &num_of_threads);
	int *data = (int*)malloc(sizeof(int)*n);
	srand(seed);
	for (int i = 0; i < n; i++)
		if (0 == seed) data[i] = 0;
		else if (-1 == seed) data[i] = n - i;
		else data[i] = rand() % n + 1;	
	
	// 允许嵌套并行
	omp_set_nested(1);
	// 设置线程数量
	omp_set_num_threads(num_of_threads);
	
	double tmp = omp_get_wtime();
	
	#pragma omp parallel shared(data, n)
	{
		#pragma omp single nowait
		{
			qsort(data, 0, n - 1);
		}
	}
	
	tmp = omp_get_wtime() - tmp;
	validate(data, n);
	//printf("Sort of %d ints took %lf secs\n", n, end_time - start_time);
	printf("%lf\n", tmp);
	return 0;
}
