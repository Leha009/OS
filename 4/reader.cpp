#include <iostream>
#include <windows.h>

#define GLOBALWRITESEMAPHORE "globalWriteSemaphore"
#define GLOBALREADSEMAPHORE "globalReadSemaphore"
#define WRITEMUTEXNAME(num) ("writemutex" + std::to_string(num))
#define READMUTEXNAME(num) ("readmutex" + std::to_string(num))
#define PAGE_NUMBER 20
#define PAGE_SIZE 4096
#define FILE_MAP_NAME "lab4bufFile"

#define DEBUG(text) std::cout << text << '\n'

bool InitMutexesAndSemaphore();         // Инициализация мьютексов и семафора читателя 
void CloseMutexesAndSemaphore();        // Закрытие всех мьютексов и семафора читателя

bool WaitForStart();                                                    // Ожидание знака начала работы
bool InitWriterMutexes(HANDLE hWriteGlobalSemaphore);                   // Инициализация мьютексов писателя

HANDLE FileMap();

HANDLE  g_hWriteMutex[PAGE_NUMBER],
        g_hReadMutex[PAGE_NUMBER],
        g_hReadGlobalSemaphore;

int main()
{
    if(InitMutexesAndSemaphore())
    {
        // Создаем проецируемый файл. Если его вдруг уже создали, то получим HANDLE созданного файла
        HANDLE hFileMap = CreateFileMappingA(NULL, NULL, PAGE_READWRITE, 0UL, PAGE_NUMBER*PAGE_SIZE, FILE_MAP_NAME);
        if(hFileMap != NULL)
        {
            // Проецируем в память процесса
            char* lpCharFileView = (char*)MapViewOfFile(hFileMap, FILE_MAP_WRITE, 0UL, 0UL, 0UL);
            if(lpCharFileView != NULL)
            {
                // Блокируем память
                if(VirtualLock(lpCharFileView, PAGE_NUMBER*PAGE_SIZE))
                {
                    DEBUG("Wait for start, get writer semaphore's HANDLE");
                    // Ждем старта, заодно получаем HANDLE семафора писателя
                    HANDLE hWriteGlobalSemaphore = OpenSemaphoreA(SYNCHRONIZE | SEMAPHORE_MODIFY_STATE, true, GLOBALWRITESEMAPHORE);
                    //if(hWriteGlobalSemaphore != INVALID_HANDLE_VALUE)
                    if(hWriteGlobalSemaphore != NULL)
                    {
                        DEBUG("InitWriterMutexes");
                        // Инициализируем мьютексы писателя
                        if(InitWriterMutexes(hWriteGlobalSemaphore))
                        {
                            if(WaitForStart())
                            {
                                std::cout << "Reader reads...\n";
                            }
                            else
                            {
                                std::cout << "Too much time wasted waiting for writer! Reader is going down!\n";
                            }
                        }
                        CloseHandle(hWriteGlobalSemaphore);
                    }
                    else
                    {
                        std::cout << "Writer isn't active, writer is going down!\n";
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

/**
 * @brief Инициализация мьютексов и семафора писателя 
 * 
 * @return true инициализация прошла успешно
 * @return false инициализация провалилась
 */
bool InitMutexesAndSemaphore()
{
    for(int i = 0; i < PAGE_NUMBER; ++i)
    {
        g_hReadMutex[i] = CreateMutexA(NULL, true, READMUTEXNAME(i).c_str());
        if(g_hReadMutex[i] == INVALID_HANDLE_VALUE)
        {
            std::cout << "Failed to create #" << i << " reader's mutex. Error code is " << GetLastError() << '\n';
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
        std::cout << "Failed to create global reader's semaphore. Error code is " << GetLastError() << '\n';
        CloseMutexesAndSemaphore();
        return false;
    }
    return true;
}

/**
 * @brief Закрытие всех мьютексов и семафора читателя
 * 
 */
void CloseMutexesAndSemaphore()
{
    for(int i = 0; i < PAGE_NUMBER; ++i)
    {
        if(g_hWriteMutex[i] != NULL)
            CloseHandle(g_hWriteMutex[i]);
        if(g_hReadMutex[i] != NULL)
            CloseHandle(g_hReadMutex[i]);
    }
    if(g_hReadGlobalSemaphore != NULL)
    {
        CloseHandle(g_hReadGlobalSemaphore);
    }
}

/**
 * @brief Ожидание старта
 * 
 * @return true семафор был использован читателем
 * @return false иначе
 */
bool WaitForStart()
{
    DWORD dwWait = WaitForSingleObject(g_hReadGlobalSemaphore, 10000UL);
    if(dwWait == WAIT_OBJECT_0)
    {
        ReleaseSemaphore(g_hReadGlobalSemaphore, 1, NULL);
        return true;
    }
    else
    {
        return false;
    }
}

/**
 * @brief Инициализация мьютексов писателя
 * 
 * @param hWriteGlobalSemaphore HANDLE семафора писателя
 * 
 * @return true инициализация успешная
 * @return false инициализация провалилась
 */
bool InitWriterMutexes(HANDLE hWriteGlobalSemaphore)
{
    // Получаем мьютексы писателя
    for(int i = 0; i < PAGE_NUMBER; ++i)
    {
        g_hWriteMutex[i] = OpenMutexA(MUTEX_MODIFY_STATE, false, WRITEMUTEXNAME(i).c_str());
        if(g_hWriteMutex[i] == NULL)
        {
            std::cout << "Failed to get writer's mutexes! Error code is " << GetLastError() << "\n";
            for(int j = 0; j < i; ++j)
                CloseHandle(g_hWriteMutex[j]);
            return false;
        }
    }

    // Даем знак писателю, что читатель получил все мьютексы
    ReleaseSemaphore(hWriteGlobalSemaphore, 1, NULL);
    return true;
}