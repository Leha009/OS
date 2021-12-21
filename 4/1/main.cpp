#include <iostream>
#include <windows.h>

#ifdef _UNICODE
#define GLOBALWRITESEMAPHORE(num) (L"writeSemaphoreLab4" + std::to_wstring(num))
#define GLOBALREADSEMAPHORE(num) (L"readSemaphoreLab4" + std::to_wstring(num))
#define CHAR_SIZE sizeof(WCHAR)
#define MAP_FILE_NAME L"mapFileLab4"
#define WRITER_PROCESS_NAME L"./writer.exe"
#define READER_PROCESS_NAME L"./reader.exe"
#define NEW_PROCESS_ID(num) (WCHAR*)(std::to_wstring(num).c_str())
#else
#define GLOBALWRITESEMAPHORE(num) ("writeSemaphoreLab4" + std::to_string(num))
#define GLOBALREADSEMAPHORE(num) ("readSemaphoreLab4" + std::to_string(num))
#define CHAR_SIZE sizeof(CHAR)
#define MAP_FILE_NAME "mapFileLab4"
#define WRITER_PROCESS_NAME "./writer.exe"
#define READER_PROCESS_NAME "./reader.exe"
#define NEW_PROCESS_ID(num) (CHAR*)(std::to_string(num).c_str())
#endif

#define LOG_READER_MUTEX_NAME "readerMutexLog"
#define LOG_WRITER_MUTEX_NAME "writerMutexLog"
#define PAGE_NUMBER 20
#define PAGE_SIZE 4096

int main(int argc, char* argv[])
{
    int iProcessesNumber = 0;
    if(argc < 2)
    {
        std::cout << "Run the program with number of processes to create!\n";
        return 0;
    }
    else
        iProcessesNumber = std::atoi(argv[1]);

    if(iProcessesNumber < 0)
    {
        std::cout << "Processes number must be more than 0!\n";
        return 0;
    }
    
    HANDLE  hWriteSemaphore[PAGE_NUMBER],
            hReadSemaphore[PAGE_NUMBER],
            hMappedFile;                // Хендл для проецируемого файла (интересный факт: его нет в папке с приложениями)

    HANDLE  hReaderLogMutex,
            hWriterLogMutex;
    
    hReaderLogMutex = CreateMutexA(NULL, false, LOG_READER_MUTEX_NAME);
    if(hReaderLogMutex == NULL)
    {
        std::cout << "Failed to create log mutex for readers!" << std::endl;
        return 0;
    }
    hWriterLogMutex = CreateMutexA(NULL, false, LOG_WRITER_MUTEX_NAME);
    if(hWriterLogMutex == NULL)
    {
        std::cout << "Failed to create log mutex for writers!" << std::endl;
        CloseHandle(hReaderLogMutex);
        return 0;
    }

    bool bInitSuccess = true;
    for(int i = 0; bInitSuccess && i < PAGE_NUMBER; ++i)
    {
        hWriteSemaphore[i] = CreateSemaphore(NULL, 1, 1, GLOBALWRITESEMAPHORE(i).c_str());
        if(hWriteSemaphore[i] != NULL)
        {
            hReadSemaphore[i] = CreateSemaphore(NULL, 0, 1, GLOBALREADSEMAPHORE(i).c_str());
            if(hReadSemaphore[i] == NULL)
            {
                std::cout << "Failed to create " << i << " reader semaphore! Error code is " << GetLastError() << "\n";
                system("pause");
                for(int j = 0; j < i; ++j)
                {
                    CloseHandle(hWriteSemaphore[j]);
                    CloseHandle(hReadSemaphore[j]);
                }
                CloseHandle(hWriteSemaphore[i]);
                bInitSuccess = false;
            }
        }
        else
        {
            std::cout << "Failed to create " << i << " reader semaphore! Error code is " << GetLastError() << "\n";
            system("pause");
            for(int j = 0; j < i; ++j)
            {
                CloseHandle(hWriteSemaphore[j]);
                CloseHandle(hReadSemaphore[j]);
            }
            bInitSuccess = false;
        }
    }
    if(bInitSuccess)
    {
        // Создаем проецируемый файл. Если его вдруг уже создали, то получим HANDLE созданного файла
        hMappedFile = CreateFileMapping(NULL, NULL, PAGE_READWRITE, 0UL, PAGE_NUMBER*PAGE_SIZE*CHAR_SIZE, MAP_FILE_NAME);
        if(hMappedFile == NULL)
        {
            std::cout << "Failed to create file for mapping! Error code is " << GetLastError() << "\n";
            system("pause");
        }
        else
        {
            // Отображаем проецируемый файл в память процесса
            char* lpCharFileView = (char*)MapViewOfFile(hMappedFile, FILE_MAP_WRITE, 0UL, 0UL, 0UL);
            if(lpCharFileView != NULL)
            {
                // Зачищаем все, чтобы ничего не находилось в памяти по адресу отображения
                ZeroMemory(lpCharFileView, PAGE_NUMBER*PAGE_SIZE*CHAR_SIZE);
                // Создаем хендлы для процессов и структуры для создания этих процессов
                HANDLE* hProcesses = new HANDLE[iProcessesNumber];
                PROCESS_INFORMATION* writerProcessInfo = new PROCESS_INFORMATION[iProcessesNumber];
                PROCESS_INFORMATION* readerProcessInfo = new PROCESS_INFORMATION[iProcessesNumber];
                if(hProcesses != NULL && writerProcessInfo != NULL && readerProcessInfo != NULL)
                {
                    // Создаем структуру с изначальной информацией и чистим ее
                    #ifdef _UNICODE
                    STARTUPINFOW startInfo;
                    ZeroMemory(&startInfo, sizeof(STARTUPINFOW));
                    #else
                    STARTUPINFOA startInfo;
                    ZeroMemory(&startInfo, sizeof(STARTUPINFOA));
                    #endif
                    startInfo.cb = sizeof(startInfo);   // Так всегда надо указывать, ничего не поделать
                    for(int i = 0; i < iProcessesNumber; ++i)
                    {
                        // Зачищаем структуры с информацией о процессе
                        ZeroMemory(&writerProcessInfo[i], sizeof(PROCESS_INFORMATION));
                        ZeroMemory(&readerProcessInfo[i], sizeof(PROCESS_INFORMATION));
                        // Создаем писателя
                        if(!CreateProcess(WRITER_PROCESS_NAME, NEW_PROCESS_ID(i), NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &startInfo, &writerProcessInfo[i]))
                        {
                            std::cout << "Failed to create writer process Error code is " << GetLastError() << "\n";
                            for(int j = 0; j < i; ++j)
                            {
                                if(!TerminateProcess(writerProcessInfo[j].hProcess, 0))
                                {
                                    std::cout << "Failed to terminate writer process Error code is " << GetLastError() << "\n";
                                }
                                if(!TerminateProcess(readerProcessInfo[j].hProcess, 0))
                                {
                                    std::cout << "Failed to terminate reader process Error code is " << GetLastError() << "\n";
                                }
                            }
                            system("pause");
                            break;
                        }
                        // Если все хорошо, то создаем читателя
                        else if(!CreateProcess(READER_PROCESS_NAME, NEW_PROCESS_ID(i), NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &startInfo, &readerProcessInfo[i]))
                        {
                            std::cout << "Failed to create reader process Error code is " << GetLastError() << "\n";
                            for(int j = 0; j < i; ++j)
                            {
                                if(!TerminateProcess(writerProcessInfo[j].hProcess, 0))
                                {
                                    std::cout << "Failed to terminate writer process Error code is " << GetLastError() << "\n";
                                }
                                if(!TerminateProcess(readerProcessInfo[j].hProcess, 0))
                                {
                                    std::cout << "Failed to terminate reader process Error code is " << GetLastError() << "\n";
                                }
                            }
                            system("pause");
                            break;
                        }
                        Sleep(100);     // Это можно и убрать, но так более вероятно по порядку запустятся процессы (айди последовательные будут)
                    }
                    /*
                    // Ждем все процессы
                    for(int i = 0; i < iProcessesNumber; ++i)
                    {
                        WaitForSingleObject(writerProcessInfo[i].hProcess, 3000);
                        WaitForSingleObject(readerProcessInfo[i].hProcess, 3000);
                    }*/
                    std::cout << "Everything's ok, press any key if you want to finish write/read processes\n";
                    system("pause");
                    delete [] hProcesses;
                    delete [] writerProcessInfo;
                    delete [] readerProcessInfo;
                }
                else
                {
                    std::cout << "Failed to allocate memory for the processes' handles or for the processes info!\n";
                    system("pause");
                    if(hProcesses != NULL)
                        delete [] hProcesses;
                    if(writerProcessInfo != NULL)
                        delete [] writerProcessInfo;
                    if(readerProcessInfo != NULL)
                        delete [] readerProcessInfo;
                }
                UnmapViewOfFile(lpCharFileView);
            }
            else
            {
                std::cout << "Failed to create map view of file! Error code is " << GetLastError() << "\n";
                system("pause");
            }
            CloseHandle(hMappedFile);
        }
        for(int i = 0; i < PAGE_NUMBER; ++i)
        {
            CloseHandle(hReadSemaphore[i]);
            CloseHandle(hWriteSemaphore[i]);
        }
    }
    return 0;
}