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

bool InitMutexesAndSemaphore();         // Инициализация мьютексов и семафора писателя 
void CloseMutexesAndSemaphore();        // Закрытие всех мьютексов и семафора писателя

bool WaitForStart();                    // Ожидание старта
bool InitReadMutexes();                 // Инициализация мьютексов читателя

HANDLE FileMap();

HANDLE  g_hWriteMutex[PAGE_NUMBER],
        g_hReadMutex[PAGE_NUMBER],
        g_hWriteGlobalSemaphore;

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
                    DEBUG("WaitForStart");
                    // Ждем старта
                    if(WaitForStart())
                    {
                        DEBUG("InitReadMutexes");
                        // Инициализируем мьютексы читателя
                        if(InitReadMutexes())
                        {
                            std::cout << "Writer writes...\n";
                        }
                    }
                    else
                    {
                        std::cout << "Reader isn't active, writer is going down!\n";
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
        std::cout << "Due to failure during mutex creation writer is going down!\n";
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
        g_hWriteMutex[i] = CreateMutexA(NULL, true, WRITEMUTEXNAME(i).c_str());
        if(g_hWriteMutex[i] == INVALID_HANDLE_VALUE)
        {
            std::cout << "Failed to create #" << i << " writer's mutex. Error code is " << GetLastError() << '\n';
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
        std::cout << "Failed to create global writer's semaphore. Error code is " << GetLastError() << '\n';
        CloseMutexesAndSemaphore();
        return false;
    }
    return true;
}

/**
 * @brief Закрытие всех мьютексов и семафора писателя
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
    if(g_hWriteGlobalSemaphore != NULL)
    {
        CloseHandle(g_hWriteGlobalSemaphore);
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
    DWORD dwWait = WaitForSingleObject(g_hWriteGlobalSemaphore, 10000UL);
    if(dwWait == WAIT_OBJECT_0)
    {
        ReleaseSemaphore(g_hWriteGlobalSemaphore, 1, NULL);
        return true;
    }
    else
    {
        return false;
    }
}

/**
 * @brief Инициализация мьютексов читателя
 * 
 * @return true инициализация успешная
 * @return false инициализация провалилась
 */
/*bool InitReadMutexes()
{
    // Получаем HANDLE семафора читателя
    HANDLE hReadGlobalSemaphore = OpenSemaphoreA(SYNCHRONIZE | SEMAPHORE_MODIFY_STATE, true, GLOBALREADSEMAPHORE);
    if(hReadGlobalSemaphore != NULL)
    {
        bool bContinue = true;
        // Занимаем семафор читателя (чтобы он не начинал читать, пока мы не будем готовы)
        DWORD dwWait = WaitForSingleObject(hReadGlobalSemaphore, 2000UL);
        if(dwWait == WAIT_OBJECT_0)
        {
            // Получаем мьютексы читателя
            for(int i = 0; i < PAGE_NUMBER; ++i)
            {
                g_hReadMutex[i] = OpenMutexA(MUTEX_MODIFY_STATE, false, READMUTEXNAME(i).c_str());
                if(g_hReadMutex[i] == NULL)
                {
                    std::cout << "Failed to get reader's mutexes! Error code is " << GetLastError() << "\n";
                    for(int j = 0; j < i; ++j)
                        CloseHandle(g_hReadMutex[j]);
                    return false;
                }
            }

            // Даем знак читателю, что писатель готов писать
            ReleaseSemaphore(hReadGlobalSemaphore, 1, NULL);
            // Ждем знака готовности от читателя
            dwWait = WaitForSingleObject(g_hWriteGlobalSemaphore, 2000UL);
            if(dwWait == WAIT_OBJECT_0)
            {
                ReleaseSemaphore(g_hWriteGlobalSemaphore, 1, NULL);
                return true;
            }
        }
        else if(dwWait == WAIT_FAILED)
        {
            std::cout << "Waiting for writer's mutexes failed! Error code is " << GetLastError() << "\n";
        }
        else
        {
            std::cout << "Too much time wasted waiting for reader's mutexes!\n";
        }
    }
    else
    {
        std::cout << "Failed to get global reader's semaphore! Error code is " << GetLastError() << "\n";
    }
    return false;
}
*/
bool InitReadMutexes()
{
    // Получаем HANDLE семафора читателя
    HANDLE hReadGlobalSemaphore = OpenSemaphoreA(SYNCHRONIZE | SEMAPHORE_MODIFY_STATE, true, GLOBALREADSEMAPHORE);
    if(hReadGlobalSemaphore != NULL)
    {
        // Получаем мьютексы читателя
        for(int i = 0; i < PAGE_NUMBER; ++i)
        {
            g_hReadMutex[i] = OpenMutexA(MUTEX_MODIFY_STATE, false, READMUTEXNAME(i).c_str());
            if(g_hReadMutex[i] == NULL)
            {
                std::cout << "Failed to get reader's mutexes! Error code is " << GetLastError() << "\n";
                for(int j = 0; j < i; ++j)
                    CloseHandle(g_hReadMutex[j]);
                return false;
            }
        }

        // Даем знак читателю, что писатель готов писать
        ReleaseSemaphore(hReadGlobalSemaphore, 1, NULL);
        // Ждем знака готовности от читателя
        DWORD dwWait = WaitForSingleObject(g_hWriteGlobalSemaphore, 2000UL);
        if(dwWait == WAIT_OBJECT_0)
        {
            ReleaseSemaphore(g_hWriteGlobalSemaphore, 1, NULL);
            return true;
        }
    }
    else
    {
        std::cout << "Failed to get global reader's semaphore! Error code is " << GetLastError() << "\n";
    }
    return false;
}