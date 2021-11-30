#include <iostream>
#include <stdio.h>
#include <windows.h>

#define BLOCKSIZE 9308230

const int N = 100000000;
//const int N = 10000000;

DWORD WINAPI CalculatePi(LPVOID lpParameter);

struct test
{
    int iBlock;
    int* iLastBlock;
    int* iCount;
    HANDLE h;
    double dPi;
};

int main()
{
    for(int iThreadNum = 1, k = 1; iThreadNum <= 16; iThreadNum = 1 << (k++))
    {
        int iNum = 0;
        int iCount = iThreadNum;
        test* t = new test[iThreadNum];
        HANDLE* hThread = new HANDLE[iThreadNum];
        for(int i = 0; i < iThreadNum; ++i)
        {
            hThread[i] = CreateThread(NULL, 0, CalculatePi, &t[i], CREATE_SUSPENDED, NULL);
            t[i].h = hThread[i];
            t[i].iLastBlock = &iNum;
            t[i].iBlock = iNum;
            //iNum += BLOCKSIZE;
            t[i].dPi = 0.0;
            t[i].iCount = &iCount;
        }

        DWORD start = GetTickCount();

        while(iNum < N)
        {
            for(int i = 0; i < iThreadNum; ++i)
            {
                ResumeThread(hThread[i]);
            }
        }

        while(iCount != iThreadNum);

        std::cout << "The calculating using " << iThreadNum << " threads took " << ((double)(GetTickCount()-start)/1000) << " seconds\n";

        double num = 0.0;
        for(int i = 0; i < iThreadNum; ++i)
            num += t[i].dPi;

        printf("Pi = %.50f\n", num/N);

        delete hThread;
        delete t;

        Sleep(500);

    }

    //====================== Прямой цикл, без потоков
    double dPi = 0.0;
    for(int i = 0; i < N; ++i)
    {
        dPi += 4.0/(1.0 + ((i+0.5)/N)*((i+0.5)/N));
    }
    std::cout << dPi/N << '\n';
    //================================
    
    return 0;
}

DWORD WINAPI CalculatePi(LPVOID lpParameter)
{
    test* t = (test*)lpParameter;
    double dN = 1.0/N;
    double x;
    //while(*(t->iLastBlock) < N)
    while(t->iBlock < N)
    {
        *(t->iCount) = *(t->iCount)-1;
        t->iBlock = *(t->iLastBlock);
        *(t->iLastBlock) = *(t->iLastBlock)+BLOCKSIZE;
        //std::cout << t->iBlock << " | " << *(t->iLastBlock) << '\n';
        for(int i = t->iBlock; i < N && i < t->iBlock+BLOCKSIZE; ++i)
        {
            /*if(i == t->iBlock+BLOCKSIZE-1 || i == N-1)
                std::cout << i << " | N = " << N << " | iBlock = " << t->iBlock << '\n';*/
            x = (0.5+i)*dN;
            t->dPi += 4.0/(1.0 + x*x);
        }
        *(t->iCount) = *(t->iCount)+1;
        SuspendThread(t->h);
    }
    return 0;
}