#include <iostream>
#include <stdio.h>
#include <windows.h>
#include <omp.h>

#define BLOCKSIZE 9308

const int N = 100000000;

int main()
{
    int iThreadsNum = 1;
    int i;
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
        std::cout << "The calculating using " << iThreadsNum << " threads took " << ((double)(GetTickCount()-start)/1000) << " seconds\n";
        //std::cout << dPi/N << '\n';
        printf("%.50f\n", dPi/N);
        if(iThreadsNum != 16)
            Sleep(1000);
    }
    return 0;
}