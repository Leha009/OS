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

HANDLE FileMap();

HANDLE  g_hWriteMutex[PAGE_NUMBER],
        g_hReadMutex[PAGE_NUMBER],
        g_hWriteGlobalSemaphore;

int main()
{
    if(InitMutexesAndSemaphore())
    {
        std::cout << "All good :)\n";
        HANDLE hFileMap = CreateFileMappingA(NULL, NULL, PAGE_READWRITE, 0UL, PAGE_NUMBER*PAGE_SIZE, FILE_MAP_NAME);
        if(hFileMap != NULL)
        {
            char* lpCharFileView = (char*)MapViewOfFile(hFileMap, FILE_MAP_WRITE, 0UL, 0UL, 0UL);
            if(lpCharFileView != NULL)
            {
                if(VirtualLock(lpCharFileView, PAGE_NUMBER*PAGE_SIZE))
                {
                    HANDLE hReadMutexes[PAGE_NUMBER];
                    bool bContinue = true;
                    DWORD dwWait = WaitForSingleObject(g_hWriteGlobalSemaphore, 10000UL);
                    if(dwWait == WAIT_OBJECT_0)
                    {
                        ReleaseSemaphore(g_hWriteGlobalSemaphore, 1, NULL);
                        HANDLE hReadGlobalSemaphore = OpenSemaphoreA(SEMAPHORE_MODIFY_STATE, true, GLOBALREADSEMAPHORE);
                        if(hReadGlobalSemaphore != NULL)
                        {
                            dwWait = WaitForSingleObject(hReadGlobalSemaphore, 2000UL);
                            if(dwWait == WAIT_OBJECT_0)
                            {
                                for(int i = 0; i < PAGE_NUMBER; ++i)
                                {
                                    hReadMutexes[i] = OpenMutexA(MUTEX_MODIFY_STATE, false, READMUTEXNAME(i).c_str());
                                    if(hReadMutexes[i] == NULL)
                                    {
                                        std::cout << "Failed to get read mutexes! Error code is " << GetLastError() << "\n";
                                        for(int j = 0; j < i; ++j)
                                            CloseHandle(hReadMutexes[j]);
                                        bContinue = false;
                                    }
                                }
                                if(bContinue)
                                {
                                    ReleaseSemaphore(hReadGlobalSemaphore, 1, NULL);
                                    dwWait = WaitForSingleObject(g_hWriteGlobalSemaphore, 2000UL);
                                    if(dwWait == WAIT_OBJECT_0)
                                    {
                                        //Let's go!
                                    }
                                }
                            }
                            else
                            {
                                std::cout << "Too much time wasted waiting for global read semaphore!\n";
                            }
                        }
                        else
                        {
                            std::cout << "Failed to get global read semaphore! Error code is " << GetLastError() << "\n";
                        }
                    }
                    else
                    {
                        std::cout << "Too much time wasted waiting for read mutexes!\n";
                    }
                }
                else
                {
                    std::cout << "Failed to lock virtual space! Error code is " << GetLastError() << '\n';
                }
                UnmapViewOfFile(lpCharFileView);
            }
            else
            {
                std::cout << "Failed to map view of file! Error code is " << GetLastError() << '\n';
            }
            CloseHandle(hFileMap);
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
        g_hWriteMutex[i] = CreateMutexA(NULL, true, WRITEMUTEXNAME(i).c_str());
        if(g_hWriteMutex[i] == INVALID_HANDLE_VALUE)
        {
            std::cout << "Failed to create #" << i << " write mutex. Error code is " << GetLastError() << '\n';
            for(int j = 0; j < i; ++j)
            {
                CloseHandle(g_hWriteMutex[j]);
            }
            return false;
        }
    }
    g_hWriteGlobalSemaphore = CreateSemaphoreA(NULL, 0, 1, GLOBALWRITESEMAPHORE);
    if(g_hWriteGlobalSemaphore == NULL)
    {
        std::cout << "Failed to create global write semaphore. Error code is " << GetLastError() << '\n';
        CloseMutexesAndSemaphore();
        return false;
    }
    return true;
}

void CloseMutexesAndSemaphore()
{
    for(int i = 0; i < PAGE_NUMBER; ++i)
    {
        CloseHandle(g_hWriteMutex[i]);
    }
    if(g_hWriteGlobalSemaphore != NULL)
    {
        CloseHandle(g_hWriteGlobalSemaphore);
    }
}

bool WaitForStart()
{
    DWORD dwWait = WaitForSingleObject(g_hWriteGlobalSemaphore, 10000UL);
    if(dwWait == WAIT_OBJECT_0)
    {
        return true;
    }
    else
    {
        std::cout << "Too much time wasted waiting for read mutexes!\n";
        return false;
    }
}