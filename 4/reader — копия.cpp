#include <iostream>
#include <windows.h>

#define GLOBALWRITESEMAPHORE "globalWriteSemaphore"
#define GLOBALREADSEMAPHORE "globalReadSemaphore"
#define WRITEMUTEXNAME(num) ("writemutex" + std::to_string(num))
#define READMUTEXNAME(num) ("readmutex" + std::to_string(num))
#define PAGE_NUMBER 20
#define PAGE_SIZE 4096
#define FILE_MAP_NAME "lab4bufFile"

bool InitMutexesAndSemaphore();
void CloseMutexesAndSemaphore();

HANDLE  g_hReadMutex[PAGE_NUMBER],
        g_hReadGlobalSemaphore;

int main()
{
    if(InitMutexesAndSemaphore())
    {
        std::cout << "All good :)\n";
        HANDLE hFileMap = CreateFileMappingA(NULL, NULL, PAGE_READWRITE, 0UL, 512UL, FILE_MAP_NAME);
        if(hFileMap != NULL)
        {
            HANDLE hWriteGlobalSemaphore = OpenSemaphoreA(SEMAPHORE_MODIFY_STATE, true, GLOBALWRITESEMAPHORE);
            if(hWriteGlobalSemaphore != NULL)
            {
                ReleaseSemaphore(hWriteGlobalSemaphore, 1, NULL);
                DWORD dwWait = WaitForSingleObject(hWriteGlobalSemaphore, 10000UL);
                if(dwWait == WAIT_OBJECT_0)
                {
                    HANDLE hWriteMutexes[PAGE_NUMBER];
                    bool bContinue = true;
                    for(int i = 0; i < PAGE_NUMBER; ++i)
                    {
                        hWriteMutexes[i] = OpenMutexA(MUTEX_MODIFY_STATE, false, WRITEMUTEXNAME(i).c_str());
                        if(hReadMutexes[i] == NULL)
                        {
                            std::cout << "Failed to get write mutexes! Error code is " << GetLastError() << "\n";
                            for(int j = 0; j < i; ++j)
                                CloseHandle(hReadMutexes[j]);
                            bContinue = false;
                        }
                    }
                    if(bContinue)
                    {
                        ReleaseSemaphore(hWriteGlobalSemaphore, 1, NULL);
                        dwWait = WaitForSingleObject(g_hReadGlobalSemaphore, 2000UL);
                        if(dwWait == WAIT_OBJECT_0)
                        {
                            //Let's go!
                        }
                    }
                }
                else
                {
                    std::cout << "Too much time wasted waiting for writer!";
                }
            }
            else
            {
                std::cout << "Failed to get global write semaphore! Error code is " << GetLastError() << "\n";
            }
        }
        else
        {
            std::cout << "Failed to create file mapping! Error code is " << GetLastError() << '\n';
        }
        CloseMutexesAndSemaphore();
    }
    else
    {
        std::cout << "Due to failure during mutex creation reader is going down!\n";
        system("pause");
    }
    return 0;
}

bool InitMutexesAndSemaphore()
{
    for(int i = 0; i < PAGE_NUMBER; ++i)
    {
        g_hReadMutex[i] = CreateMutexA(NULL, true, READMUTEXNAME(i).c_str());
        if(g_hReadMutex[i] == INVALID_HANDLE_VALUE)
        {
            std::cout << "Failed to create #" << i << " read mutex. Error code is " << GetLastError() << '\n';
            for(int j = 0; j < i; ++j)
            {
                CloseHandle(g_hReadMutex[j]);
            }
            return false;
        }
    }
    g_hReadGlobalSemaphore = CreateSemaphoreA(NULL, 0, 1, GLOBALREADSEMAPHORE);
    if(g_hReadGlobalSemaphore == NULL)
    {
        std::cout << "Failed to create global read semaphore. Error code is " << GetLastError() << '\n';
        CloseMutexesAndSemaphore();
        return false;
    }
    return true;
}

void CloseMutexesAndSemaphore()
{
    for(int i = 0; i < PAGE_NUMBER; ++i)
    {
        CloseHandle(g_hReadMutex[i]);
    }
    if(g_hReadGlobalSemaphore != NULL)
    {
        CloseHandle(g_hReadGlobalSemaphore);
    }
}