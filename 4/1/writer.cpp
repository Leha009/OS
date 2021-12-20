#include <iostream>
#include <windows.h>
#include <fstream>

#ifdef _UNICODE
#define GLOBALWRITESEMAPHORE L"/globalWriteSemaphore"
#define GLOBALREADSEMAPHORE L"/globalReadSemaphore"
#define MUTEXNAME(num) (L"mutex" + std::to_wstring(num))
#define CHAR_SIZE sizeof(wchar)
#define MAP_FILE_NAME L"mapFileLab4"
#else
#define CHAR_SIZE sizeof(char)
#define GLOBALWRITESEMAPHORE "/globalWriteSemaphore"
#define GLOBALREADSEMAPHORE "/globalReadSemaphore"
#define MUTEXNAME(num) ("mutex" + std::to_string(num))
#define MAP_FILE_NAME "mapFileLab4"
#endif

#define LOG_FILE_NAME "./writer.log"
#define PAGE_NUMBER 20
#define PAGE_SIZE 4096

int main()
{
    std::fstream logStream;
    logStream.open(LOG_FILE_NAME, std::fstream::out | std::fstream::app);
    HANDLE hReadSemaphore = OpenSemaphore(SYNCHRONIZE | SEMAPHORE_MODIFY_STATE, false, GLOBALREADSEMAPHORE);
    HANDLE hWriteSemaphore = OpenSemaphore(SYNCHRONIZE | SEMAPHORE_MODIFY_STATE, false, GLOBALWRITESEMAPHORE);
    if(hReadSemaphore != NULL)
    {
        if(hWriteSemaphore != NULL)
            logStream << std::to_string(GetCurrentProcessId()) + " reader: ready to read!\n";
        else
            logStream << std::to_string(GetCurrentProcessId()) + " reader: cannot get writer semaphore :c Error code is " << GetLastError() << "\n";
    }
    else
    {
        logStream << std::to_string(GetCurrentProcessId()) + " reader: cannot get reader semaphore :c Error code is " << GetLastError() << "\n";
    }
    logStream.close();
    if(hReadSemaphore != NULL)
        CloseHandle(hReadSemaphore);
    if(hWriteSemaphore != NULL)
        CloseHandle(hWriteSemaphore);
    return 0;
}