#include <iostream>
#include <stdio.h>
#include <windows.h>
#include <omp.h>

#define BLOCKSIZE 9308230
#define TIMES 100

const int N = 100000000;

int main()
{
    int iThreadsNum = 1;
    int i;
    double dAvgTime[5] = {0.0, 0.0, 0.0, 0.0, 0.0};
    for(int times = 0; times < TIMES; ++times)
    {
        for(iThreadsNum = 1, i = 1; iThreadsNum <= 16; iThreadsNum = 1 << (i++))
        {
            double dPi = 0.0;
            omp_set_num_threads(iThreadsNum);
            DWORD start = GetTickCount();
            #pragma omp parallel for schedule(dynamic, BLOCKSIZE) reduction(+:dPi)
            //#pragma omp parallel for
            for(int i = 0; i < N; ++i)
            {
                dPi += 4.0/(1.0 + ((i+0.5)/N)*((i+0.5)/N));
            }
            double dTime = (double)(GetTickCount()-start)/1000;
            dAvgTime[i-1] += dTime;
            /*printf("The calculating using %d threads took %.5f seconds\n", iThreadNum, dTime);

            printf("Pi = %.10f\n", dPi);*/
        }
    }

    printf("Number of threads - taken time\n1 - %.10f\n2 - %.10f\n4 - %.10f\n8 - %.10f\n16 - %.10f", dAvgTime[0]/TIMES, dAvgTime[1]/TIMES, dAvgTime[2]/TIMES, dAvgTime[3]/TIMES, dAvgTime[4]/TIMES);
    return 0;
}