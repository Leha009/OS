#include <iostream>
#include <windows.h>
#include <mutex>
#include <string>
#include <ctime>

#define _LogEnable 1
#ifdef _LogEnable
#define _LogToFile 1
#endif

#define MAPPED_FILE_NAME "bufferFile"
#define BUF_PAGES 16UL                      // 30823 => 3 + 0 + 8 + 2 + 3 = 16
#define PAGE_SIZE 4096                      // Используется, если реально что-то пишется в память
#define SLEEP_TIME 500 + rand() % 1001      // Ждем от 500 до 1500 мс
#define THREAD_NUM 16                       // Сколько писателей/читателей
#define READ_WRITE_REPEATS 10               // Сколько раз прогнать написание/чтение

// Запись в файлик какой-то строки
bool LogToFile(HANDLE hFile, LPCSTR lpStringToWrite, DWORD dwStringLength);

// Функции ниже пишут что-то
bool WriteToMemory(DWORD iThreadNum, LPVOID lpAddress);
DWORD WINAPI WriteViaThread(LPVOID lpAddress);

// Функции ниже читают что-то
bool ReadFromMemory(DWORD iThreadNum, LPVOID lpAddress);
DWORD WINAPI ReadViaThread(LPVOID lpAddress);

// Каждые 100 мс получает кол-во занятых страниц (сколько читают, сколько пишут) для матлаба
DWORD WINAPI WriteBusinessOfPages(LPVOID lp);

HANDLE  g_hWriteSemaphore[BUF_PAGES],       // Семафоры писателей
        g_hReadSemaphore[BUF_PAGES],        // Семафоры читателей
        g_hLogFile;                         // Файл для лога состояний

char g_cSymbolToPage[BUF_PAGES];            // Символы, чтобы реально что-то писать в страницы

int     g_iWriters,                         // Счетчик писателей
        g_iReaders;                         // Счетчик читателей

int main(int argc, char* argv[])
{
    srand(time(NULL));
    HANDLE  hMappedFile,                    // Проецируемый файл (общая память)
            hWritersThreads[THREAD_NUM],    // Потоки писателей
            hReadersThreads[THREAD_NUM];    // Потоки читателей
    DWORD   dwPageSize;                     // Реальный размер страницы в системе
    SYSTEM_INFO sSysInfo;                   // Информация о системе
    GetSystemInfo(&sSysInfo);
    dwPageSize = sSysInfo.dwPageSize;
    #ifdef _LogToFile
    // Открываем файл для лога состояния
    g_hLogFile = CreateFileA("actions.log", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if(g_hLogFile == INVALID_HANDLE_VALUE)
    {
        std::cout << "Failed to create log file! Error code is " << GetLastError() << '\n';
        system("pause");
        return 1;
    }
    #endif

    g_iWriters = g_iReaders = 0;

    hMappedFile = CreateFileMappingA(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0UL, BUF_PAGES*dwPageSize, MAPPED_FILE_NAME);
    if(hMappedFile != INVALID_HANDLE_VALUE)
    {
        LPVOID lpFileView = MapViewOfFile(hMappedFile, FILE_MAP_WRITE, 0UL, 0UL, BUF_PAGES*dwPageSize);
        if(lpFileView != NULL)
        {
            if(VirtualLock(lpFileView, BUF_PAGES*dwPageSize))
            {
                bool bSuccess = true;
                // Ниже создаем семафоры и смотрим, чтобы все было хорошо
                for(int i = 0; bSuccess && i < BUF_PAGES; ++i)
                {
                    g_hWriteSemaphore[i] = CreateSemaphoreA(NULL, 1, 1, NULL);
                    if(g_hWriteSemaphore[i] == INVALID_HANDLE_VALUE)    // Если что-то пошло не так, то убираем за собой
                    {
                        for(int j = 0; j < i; ++j)
                        {
                            CloseHandle(g_hWriteSemaphore[i]);
                            if(j < i-1)
                                CloseHandle(g_hReadSemaphore[i]);
                        }
                        bSuccess = false;
                        std::cout << "Failed to create write semaphore! Error code is " << GetLastError() << '\n';
                    }
                    else
                    {
                        g_hReadSemaphore[i] = CreateSemaphoreA(NULL, 0, 1, NULL);
                        if(g_hReadSemaphore[i] == INVALID_HANDLE_VALUE) // Если что-то пошло не так, то убираем за собой
                        {
                            for(int j = 0; j < i; ++j)
                            {
                                CloseHandle(g_hWriteSemaphore[i]);
                                CloseHandle(g_hReadSemaphore[i]);
                            }
                            bSuccess = false;
                            std::cout << "Failed to create read semaphore! Error code is " << GetLastError() << '\n';
                        }
                    }
                }
                if(bSuccess)
                {
                    // Если использовать реальную запись в файл, чтобы посмотреть работоспособность
                    for(int i = 0; i < BUF_PAGES; ++i)
                    {
                        g_cSymbolToPage[i] = (char)(65+i);
                    }
                    for(int i = 0; bSuccess && i < THREAD_NUM; ++i)
                    {
                        hWritersThreads[i] = CreateThread(NULL, 0, WriteViaThread, lpFileView, 0, NULL);
                        if(hWritersThreads[i] == INVALID_HANDLE_VALUE)  // Если что-то пошло не так, то убираем за собой
                        {
                            bSuccess = false;
                            for(int j = 0; j < i; ++j)
                            {
                                CloseHandle(hWritersThreads[i]);
                                if(j < i-1)
                                    CloseHandle(hReadersThreads[i]);
                            }
                            std::cout << "Failed to create write thread! Error code is " << GetLastError() << '\n';
                        }
                        else
                        {
                            hReadersThreads[i] = CreateThread(NULL, 0, ReadViaThread, lpFileView, 0, NULL);
                            if(hReadersThreads[i] == INVALID_HANDLE_VALUE)  // Если что-то пошло не так, то убираем за собой
                            {
                                for(int j = 0; j < i; ++j)
                                {
                                    CloseHandle(hWritersThreads[i]);
                                    CloseHandle(hReadersThreads[i]);
                                }
                                bSuccess = false;
                                std::cout << "Failed to create read thread! Error code is " << GetLastError() << '\n';
                            }
                        }
                    }
                    if(bSuccess)    // Все хорошо, ждем конца работы писателей-читателей, параллельно логируя состояния страниц (потом и потоков, они же писатели/читатели)
                    {
                        HANDLE hBusinessLog = CreateThread(NULL, 0, WriteBusinessOfPages, NULL, 0, NULL);
                        WaitForMultipleObjects(THREAD_NUM, hWritersThreads, true, INFINITE);
                        WaitForMultipleObjects(THREAD_NUM, hReadersThreads, true, INFINITE);
                        if(hBusinessLog != INVALID_HANDLE_VALUE)
                        {
                            WaitForSingleObject(hBusinessLog, INFINITE);
                            CloseHandle(hBusinessLog);
                        }
                        for(int i = 0; i < THREAD_NUM; ++i) // Не забываем закрыть дескрипторы, которые наоткрывали
                        {
                            CloseHandle(hWritersThreads[i]);
                            CloseHandle(hReadersThreads[i]);
                        }
                    }

                    for(int i = 0; i < BUF_PAGES; ++i)  // Не забываем закрыть дескрипторы, которые наоткрывали
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

/*
 * Запись в файлик какой-то строки
 */
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


/*
 * Запись чего-нибудь в страницу (fake or real)
 */
bool WriteToMemory(DWORD iThreadNum, LPVOID lpAddress)
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
                ++g_iWriters;
                #ifdef _LogToFile
                //Fake write
                sLogAction = std::to_string(GetTickCount()) + " | Writing Thread #" + std::to_string(iThreadNum) + ": writing something to page #" + std::to_string(i) + "...\n";
                //Real Write
                //sLogAction = std::to_string(GetTickCount()) + " | Writing Thread #" + std::to_string(iThreadNum) + ": writing " + (char)(g_cSymbolToPage[i]) + " to page " + std::to_string(i) + "...\n";
                LogToFile(g_hLogFile, sLogAction.c_str(), sLogAction.length());
                #elif _LogEnable
                std::cout << "Writing Thread #" << iThreadNum << ": writing something... to page #" << i << "...\n";
                #endif

                //Fake write
                Sleep(SLEEP_TIME);
                //Real write
                //CopyMemory((LPVOID)((char*)lpAddress+i*PAGE_SIZE), (LPVOID)(&g_cSymbolToPage[i]), 1);

                #ifdef _LogToFile
                sLogAction = std::to_string(GetTickCount()) + " | Writing Thread #" + std::to_string(iThreadNum) + ": releasing semaphore...\n";
                LogToFile(g_hLogFile, sLogAction.c_str(), sLogAction.length());
                #elif _LogEnable
                std::cout << "Writing Thread #" << iThreadNum << ": releasing semaphore...\n";
                #endif

                --g_iWriters;
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

/*
 * Поток-писатель
 */
DWORD WINAPI WriteViaThread(LPVOID lpAddress)
{
    DWORD iThreadNum = GetCurrentThreadId();
    for(int i = 0; i < READ_WRITE_REPEATS; ++i)
        WriteToMemory(iThreadNum, lpAddress);
    return 0UL;
}

/*
 * Чтение собственной персоной
 */
bool ReadFromMemory(DWORD iThreadNum, LPVOID lpAddress)
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
                ++g_iReaders;
                #ifdef _LogToFile
                //Fake read
                sLogAction = std::to_string(GetTickCount()) + " | Reading Thread #" + std::to_string(iThreadNum) + ": reading something from page #" + std::to_string(i) + "...\n";
                //Real read
                //sLogAction = std::to_string(GetTickCount()) + " | Reading Thread #" + std::to_string(iThreadNum) + ": read " + ((char*)lpAddress+i*PAGE_SIZE) + " from page #" + std::to_string(i) + "...\n";
                LogToFile(g_hLogFile, sLogAction.c_str(), sLogAction.length());
                #elif _LogEnable
                std::cout << "Reading Thread #" << iThreadNum << ": reading something from page #" << i << "...\n";
                #endif

                //Fake read. Если закомментировать и использовать реальное чтение символа, то это происходит довольно быстро, и все действия происходят на одной странице
                Sleep(SLEEP_TIME);

                #ifdef _LogToFile
                sLogAction = std::to_string(GetTickCount()) + " | Reading Thread #" + std::to_string(iThreadNum) + ": releasing semaphore...\n";
                LogToFile(g_hLogFile, sLogAction.c_str(), sLogAction.length());
                #elif _LogEnable
                std::cout << "Reading Thread #" << iThreadNum << ": releasing semaphore...\n";
                #endif

                --g_iReaders;
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

/*
 * Поток-читатель
 */
DWORD WINAPI ReadViaThread(LPVOID lpAddress)
{
    DWORD iThreadNum = GetCurrentThreadId();
    for(int i = 0; i < READ_WRITE_REPEATS; ++i)
        ReadFromMemory(iThreadNum, lpAddress);
    return 0UL;
}

/* 
 * Каждые 100 мс получает кол-во занятых страниц (сколько читают, сколько пишут) для матлаба
 */
DWORD WINAPI WriteBusinessOfPages(LPVOID lp)
{
    HANDLE hFile = CreateFileA("businessOfPages.txt", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if(hFile == INVALID_HANDLE_VALUE)
        return 0UL;
    std::string sWriters = "writingPages = [";
    std::string sReaders = "readingPages = [";
    std::string sTime = "time = [";
    int iTimes = 0;
    while(true)
    {
        sWriters += std::to_string(g_iWriters) + " ";
        sReaders += std::to_string(g_iReaders) + " ";
        sTime += std::to_string(iTimes*100) + " ";
        ++iTimes;
        if(g_iWriters == 0 && g_iReaders == 0)
            break;
        Sleep(100);
    }
    sWriters += "];\n";
    sReaders += "];\n";
    sTime += "];\n";
    WriteFile(hFile, sWriters.c_str(), sWriters.length(), NULL, NULL);
    WriteFile(hFile, sReaders.c_str(), sReaders.length(), NULL, NULL);
    WriteFile(hFile, sTime.c_str(), sTime.length(), NULL, NULL);
    CloseHandle(hFile);
    return 0UL;
}