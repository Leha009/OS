#include <iostream>
#include <windows.h>

#ifdef _UNICODE
#define GLOBALWRITESEMAPHORE L"/globalWriteSemaphore"
#define GLOBALREADSEMAPHORE L"/globalReadSemaphore"
#define MUTEXNAME(num) (L"mutex" + std::to_wstring(num))
#define CHAR_SIZE sizeof(wchar)
#define MAP_FILE_NAME L"mapFileLab4"
#define WRITER_PROCESS_NAME L"./writer.exe"
#define READER_PROCESS_NAME L"./reader.exe"
#else
#define CHAR_SIZE sizeof(char)
#define GLOBALWRITESEMAPHORE "/globalWriteSemaphore"
#define GLOBALREADSEMAPHORE "/globalReadSemaphore"
#define MUTEXNAME(num) ("mutex" + std::to_string(num))
#define MAP_FILE_NAME "mapFileLab4"
#define WRITER_PROCESS_NAME "./writer.exe"
#define READER_PROCESS_NAME "./reader.exe"
#endif

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
    
    HANDLE  hWriteSemaphore,
            hReadSemaphore,
            hMutex[PAGE_NUMBER],
            hMappedFile;                // Хендл для проецируемого файла (интересный факт: его нет в папке с приложениями)

    hWriteSemaphore = CreateSemaphore(NULL, iProcessesNumber, iProcessesNumber, GLOBALWRITESEMAPHORE);
    hReadSemaphore = CreateSemaphore(NULL, 0, iProcessesNumber, GLOBALREADSEMAPHORE);
    if(hWriteSemaphore == NULL || hReadSemaphore == NULL)
    {
        if(hWriteSemaphore != NULL)
            CloseHandle(hWriteSemaphore);
        if(hReadSemaphore != NULL)
            CloseHandle(hReadSemaphore);
        std::cout << "Failed to create writers' or readers' semaphore! Error code is " << GetLastError() << "\n";
    }
    else
    {
        bool bInitSuccess = true;
        // Создаем проецируемый файл. Если его вдруг уже создали, то получим HANDLE созданного файла
        hMappedFile = CreateFileMapping(NULL, NULL, PAGE_READWRITE, 0UL, PAGE_NUMBER*PAGE_SIZE*CHAR_SIZE, MAP_FILE_NAME);
        if(hMappedFile == NULL)
        {
            std::cout << "Failed to create file for mapping! Error code is " << GetLastError() << "\n";
            bInitSuccess = false;
        }
        // Создаем мьютексы для страниц
        for(int i = 0; bInitSuccess && i < PAGE_NUMBER; ++i)
        {
            hMutex[i] = CreateMutex(NULL, false, MUTEXNAME(i).c_str());
            if(hMutex[i] == NULL)
            {
                bInitSuccess = false;
                for(int j = 0; j < i; ++j)
                    CloseHandle(hMutex[j]);
                std::cout << "Failed to create mutexes! Error code is " << GetLastError() << "\n";
            }
        }
        if(bInitSuccess)
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
                    for(int i = 0; bInitSuccess && i < iProcessesNumber; ++i)
                    {
                        // Зачищаем структуры с информацией о процессе
                        ZeroMemory(&writerProcessInfo[i], sizeof(PROCESS_INFORMATION));
                        ZeroMemory(&readerProcessInfo[i], sizeof(PROCESS_INFORMATION));
                        // Создаем писателя
                        if(!CreateProcess(WRITER_PROCESS_NAME, NULL, NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &startInfo, &writerProcessInfo[i]))
                        {
                            bInitSuccess = false;
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
                        }
                        // Если все хорошо, то создаем читателя
                        else if(!CreateProcess(READER_PROCESS_NAME, NULL, NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &startInfo, &readerProcessInfo[i]))
                        {
                            bInitSuccess = false;
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
                        }
                    }
                    /*
                    // Ждем все процессы, чтоб создались
                    for(int i = 0; i < iProcessesNumber; ++i)
                    {
                        WaitForSingleObject(writerProcessInfo[i].hProcess, 3000);
                        WaitForSingleObject(readerProcessInfo[i].hProcess, 3000);
                    }*/
                    std::cout << "Everything's ok, press any key if you want to finish write/read processes\n";
                    delete [] hProcesses;
                    delete [] writerProcessInfo;
                    delete [] readerProcessInfo;
                }
                else
                {
                    std::cout << "Failed to allocate memory for the processes' handles or for the processes info!\n";
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
            }

            for(int i = 0; i < PAGE_NUMBER; ++i)
            {
                CloseHandle(hMutex[i]);
            }
        }
        else if(hMappedFile != NULL)
            CloseHandle(hMappedFile);
        CloseHandle(hReadSemaphore);
        CloseHandle(hWriteSemaphore);
    }
    system("pause");
    return 0;
}