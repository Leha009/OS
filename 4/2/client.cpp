#include <iostream>
#include <windows.h>

#ifdef _UNICODE
#define STRING std::wstring
#define STRSIZE sizeof(WCHAR)
#else
#define STRING std::string
#define STRSIZE sizeof(CHAR)
#endif

#define BUFFER_SIZE 256

int SelectMenu();
HANDLE ConnectToPipe();
void ReadMessageFromPipe(HANDLE hPipe);
void KindaFunc(DWORD dwErrorCode, DWORD dwNumberOfBytesTransfered, LPOVERLAPPED lpOverlapped);

int main()
{
    int iMenuItem;
    HANDLE hPipe = INVALID_HANDLE_VALUE;
    do
    {
        system("cls");
        iMenuItem = SelectMenu();
        if(iMenuItem == 1)              // Connect to pipe
        {
            system("cls");
            if(hPipe != INVALID_HANDLE_VALUE)
                CloseHandle(hPipe);
            hPipe = ConnectToPipe();
            if(hPipe != INVALID_HANDLE_VALUE)
            {
                std::cout << "Connecting to a named pipe was successful!\n";
            }
            else
            {
                std::cout << "Failed to connect to a named pipe! Error code is " << GetLastError() << "\n";
            }
            system("pause");
        }
        else if(iMenuItem == 2)         // Read from pipe
        {
            system("cls");
            ReadMessageFromPipe(hPipe);
            system("pause");
        }
    } while (iMenuItem != 0);
    
    if(hPipe != INVALID_HANDLE_VALUE)
        CloseHandle(hPipe);
    return 0;
}

int SelectMenu()
{
    int iMenuItem;
    do
    {
        std::cout << "1. Connect to a named pipe\n";
        std::cout << "2. Read from the pipe\n";
        std::cout << "0. Close the program\n";
        std::cout << ">> ";
        std::cin >> iMenuItem;
        if(iMenuItem < 0 || iMenuItem > 2)
            std::cout << "No such menu item, try again!\n";
    } while (iMenuItem < 0 || iMenuItem > 2);
    return iMenuItem;
}

HANDLE ConnectToPipe()
{
    HANDLE hPipe;
    TCHAR sPipeName[BUFFER_SIZE+1];
    std::cout << "Input the name of a pipe (" << BUFFER_SIZE << " symbols maximum, must starts with \\\\.\\pipe\\): ";
    std::cin >> sPipeName;
    hPipe = CreateFile(
        sPipeName,
        GENERIC_READ,
        0UL,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_READONLY | FILE_FLAG_OVERLAPPED,
        NULL
    );
    return hPipe;
}

void ReadMessageFromPipe(HANDLE hPipe)
{
    TCHAR sBuffer[BUFFER_SIZE+1];
    OVERLAPPED overlapped;
    overlapped.Offset = 0UL;
    overlapped.OffsetHigh = 0UL;
    if(!ReadFileEx(hPipe, sBuffer, sizeof(sBuffer), &overlapped, KindaFunc))
    {
        DWORD dwError = GetLastError();
        if(dwError == 6)
        {
            std::cout << "You wasn't connected to the pipe, try to connect again!\n";
        }
        else if(dwError == 109)
        {
            std::cout << "The pipe was closed :c\n";
        }
        else if(dwError == 233)
        {
            std::cout << "You was disconnected from the pipe, try to connect again!\n";
        }
        else
        {
            std::cout << "Failed to read message from the pipe! Error code is " << dwError << "\n";
        }
    }
    else
    {
        SleepEx(INFINITE, true);
        std::cout << "Received message:\n" << sBuffer << '\n';
    }
}

void KindaFunc(DWORD dwErrorCode, DWORD dwNumberOfBytesTransfered, LPOVERLAPPED lpOverlapped) {}