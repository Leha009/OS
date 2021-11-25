#include <iostream>
#include <windows.h>

#define BLOCKSIZE 930823

//const int N = 100000000;
const int N = 10;

DWORD WINAPI CalculatePi(LPVOID lpParameter);

int iLastBlock = BLOCKSIZE;

struct test
{
    int i;
    int* pi;
    HANDLE h;
};

int main()
{
    int i = 0;
    test t;
    HANDLE hThread = CreateThread(NULL, 0, CalculatePi, &t, CREATE_SUSPENDED, NULL);
    t.h = hThread;
    t.i = 0;
    t.pi = &i;
    std::cout << hThread << '\n';
    std::cout << "Res " << ResumeThread(hThread) << '\n';
    while(i < N)
    {
        ResumeThread(hThread);
    }
    //WaitForMultipleObjects(1, &hThread, true, INFINITE);
    std::cout << t.i << " | OUT\n";
    CloseHandle(hThread);
    return 0;
}

DWORD WINAPI CalculatePi(LPVOID lpParameter)
{
    test* t = (test*)lpParameter;
    while(t->i != 10)
    {
        std::cout << *(t->pi) << '\n';
        t->i += 1;
        *(t->pi) = *(t->pi)+1;
        std::cout << t->h << '\n';
        SuspendThread(t->h);
        //std::cout << "Sus " << SuspendThread(t->h);
        //std::cout << "Res " << ResumeThread(t->h) << '\n';
    }
    return 0;
}