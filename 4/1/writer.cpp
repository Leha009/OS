#include <iostream>
#include <windows.h>
#include <fstream>
#include <ctime>

#ifdef _UNICODE
#define GLOBALWRITESEMAPHORE(num) (L"writeSemaphoreLab4" + std::to_wstring(num))
#define GLOBALREADSEMAPHORE(num) (L"readSemaphoreLab4" + std::to_wstring(num))
#define CHAR_SIZE sizeof(WCHAR)
#define MAP_FILE_NAME L"mapFileLab4"
#else
#define GLOBALWRITESEMAPHORE(num) ("writeSemaphoreLab4" + std::to_string(num))
#define GLOBALREADSEMAPHORE(num) ("readSemaphoreLab4" + std::to_string(num))
#define CHAR_SIZE sizeof(CHAR)
#define MAP_FILE_NAME "mapFileLab4"
#endif

#define SLEEP_TIME 500 + rand() % 1001      // Ждем от 500 до 1500 мс
#define LOG_MUTEX_NAME "writerMutexLog"
#define sID argv[0]
#define iRepeats std::atoi(argv[1])
//#define LOG_FILE_NAME (std::string("./readers/reader") + sID << ".log").c_str()
#define LOG_FILE_NAME "writer.log"
#define PAGE_NUMBER 20
#define PAGE_SIZE 4096

void StartWrite_(
    HANDLE hReadSemaphore[PAGE_NUMBER], 
    HANDLE hWriteSemaphore[PAGE_NUMBER], 
    HANDLE hLogMutex, 
    std::fstream& logStream, 
    LPCSTR ID,
    char* lpCharFileView);

int main(int argc, char* argv[])
{
    if(argc < 2)
    {
        std::cout << "Run the program with number of id and number of repeats!\n";
        return 1;
    }
    srand(time(NULL));

    HANDLE  hReadSemaphore[PAGE_NUMBER],
            hWriteSemaphore[PAGE_NUMBER];

    std::fstream logStream;
    logStream.open(LOG_FILE_NAME, std::fstream::out | std::fstream::app);
    bool bOpenSuccess = true;
    for(int i = 0; bOpenSuccess && i < PAGE_NUMBER; ++i)
    {
        hReadSemaphore[i] = OpenSemaphore(SYNCHRONIZE | SEMAPHORE_MODIFY_STATE, false, GLOBALREADSEMAPHORE(i).c_str());
        if(hReadSemaphore != NULL)
        {
            hWriteSemaphore[i] = OpenSemaphore(SYNCHRONIZE | SEMAPHORE_MODIFY_STATE, false, GLOBALWRITESEMAPHORE(i).c_str());
            if(hWriteSemaphore[i] == NULL)
            {
                logStream << GetTickCount() << " | " << sID << " writer: failed to open writer semaphore #" << i << "! Error code is " << GetLastError() << std::endl;
                for(int j = 0; j < i; ++j)
                {
                    CloseHandle(hReadSemaphore[j]);
                    CloseHandle(hWriteSemaphore[j]);
                }
                CloseHandle(hReadSemaphore[i]);
                bOpenSuccess = false;
            }
        }
        else
        {
            logStream << GetTickCount() << " | " << sID << " writer: failed to open reader semaphore #" << i << "! Error code is " << GetLastError() << std::endl;
            for(int j = 0; j < i; ++j)
            {
                CloseHandle(hReadSemaphore[j]);
                CloseHandle(hWriteSemaphore[j]);
            }
            bOpenSuccess = false;
        }
    }
    if(bOpenSuccess)
    {
        HANDLE hLogMutex = OpenMutexA(SYNCHRONIZE | MUTEX_MODIFY_STATE, false, LOG_MUTEX_NAME);
        if(hLogMutex != NULL)
        {
            HANDLE hFileMap = OpenFileMapping(FILE_MAP_WRITE, false, MAP_FILE_NAME);
            if(hFileMap != NULL)
            {
                // Проецируем в память процесса
                char* lpCharFileView = (char*)MapViewOfFile(hFileMap, FILE_MAP_WRITE, 0UL, 0UL, 0UL);
                if(lpCharFileView != NULL)
                {
                    //if(VirtualLock(lpCharFileView, PAGE_NUMBER*PAGE_SIZE*CHAR_SIZE))
                    if(true)
                    {
                        WaitForSingleObject(hLogMutex, INFINITE);
                        logStream << GetTickCount() << " | " << sID << " writer: ready to write!" << std::endl;
                        logStream.flush();
                        ReleaseMutex(hLogMutex);

                        for(int i = 0; i < iRepeats; ++i)
                        {
                            StartWrite_(hReadSemaphore, hWriteSemaphore, hLogMutex, logStream, sID, lpCharFileView);
                        }

                        //VirtualUnlock(lpCharFileView, PAGE_NUMBER*PAGE_SIZE*CHAR_SIZE);
                    }
                    else
                    {
                        WaitForSingleObject(hLogMutex, INFINITE);
                        logStream << GetTickCount() << " | " << GetTickCount() << " | " << sID << " writer: failed to virtual lock! Error code is " << GetLastError() << std::endl;
                        logStream.flush();
                        ReleaseMutex(hLogMutex);
                    }

                    UnmapViewOfFile(lpCharFileView);
                }
                else
                {
                    WaitForSingleObject(hLogMutex, INFINITE);
                    logStream << GetTickCount() << " | " << GetTickCount() << " | " << sID << " writer: failed to get view of file! Error code is " << GetLastError() << std::endl;
                    logStream.flush();
                    ReleaseMutex(hLogMutex);
                }

                CloseHandle(hFileMap);
            }
            else
            {
                WaitForSingleObject(hLogMutex, INFINITE);
                logStream << GetTickCount() << " | " << GetTickCount() << " | " << sID << " writer: failed to get file mapping! Error code is " << GetLastError() << std::endl;
                logStream.flush();
                ReleaseMutex(hLogMutex);
            }

            CloseHandle(hLogMutex);
        }
        else
        {
            logStream << GetTickCount() << " | " << GetTickCount() << " | " << sID << " writer: failed to get log mutex! Error code is " << GetLastError() << std::endl;
            logStream.flush();
        }
    }
    Sleep(100);
    logStream << GetTickCount() << " | " << sID << " writer: going down..." << std::endl;
    logStream.flush();
    logStream.close();
    Sleep(10);
    if(hReadSemaphore != NULL)
        CloseHandle(hReadSemaphore);
    if(hWriteSemaphore != NULL)
        CloseHandle(hWriteSemaphore);
    return 0;
}

void StartWrite_(
    HANDLE hReadSemaphore[PAGE_NUMBER], 
    HANDLE hWriteSemaphore[PAGE_NUMBER], 
    HANDLE hLogMutex, 
    std::fstream& logStream, 
    LPCSTR ID,
    char* lpCharFileView)
{
    WaitForSingleObject(hLogMutex, INFINITE);
    logStream << GetTickCount() << " | " << ID << " writer: waiting for writer's semaphore" << std::endl;
    //logStream.flush();
    ReleaseMutex(hLogMutex);

    DWORD dwPageToRead = WaitForMultipleObjects(PAGE_NUMBER, hWriteSemaphore, false, INFINITE);
    WaitForSingleObject(hLogMutex, INFINITE);
    logStream << GetTickCount() << " | " << ID << " writer: writing page number to page #" << dwPageToRead << std::endl;
    //logStream.flush();
    ReleaseMutex(hLogMutex);
    std::string sPage = std::to_string(dwPageToRead);
    CopyMemory(lpCharFileView+(PAGE_SIZE*dwPageToRead), sPage.c_str(), sPage.length());
    Sleep(SLEEP_TIME);

    WaitForSingleObject(hLogMutex, INFINITE);
    logStream << GetTickCount() << " | " << ID << " writer: release reader's semaphore #" << dwPageToRead << std::endl;
    //logStream.flush();
    ReleaseMutex(hLogMutex);
    ReleaseSemaphore(hReadSemaphore[dwPageToRead], 1, NULL);
}