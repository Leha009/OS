#include <iostream>
#include <windows.h>

#define BLOCKSIZE 930823
//#define BLOCKSIZE 9308

struct ThreadEx
{
    HANDLE hThread; 
    double dPi = 0.0;
    int* iLastAvailableBlock;
    int iCurrentBlock = 0;  //BLOCKSIZE*i
    int iThreadNum = 0;
};

DWORD WINAPI CalculatePi(LPVOID lpParameter);

ThreadEx* hThreadsEx;

const int N = 100000000;

int main()
{
    int iThreadsNum, i;
    for(iThreadsNum = 1, i = 1; iThreadsNum <= 16; iThreadsNum = 1 << (i++))
    {
        hThreadsEx = new ThreadEx[iThreadsNum];
        HANDLE* hThreadsList = new HANDLE[iThreadsNum];
        bool bInitSuccessfull = true;
        if(hThreadsEx != NULL)
        {
            int iLastAvailableBlock = 0;
            for(int i = 0; bInitSuccessfull && i < iThreadsNum; ++i)
            {
                hThreadsEx[i].iCurrentBlock = i*BLOCKSIZE;
                hThreadsEx[i].iLastAvailableBlock = &iLastAvailableBlock;
                hThreadsEx[i].iThreadNum = i;
                hThreadsEx[i].hThread = CreateThread(NULL, 0, CalculatePi, &hThreadsEx[i].iThreadNum, CREATE_SUSPENDED, NULL);
                if(hThreadsEx[i].hThread == NULL)
                {
                    bInitSuccessfull = false;
                    std::cout << "Failed to create a thread! Error code is " << GetLastError() << '\n';
                    for(int j = 0; j < i; ++j)
                        CloseHandle(hThreadsEx[i].hThread);
                }
                hThreadsList[i] = hThreadsEx[i].hThread;
            }
            if(bInitSuccessfull)
            {
                DWORD start = GetTickCount();

                for(int i = 0; i < iThreadsNum; ++i)
                {
                    ResumeThread(hThreadsEx[i].hThread);
                }
                
                WaitForMultipleObjects(iThreadsNum, hThreadsList, true, INFINITE);

                std::cout << "The calculating using " << iThreadsNum << " threads took " << ((double)(GetTickCount()-start)/1000) << " seconds\n";

                double dPi = 0.0;

                for(int i = 0; i < iThreadsNum; ++i)
                    dPi += hThreadsEx[i].dPi;
                
                std::cout << "Pi = " << dPi/N << '\n';

                /*dPi = 0.0;        Прямой цикл, без потоков
                for(int i = 0; i < N; ++i)
                {
                    dPi += 4.0/(1.0 + ((i+0.5)/N)*((i+0.5)/N));
                }

                std::cout << dPi/N << '\n';*/

                for(int i = 0; i < iThreadsNum; ++i)
                    CloseHandle(hThreadsEx[i].hThread);
            }
            delete hThreadsEx;
            delete hThreadsList;
        }
        if(iThreadsNum != 16)
            Sleep(5000);
    }
    return 0;
}

DWORD WINAPI CalculatePi(LPVOID lpParameter)
{
    int* lpThreadNum = (int*)lpParameter;
    int iThread = *lpThreadNum;
    hThreadsEx[iThread].dPi = 0.0;
    //std::cout << "Thread " << iThread << ": " << hThreadsEx[iThread].iCurrentBlock << " | " << hThreadsEx[iThread].dPi << '\n';   //DEBUG
    double x;
    while(hThreadsEx[iThread].iCurrentBlock < N)
    {
        *hThreadsEx[iThread].iLastAvailableBlock += BLOCKSIZE;
        for(int i = hThreadsEx[iThread].iCurrentBlock; i < N && i < hThreadsEx[iThread].iCurrentBlock + BLOCKSIZE; ++i)
        {
            x = (i+0.5)/N;
            hThreadsEx[iThread].dPi += 4.0/(1.0 + x*x);
        }
        hThreadsEx[iThread].iCurrentBlock = *hThreadsEx[iThread].iLastAvailableBlock;
    }
    return 0;
}