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
        //std::cout << "START\n";
        int iCount = iThreadNum;
        int iCurrentN = 0;
        test* t = new test[iThreadNum];
        HANDLE* hThread = new HANDLE[iThreadNum];
        for(int i = 0; i < iThreadNum; ++i)
        {
            //std::cout << "In1\n";
            hThread[i] = CreateThread(NULL, 0, CalculatePi, &t[i], CREATE_SUSPENDED, NULL);
            //std::cout << "In2\n";
            t[i].iCount = &iCount;
            t[i].dPi = 0.0;
            t[i].iCurrentN = &iCurrentN;
            //std::cout << "In3\n";
        }
        //std::cout << "START2\n";
        DWORD start = GetTickCount();

        for(int i = 0; i < iThreadNum; ++i)
            ResumeThread(hThread[i]);

        ///while(iCount != iThreadNum);
        WaitForMultipleObjects(iThreadNum, hThread, true, INFINITE);

        //std::cout << "The calculating using " << iThreadNum << " threads took " << ((double)(GetTickCount()-start)/1000) << " seconds\n";
        printf("The calculating using %d threads took %.50f seconds\n", iThreadNum, (double)(GetTickCount()-start)/1000);

        double num = 0.0;
        for(int i = 0; i < iThreadNum; ++i)
            num += t[i].dPi;

        //std::cout << "Pi = " << num/N << '\n';
        printf("Pi = %.50f\n", num/N);

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
    //std::cout << "Real pi is " << dPi/N << '\n';
    printf("Pi = %.50f\n", dPi/N);
    //================================
    
    return 0;
}

DWORD WINAPI CalculatePi(LPVOID lpParameter)
{
    test* t = (test*)lpParameter;
    double x;
    //std::cout << t->bInProcess << '\n';
    //*(t->iCount) = *(t->iCount)-1;
    int iStop;
    while(*(t->iCurrentN) < N)
    {
        *(t->iCurrentN) += BLOCKSIZE;
        iStop = *(t->iCurrentN);
        for(int i = iStop-BLOCKSIZE; i < N && i < iStop; ++i)
        {
            x = (i+0.5)/N;
            t->dPi += 4.0/(1.0 + x*x);
        }
    }
    /*int iToAdd = BLOCKSIZE*t->iThreads;
    while(t->iStart < N)
    {
        for(int i = t->iStart; i < N && i < t->iStop; ++i)
        {
            x = (i+0.5)/N;
            t->dPi += 4.0/(1.0 + x*x);
        }
        t->iStart += iToAdd;
        t->iStop += iToAdd;
    }*/
    //*(t->iCount) = *(t->iCount)+1;
    return 0;
}