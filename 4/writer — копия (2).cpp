#include <iostream>
#include <windows.h>
#include <mutex>
#include <ctime>

#define _LogEnable 1
#ifdef _LogEnable
#define _LogToFile 1
#endif

#define MAPPED_FILE_NAME "bufferFile"
#define BUF_PAGES 16UL
#define SLEEP_TIME 500 + rand() % 1001
#define THREAD_NUM 2

bool LogToFile(HANDLE hFile, LPCSTR lpStringToWrite, DWORD dwStringLength);
bool WriteToMemory(DWORD iThreadNum);
DWORD WINAPI WriteViaThread(LPVOID lpThreadNum);
bool ReadFromMemory(DWORD iThreadNum);
DWORD WINAPI ReadViaThread(LPVOID lpThreadNum);

HANDLE  g_hWriteSemaphore[BUF_PAGES],
        g_hReadSemaphore[BUF_PAGES],
        g_hLogFile;

char g_cSymbolToPage[BUF_PAGES];

int main()
{
    srand(time(NULL));
    HANDLE  hLogFile,
            hMappedFile,
            hWritersThreads[THREAD_NUM],
            hReadersThreads[THREAD_NUM];
    DWORD   dwPageSize;
    SYSTEM_INFO sSysInfo;
    GetSystemInfo(&sSysInfo);
    dwPageSize = sSysInfo.dwPageSize;
    #ifdef _LogToFile
    g_hLogFile = CreateFileA("actions.log", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if(g_hLogFile == INVALID_HANDLE_VALUE)
    {
        std::cout << "Failed to create log file! Error code is " << GetLastError() << '\n';
        system("pause");
        return 1;
    }
    #endif

    hMappedFile = CreateFileMappingA(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0UL, BUF_PAGES*dwPageSize, MAPPED_FILE_NAME);
    if(hMappedFile != INVALID_HANDLE_VALUE)
    {
        LPVOID lpFileView = MapViewOfFile(hMappedFile, FILE_MAP_WRITE, 0UL, 0UL, BUF_PAGES*dwPageSize);
        if(lpFileView != NULL)
        {
            if(VirtualLock(lpFileView, BUF_PAGES*dwPageSize))
            {
                bool bStartProcess = true;
                for(int i = 0; i < BUF_PAGES; ++i)
                {
                    g_hWriteSemaphore[i] = CreateSemaphoreA(NULL, 1, 1, NULL);
                    if(g_hWriteSemaphore[i] == INVALID_HANDLE_VALUE)
                    {
                        for(int j = 0; j < i; ++j)
                        {
                            CloseHandle(g_hWriteSemaphore[i]);
                            if(j < i-1)
                                CloseHandle(g_hReadSemaphore[i]);
                        }
                        bStartProcess = false;
                        std::cout << "Failed to create write semaphore! Error code is " << GetLastError() << '\n';
                    }
                    else
                    {
                        g_hReadSemaphore[i] = CreateSemaphoreA(NULL, 0, 1, NULL);
                        if(g_hReadSemaphore[i] == INVALID_HANDLE_VALUE)
                        {
                            for(int j = 0; j < i; ++j)
                            {
                                CloseHandle(g_hWriteSemaphore[i]);
                                CloseHandle(g_hReadSemaphore[i]);
                            }
                            bStartProcess = false;
                            std::cout << "Failed to create read semaphore! Error code is " << GetLastError() << '\n';
                        }
                    }
                }
                if(bStartProcess)
                {
                    for(int i = 0; i < BUF_PAGES; ++i)
                    {
                        g_cSymbolToPage[i] = (char)(65+i);
                    }
                    for(int i = 0; i < THREAD_NUM; ++i)
                    {
                        hWritersThreads[i] = CreateThread(NULL, 0, WriteViaThread, NULL, 0, NULL);
                        hReadersThreads[i] = CreateThread(NULL, 0, ReadViaThread, NULL, 0, NULL);
                    }
                    WaitForMultipleObjects(THREAD_NUM, hWritersThreads, true, INFINITE);
                    WaitForMultipleObjects(THREAD_NUM, hReadersThreads, true, INFINITE);

                    for(int i = 0; i < THREAD_NUM; ++i)
                    {
                        CloseHandle(hWritersThreads[i]);
                        CloseHandle(hReadersThreads[i]);
                    }

                    for(int i = 0; i < BUF_PAGES; ++i)
                    {
                        CloseHandle(g_hWriteSemaphore[i]);
                        CloseHandle(g_hReadSemaphore[i]);
                    }
                }                
                VirtualUnlock(lpFileView, BUF_PAGES*dwPageSize);
            }
            UnmapViewOfFile(lpFileView);
        }
        else
        {
            std::cout << "Failed to create view of the mapped file! Error code is " << GetLastError() << '\n';
        }
        CloseHandle(hMappedFile);
    }
    else
    {
        std::cout << "Failed to create mapped file! Error code is " << GetLastError() << '\n';
    }

    #ifdef _LogToFile
    CloseHandle(g_hLogFile);
    #endif

    return 0;
}

bool LogToFile(HANDLE hFile, LPCSTR lpStringToWrite, DWORD dwStringLength)
{
    DWORD dwWrittenBytes;
    bool bSuccess = WriteFile(hFile, lpStringToWrite, dwStringLength, &dwWrittenBytes, NULL);
    if(!bSuccess)
    {
        std::cout << "Failed to log to the file! Error code is " << GetLastError() << '\n';
        return false;
    }
    if(dwStringLength != dwWrittenBytes)
    {
        std::cout << "Number of the written bytes isn't equal to the string length!\n";
    }
    return true;
}

bool WriteToMemory(DWORD iThreadNum)
{
    #ifdef _LogToFile
    std::string sLogAction;
    sLogAction = std::to_string(GetTickCount()) + " | Writing Thread #" + std::to_string(iThreadNum) + ": waiting...\n";
    LogToFile(g_hLogFile, sLogAction.c_str(), sLogAction.length());
    #elif _LogEnable
    std::cout << "Writing Thread #" << iThreadNum << ": waiting...\n";
    #endif

    DWORD dwWaitResult;
    while(true)
    {
        for(int i = 0; i < BUF_PAGES; ++i)
        {
            dwWaitResult = WaitForSingleObject(g_hWriteSemaphore[i], 1UL);
            if(dwWaitResult == WAIT_OBJECT_0)
            {
                #ifdef _LogToFile
                sLogAction = std::to_string(GetTickCount()) + " | Writing Thread #" + std::to_string(iThreadNum) + ": writing something to page " + std::to_string(i) + "...\n";
                LogToFile(g_hLogFile, sLogAction.c_str(), sLogAction.length());
                #elif _LogEnable
                std::cout << "Writing Thread #" << iThreadNum << ": writing something... to page #" << i << "...\n";
                #endif

                Sleep(SLEEP_TIME);

                #ifdef _LogToFile
                sLogAction = std::to_string(GetTickCount()) + " | Writing Thread #" + std::to_string(iThreadNum) + ": releasing semaphore...\n";
                LogToFile(g_hLogFile, sLogAction.c_str(), sLogAction.length());
                #elif _LogEnable
                std::cout << "Writing Thread #" << iThreadNum << ": releasing semaphore...\n";
                #endif

                if(!ReleaseSemaphore(g_hReadSemaphore[i], 1, NULL))
                {
                    std::cout << "Failed to release the semaphore! Error code is " << GetLastError() << '\n';
                    return false;
                }

                #ifdef _LogToFile
                sLogAction = std::to_string(GetTickCount()) + " | Writing Thread #" + std::to_string(iThreadNum) + ": successfully released semaphore...\n";
                LogToFile(g_hLogFile, sLogAction.c_str(), sLogAction.length());
                #elif _LogEnable
                std::cout << "Writing Thread #" << iThreadNum << ": released semaphore...\n";
                #endif

                return true;
            }
        }
    }
    return false;
}

DWORD WINAPI WriteViaThread(LPVOID lpThreadNum)
{
    DWORD iThreadNum = GetCurrentThreadId();
    for(int i = 0; i < 2; ++i)
        WriteToMemory(iThreadNum);
    return 0UL;
}

bool ReadFromMemory(DWORD iThreadNum)
{
    #ifdef _LogToFile
    std::string sLogAction;
    sLogAction = std::to_string(GetTickCount()) + " | Reading Thread #" + std::to_string(iThreadNum) + ": waiting...\n";
    LogToFile(g_hLogFile, sLogAction.c_str(), sLogAction.length());
    #elif _LogEnable
    std::cout << "Reading Thread #" << iThreadNum << ": waiting...\n";
    #endif

    DWORD dwWaitResult;
    while(true)
    {
        for(int i = 0; i < BUF_PAGES; ++i)
        {
            dwWaitResult = WaitForSingleObject(g_hReadSemaphore[i], 1UL);
            if(dwWaitResult == WAIT_OBJECT_0)
            {
                #ifdef _LogToFile
                sLogAction = std::to_string(GetTickCount()) + " | Reading Thread #" + std::to_string(iThreadNum) + ": reading something from page #" + std::to_string(i) + "...\n";
                LogToFile(g_hLogFile, sLogAction.c_str(), sLogAction.length());
                #elif _LogEnable
                std::cout << "Reading Thread #" << iThreadNum << ": reading something from page #" << i << "...\n";
                #endif

                Sleep(SLEEP_TIME);

                #ifdef _LogToFile
                sLogAction = std::to_string(GetTickCount()) + " | Reading Thread #" + std::to_string(iThreadNum) + ": releasing semaphore...\n";
                LogToFile(g_hLogFile, sLogAction.c_str(), sLogAction.length());
                #elif _LogEnable
                std::cout << "Reading Thread #" << iThreadNum << ": releasing semaphore...\n";
                #endif

                if(!ReleaseSemaphore(g_hWriteSemaphore[i], 1, NULL))
                {
                    std::cout << "Failed to release the semaphore! Error code is " << GetLastError() << '\n';
                    return false;
                }

                #ifdef _LogToFile
                sLogAction = std::to_string(GetTickCount()) + " | Reading Thread #" + std::to_string(iThreadNum) + ": successfully released semaphore...\n";
                LogToFile(g_hLogFile, sLogAction.c_str(), sLogAction.length());
                #elif _LogEnable
                std::cout << "Reading Thread #" << iThreadNum << ": released semaphore...\n";
                #endif

                return true;
            }
        }
    }
    return false;
}

DWORD WINAPI ReadViaThread(LPVOID lpThreadNum)
{
    DWORD iThreadNum = GetCurrentThreadId();
    for(int i = 0; i < 2; ++i)
        ReadFromMemory(iThreadNum);
    return 0UL;
}