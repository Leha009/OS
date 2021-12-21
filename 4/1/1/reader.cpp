#include <iostream>
#include <windows.h>
#include <fstream>
#include <ctime>

#ifdef _UNICODE
#define GLOBALWRITESEMAPHORE L"writeSemaphoreLab4"
#define GLOBALREADSEMAPHORE L"readSemaphoreLab4"
#define MUTEXNAME(num) (L"mutex" + std::to_wstring(num))
#define CHAR_SIZE sizeof(wchar)
#define MAP_FILE_NAME L"mapFileLab4"
#else
#define CHAR_SIZE sizeof(char)
#define GLOBALWRITESEMAPHORE "writeSemaphoreLab4"
#define GLOBALREADSEMAPHORE "readSemaphoreLab4"
#define MUTEXNAME(num) ("mutex" + std::to_string(num))
#define MAP_FILE_NAME "mapFileLab4"
#endif

#define SLEEP_TIME 500 + rand() % 1001      // Ждем от 500 до 1500 мс
#define LOG_MUTEX_NAME "readerMutexLog"
#define sID argv[0]
//#define LOG_FILE_NAME (std::string("./readers/reader") + sID << ".log").c_str()
#define LOG_FILE_NAME "reader.log"
#define PAGE_NUMBER 20
#define PAGE_SIZE 4096

void StartRead_(
    HANDLE hReadSemaphore, 
    HANDLE hWriteSemaphore, 
    HANDLE hMutex[PAGE_NUMBER], 
    HANDLE hLogMutex, 
    std::fstream& logStream, 
    LPCSTR ID);

int main(int argc, char* argv[])
{
    if(argc < 1)
    {
        std::cout << "Run the program with number of id!\n";
        return 1;
    }
    srand(time(NULL));

    std::fstream logStream;
    logStream.open(LOG_FILE_NAME, std::fstream::out | std::fstream::app);
    HANDLE hReadSemaphore = OpenSemaphore(SYNCHRONIZE | SEMAPHORE_MODIFY_STATE, false, GLOBALREADSEMAPHORE);
    HANDLE hWriteSemaphore = OpenSemaphore(SYNCHRONIZE | SEMAPHORE_MODIFY_STATE, false, GLOBALWRITESEMAPHORE);
    if(hReadSemaphore != NULL)
    {
        if(hWriteSemaphore != NULL)
        {
            HANDLE hLogMutex = OpenMutexA(SYNCHRONIZE | MUTEX_MODIFY_STATE, false, LOG_MUTEX_NAME);
            if(hLogMutex != NULL)
            {
                HANDLE hMutex[PAGE_NUMBER];
                bool bSuccess = true;
                for(int i = 0; bSuccess && i < PAGE_NUMBER; ++i)
                {
                    hMutex[i] = OpenMutex(SYNCHRONIZE | MUTEX_MODIFY_STATE, false, MUTEXNAME(i).c_str());
                    if(hMutex[i] == NULL)
                    {
                        for(int j = 0; j < i; ++j)
                            CloseHandle(hMutex[j]);
                        bSuccess = false;
                        logStream << GetTickCount() << " | " << sID << " reader: failed to open mutex #" << i << "! Error code is " << GetLastError() << std::endl;
                    }
                }
                if(bSuccess)
                {
                    WaitForSingleObject(hLogMutex, INFINITE);
                    logStream << GetTickCount() << " | " << sID << " reader: ready to read!" << std::endl;
                    logStream.flush();
                    ReleaseMutex(hLogMutex);

                    StartRead_(hReadSemaphore, hWriteSemaphore, hMutex, hLogMutex, logStream, sID);
                }

                CloseHandle(hLogMutex);
            }
            else
            {
                logStream << GetTickCount() << " | " << GetTickCount() << " | " << sID << " reader: failed to get log mutex! Error code is " << GetLastError() << std::endl;
            }
        }
        else
        {
            logStream << GetTickCount() << " | " << sID << " reader: cannot get writer semaphore :c Error code is " << GetLastError() << std::endl;
        }
    }
    else
    {
        logStream << GetTickCount() << " | " << sID << " reader: cannot get reader semaphore :c Error code is " << GetLastError() << std::endl;
    }
    logStream << GetTickCount() << " | " << sID << " reader: going down..." << std::endl;
    logStream.close();
    Sleep(10);
    if(hReadSemaphore != NULL)
        CloseHandle(hReadSemaphore);
    if(hWriteSemaphore != NULL)
        CloseHandle(hWriteSemaphore);
    return 0;
}

void StartRead_(
    HANDLE hReadSemaphore, 
    HANDLE hWriteSemaphore, 
    HANDLE hMutex[PAGE_NUMBER], 
    HANDLE hLogMutex, 
    std::fstream& logStream, 
    LPCSTR ID)
{
    WaitForSingleObject(hReadSemaphore, INFINITE);
    DWORD dwPageToRead = WaitForMultipleObjects(PAGE_NUMBER, hMutex, false, INFINITE);
    WaitForSingleObject(hLogMutex, INFINITE);
    logStream << GetTickCount() << " | " << ID << " reader: reading page #" << dwPageToRead << std::endl;
    logStream.flush();
    ReleaseMutex(hLogMutex);
    Sleep(SLEEP_TIME);
    ReleaseMutex(hMutex[dwPageToRead]);
    ReleaseSemaphore(hWriteSemaphore, 1, NULL);
}