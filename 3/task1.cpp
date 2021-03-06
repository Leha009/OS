#include <iostream>
#include <stdio.h>
#include <windows.h>
#include <mutex>

#define BLOCKSIZE 9308230   // Размер блока
#define TIMES 10           // Число замеров

const int N = 100000000;

DWORD WINAPI CalculatePi(LPVOID lpParameter);

double dPi;
std::mutex mIncreaseN;

int main()
{
    double dAvgTime[7] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
    int iCurrentN;
    int k = 1;
    for(int iThreadNum = 1; iThreadNum <= 64; iThreadNum = 1 << (k++))
    {
        std::cout << iThreadNum << " threads running...\n";
        for(int times = 0; times < TIMES; ++times)
        {
            iCurrentN = 0;
            dPi = 0.0;
            HANDLE* hThread = new HANDLE[iThreadNum];
            if(hThread != NULL)
            {
                for(int i = 0; i < iThreadNum; ++i)
                {
                    hThread[i] = CreateThread(NULL, 0, CalculatePi, &iCurrentN, CREATE_SUSPENDED, NULL);
                }

                DWORD start = GetTickCount();

                for(int i = 0; i < iThreadNum; ++i)
                    ResumeThread(hThread[i]);

                WaitForMultipleObjects(iThreadNum, hThread, true, INFINITE);


                /*double num = 0.0;
                for(int i = 0; i < iThreadNum; ++i)
                    num += pData[i].dPi;*/

                double dTime = (double)(GetTickCount()-start)/1000;
                dAvgTime[k-1] += dTime;
                if(TIMES == 1)
                {
                    printf("The calculating using %d threads took %.5f seconds\n", iThreadNum, dTime);

                    printf("Pi = %.10f\n", dPi);
                    //printf("Number = %d\n", iTest);
                }

                for(int i = 0; i < iThreadNum; ++i)
                    CloseHandle(hThread[i]);
                delete hThread;
            }
        }
        printf("%.10f\n", dPi);
    }

    if(TIMES > 1)
    {
        for(int i = 0; i < k-1; ++i)
            printf("%d - %.10f seconds\n", (1 << i), dAvgTime[i]/TIMES);
        //printf("Number of threads - taken time\n1 - %.10f\n2 - %.10f\n4 - %.10f\n8 - %.10f\n16 - %.10f", dAvgTime[0]/TIMES, dAvgTime[1]/TIMES, dAvgTime[2]/TIMES, dAvgTime[3]/TIMES, dAvgTime[4]/TIMES);
    }
    //====================== Прямой цикл, без потоков
    /*dPi = 0.0;
    double x;
    for(int i = 0; i < N; ++i)
    {
        x = (i+0.5)/N;
        dPi += 4.0/(1.0 + x*x);
    }
    printf("\n\tOne for\nPi = %.10f\n", dPi/N);*/
    //================================
    
    return 0;
}

DWORD WINAPI CalculatePi(LPVOID lpParameter)
{
    int*   iCurrentN = (int*)lpParameter;
    /*double  dN = 1.0/N;
    double  x,*/
    double dLocPi = 0.0;
    int     iStop = 0;
    while(iStop < N)
    {
        mIncreaseN.lock();
        *(iCurrentN) += BLOCKSIZE;
        iStop = *(iCurrentN);
        mIncreaseN.unlock();
        for(int i = iStop-BLOCKSIZE; i < N && i < iStop; ++i)
        {
            /*x = (0.5+i)*dN;
            dLocPi += 4.0/(1.0 + x*x);*/
            dLocPi += 4.0/(1.0 + ((i+0.5)/N)*((i+0.5)/N));
        }
    }
    //dLocPi *= dN;
    dPi += dLocPi/N;
    return 0;
}