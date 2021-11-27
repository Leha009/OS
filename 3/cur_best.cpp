#include <iostream>
#include <stdio.h>
#include <windows.h>

#define BLOCKSIZE 930823

const int N = 100000000;

DWORD WINAPI CalculatePi(LPVOID lpParameter);

struct test
{
    int* iCount;
    int* iCurrentN;
    double dPi;
};

int main()
{
    for(int iThreadNum = 1, k = 1; iThreadNum <= 16; iThreadNum = 1 << (k++))
    {
        int iCount = iThreadNum;
        int iCurrentN = 0;
        test* t = new test[iThreadNum];
        HANDLE* hThread = new HANDLE[iThreadNum];
        for(int i = 0; i < iThreadNum; ++i)
        {
            hThread[i] = CreateThread(NULL, 0, CalculatePi, &t[i], CREATE_SUSPENDED, NULL);
            t[i].iCount = &iCount;
            t[i].dPi = 0.0;
            t[i].iCurrentN = &iCurrentN;
        }

        DWORD start = GetTickCount();

        for(int i = 0; i < iThreadNum; ++i)
            ResumeThread(hThread[i]);

        WaitForMultipleObjects(iThreadNum, hThread, true, INFINITE);


        double num = 0.0;
        for(int i = 0; i < iThreadNum; ++i)
            num += t[i].dPi;
        printf("The calculating using %d threads took %.10f seconds\n", iThreadNum, (double)(GetTickCount()-start)/1000);

        printf("Pi = %.50f\n", num);

        for(int i = 0; i < iThreadNum; ++i)
            CloseHandle(hThread[i]);
        delete hThread;
        delete t;

        Sleep(1000);
    }

    //====================== Прямой цикл, без потоков
    double dPi = 0.0;
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
    test* t = (test*)lpParameter;
    double dN = 1.0/N;
    double doubleN = (double)N*N;
    double x;
    int iStop;
    while(*(t->iCurrentN) < N)
    {
        *(t->iCurrentN) += BLOCKSIZE;
        iStop = *(t->iCurrentN);
        for(int i = iStop-BLOCKSIZE; i < N && i < iStop; ++i)
        {
            x = (0.5+i)*dN;
            t->dPi += 4.0/(1.0 + x*x);
            //t->dPi += 4.0/(0.25+doubleN+i+i*i);   //BAD
        }
    }
    t->dPi *= dN;
    return 0;
}