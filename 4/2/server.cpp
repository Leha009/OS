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
HANDLE CreateNamedPipe_();
bool WriteToPipe(HANDLE hPipe, HANDLE hEvent);

int main()
{
    int     iMenuItem;
    HANDLE  hPipe = INVALID_HANDLE_VALUE,
            hEvent = NULL;

    do
    {
        system("cls");
        iMenuItem = SelectMenu();
        switch(iMenuItem)
        {
            case 1:         // Create named pipe
            {
                system("cls");
                if(hPipe == INVALID_HANDLE_VALUE)
                {
                    hPipe = CreateNamedPipe_();
                    if(hPipe != INVALID_HANDLE_VALUE)
                    {
                        std::cout << "The pipe was successfully created!\n";
                    }
                    else
                    {
                        std::cout << "Failed to create named pipe! Error code is " << GetLastError() << "\n";
                    }
                }
                else
                {
                    std::cout << "The pipe is already created! Close it using #6 menu item.\n";
                }
                system("pause");
                break;
            }
            case 2:         // Connect client to pipe
            {
                system("cls");
                if(hPipe == INVALID_HANDLE_VALUE)
                {
                    std::cout << "Create the named pipe first! Choose #1 menu item.\n";
                }
                else if(ConnectNamedPipe(hPipe, NULL))
                {
                    std::cout << "Client was successfully connected!\n";
                }
                else
                {
                    DWORD dwError = GetLastError();
                    if(dwError == 535)
                    {
                        std::cout << "Client is already connected to the pipe!\n";
                    }
                    else
                    {
                        std::cout << "Failed to connect client to named pipe! Error code is " << dwError << "\n";
                    }
                }
                system("pause");
                break;
            }
            case 3:         // Disconnect client from pipe
            {
                system("cls");
                if(hPipe == INVALID_HANDLE_VALUE)
                {
                    std::cout << "Create the named pipe first! Choose #1 menu item.\n";
                }
                else if(DisconnectNamedPipe(hPipe))
                {
                    std::cout << "Client was successfully disconnected!\n";
                }
                else
                {
                    std::cout << "Failed to disconnect client from named pipe! Error code is " << GetLastError() << "\n";
                }
                system("pause");
                break;
            }
            case 4:         // Create an event
            {
                system("cls");
                if(hEvent == NULL)
                {
                    hEvent = CreateEvent(NULL, false, false, NULL);
                    if(hEvent != NULL)
                    {
                        std::cout << "Event successfully created!\n";
                    }
                    else
                    {
                        std::cout << "Failed to create the event! Error code is " << GetLastError() << "\n";
                    }
                }
                else
                {
                    std::cout << "The event is already created! Delete it using #7 menu item.\n";
                }
                system("pause");
                break;
            }
            case 5:         // Write to pipe
            {
                system("cls");
                if(hPipe == INVALID_HANDLE_VALUE)
                {
                    std::cout << "Create the named pipe first! Choose #1 menu item.\n";
                }
                else if(hEvent == NULL)
                {
                    std::cout << "Create the event first! Choose #4 menu item.\n";
                }
                else
                {
                    if(WriteToPipe(hPipe, hEvent))
                    {
                        std::cout << "The message was successfully written to the pipe!\n";
                    }
                    else
                    {
                        std::cout << "Failed to write to the pipe! Error code is " << GetLastError() << "\n";
                    }
                }
                system("pause");
                break;
            }
            case 6:         // Close pipe
            {
                system("cls");
                if(hPipe != INVALID_HANDLE_VALUE)
                {
                    DisconnectNamedPipe(hPipe);
                    CloseHandle(hPipe);
                    hPipe = INVALID_HANDLE_VALUE;
                }
                std::cout << "The pipe was successfully closed!\n";
                system("pause");
                break;
            }
            case 7:         // Delete event
            {
                system("cls");
                if(hEvent != NULL)
                {
                    CloseHandle(hEvent);
                    hEvent = NULL;
                }
                std::cout << "The event was successfully deleted\n";
                system("pause");
                break;
            }
        }
    } while(iMenuItem != 0);

    if(hPipe != INVALID_HANDLE_VALUE)
    {
        CloseHandle(hPipe);
    }
    if(hEvent != NULL)
    {
        CloseHandle(hEvent);
    }
    return 0;
}

int SelectMenu()
{
    int iMenuItem;
    do
    {
        std::cout << "1. Create named pipe\n";
        std::cout << "2. Connect client to pipe\n";
        std::cout << "3. Disconnect client from pipe\n";
        std::cout << "4. Create an event\n";
        std::cout << "5. Write to pipe\n";
        std::cout << "6. Close pipe\n";
        std::cout << "7. Delete event\n";
        std::cout << "0. Close the program\n";
        std::cout << ">> ";
        std::cin >> iMenuItem;
        if(iMenuItem < 0 || iMenuItem > 7)
            std::cout << "No such menu item, try again!\n";
    } while (iMenuItem < 0 || iMenuItem > 7);
    return iMenuItem;
}

HANDLE CreateNamedPipe_()
{
    TCHAR sPipeName[BUFFER_SIZE+1];
    std::cout << "Input the name of a pipe (128 symbols maximum, must starts with \\\\.\\pipe\\): ";
    std::cin >> sPipeName;
    HANDLE hPipe = CreateNamedPipe(
        sPipeName, 
        PIPE_ACCESS_OUTBOUND,           // DUPLEX, INBOUND, OUTBOUND
        PIPE_TYPE_BYTE | PIPE_WAIT,     // Pipe Mode (PIPE_NOWAIT should not be used to achieve async I/O)
        PIPE_UNLIMITED_INSTANCES,       // MaxInstances
        BUFFER_SIZE,                    // BufferOut size
        0UL,                            // BufferIn size
        0UL,                            // Timeout (0 == 50 ms)
        NULL                            // SECURITY
    );

    return hPipe;
}

bool WriteToPipe(HANDLE hPipe, HANDLE hEvent)
{
    OVERLAPPED overlapped;
    overlapped.Offset = 0UL;
    overlapped.OffsetHigh = 0UL;
    overlapped.hEvent = hEvent;
    STRING sMessage;
    std::cout << "Input the message for client (" << BUFFER_SIZE << " symbols maximum):\n";
    std::cin.ignore();
    std::getline(std::cin, sMessage);
    bool bSuccess = WriteFile(
        hPipe, 
        sMessage.c_str(), 
        BUFFER_SIZE, 
        NULL,           // Use NULL for this parameter if this is an asynchronous operation to avoid potentially erroneous results
        &overlapped);
    if(!bSuccess)
        return false;
    WaitForSingleObject(hEvent, INFINITE);
    return true;
}