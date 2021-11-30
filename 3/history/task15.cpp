#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

#define BLOCKSIZE 9308230

const int N = 100000000;

DWORD WINAPI CalculatePi(LPVOID lpParameter);

struct test
{
    int iStartBlock;
    int iThreadNum;
    double dPi;
};

double dPi;

int main()
{
    double dAvgTime[5] = {0.0, 0.0, 0.0, 0.0, 0.0};
    int iCount;
    for(int times = 0; times < 20; ++times)
    {
        for(int iThreadNum = 1, k = 1; iThreadNum <= 16; iThreadNum = 1 << (k++))
        {
            dPi = 0.0;
            iCount = iThreadNum;
            test* pTest = (test*)malloc(sizeof(test)*iThreadNum);
            HANDLE* hThread = (HANDLE*)malloc(sizeof(HANDLE)*iThreadNum);
            if(hThread != NULL && pTest != NULL)
            {
                for(int i = 0; i < iThreadNum; ++i)
                {
                    pTest[i].iStartBlock = i*BLOCKSIZE;
                    pTest[i].dPi = 0.0;
                    pTest[i].iThreadNum = iThreadNum;
                    hThread[i] = CreateThread(NULL, 0, CalculatePi, &pTest[i], CREATE_SUSPENDED, NULL);
                }

                DWORD start = GetTickCount();

                for(int i = 0; i < iThreadNum; ++i)
                    ResumeThread(hThread[i]);

                WaitForMultipleObjects(iThreadNum, hThread, true, INFINITE);

                double dTime = (double)(GetTickCount()-start)/1000;
                dAvgTime[k-1] += dTime;

                /*printf("The calculating using %d threads took %.10f seconds\n", iThreadNum, dTime);
                printf("Pi = %.12f\n", dPi);*/

                for(int i = 0; i < iThreadNum; ++i)
                    CloseHandle(hThread[i]);
            }
            if(hThread != NULL)
            {
                free(hThread);
                hThread = NULL;
            }
            if(pTest != NULL)
            {
                free(pTest);
                pTest = NULL;
            }
        }
    }

    printf("1 - %.10f\n2 - %.10f\n4 - %.10f\n8 - %.10f\n16 - %.10f", dAvgTime[0]/20, dAvgTime[1]/20, dAvgTime[2]/20, dAvgTime[3]/20, dAvgTime[4]/20);

    //====================== Прямой цикл, без потоков
    dPi = 0.0;
    double x;
    for(int i = 0; i < N; ++i)
    {
        x = (i+0.5)/N;
        dPi += 4.0/(1.0 + x*x);
    }
    printf("\n\tOne for\nPi = %.50f\n", dPi/N);
    //================================
    
    return 0;
}

DWORD WINAPI CalculatePi(LPVOID lpParameter)
{
    test* pTest = (test*)lpParameter;
    double dN = 1.0/N;
    double x;
    int iStop;
    while(pTest->iStartBlock < N)
    {
        for(int i = pTest->iStartBlock; i < N && i < pTest->iStartBlock+BLOCKSIZE; ++i)
        {
            x = (0.5+i)*dN;
            pTest->dPi += 4.0/(1.0 + x*x);
        }
        pTest->iStartBlock += BLOCKSIZE*pTest->iThreadNum;
    }
    pTest->dPi *= dN;
    dPi += pTest->dPi;
    return 0;
}