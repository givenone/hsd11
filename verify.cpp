#include <cstdio>
#include <cstdlib>
#include "fpga_api.h"

int main(void)
{
        printf("program start");
	// input vector size M
	// output vector size N
	// matix size M x N
	int M = 8;
	int N = 8;

	float* flat = new float[M*(N+1)];
	float* input = flat;
	float* mat = flat + M;  
	float* output = new float[N];
	float* output_fpga = new float[N];

    for(int i = 0; i< M; i++)
    {
        flat[i] = (float)1.0;
    }
    for(int i = 0; i< N; i++)
    {
        for(int j=0; j < M; j++)
         {
            flat[M + i * M + j] = (float)j;
            printf("%f ", flat[M + i * M + j]);
         }   
         printf("\n");
    }
    /*
	for(int i = 0 ; i < M*N ; ++i)
		flat[i + M] = ((float)rand()) / RAND_MAX;
    */
	// computation
	for (int i = 0; i < N; i++)		
	{
		output[i] = 0.0f;
		for (int j = 0; j < M; j++)
			output[i] += input[j] * mat[M*i +j];
	}		

	// FPGA offloading
	// memory load
	FPGA dev(0x40000000, 0x43c00000);
	dev.largeMV(mat, input, output_fpga, M, N);

	// display
	printf("%-10s%-10s%-10s\n", "index", "CPU", "FPGA");
	for (int i = 0; i < N; i++)
	{
		printf("%-10d%-10f%-10f%\n", i, output[i], output_fpga[i]);
	}

	delete[] flat;
	delete[] output;
	delete[] output_fpga;
	return 0;
}
