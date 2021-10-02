//g++ *.cpp -lWinmm -static-libstdc++

#include <iostream>
#include <windows.h>
#include "task2.h"

using namespace std;

#define BUFF_SIZE 16384UL
//#define BUFF_SIZE 1024UL
#define SLEEP_TIME -1

void ReadEnd(DWORD, DWORD, LPOVERLAPPED);
void WriteEnd(DWORD, DWORD, LPOVERLAPPED);
bool CopyFile_(HANDLE, HANDLE, LARGE_INTEGER*, DWORD, int);
DWORD GetClusterMinSizeWithFileDrive(LPCSTR);
DWORD GetClusterMinSize(LPCSTR);
DWORD GetBlockSize(DWORD, DWORD);

LARGE_INTEGER liShift;

void Task2Run()
{
    system("cls");
    string sBuff;
    DWORD dwMilliseconds;
    HANDLE  hFileToCopy,
            hFileToCopyIn;
    
    cout << "Input the file name to copy: ";
    cin >> sBuff;
    hFileToCopy = CreateFile(  sBuff.c_str(),
                                GENERIC_READ | GENERIC_WRITE,
                                0,
                                NULL,
                                OPEN_EXISTING,
                                FILE_FLAG_NO_BUFFERING | FILE_FLAG_OVERLAPPED,
                                NULL);
    if(hFileToCopy == INVALID_HANDLE_VALUE)
    {
        cout << "Opening file failed! Error code is " << GetLastError() << '\n';
    }
    else
    {
        cout << "Input the file name to copy in: ";
        cin >> sBuff;
        hFileToCopyIn = CreateFile(  sBuff.c_str(),
                                GENERIC_READ | GENERIC_WRITE,
                                0,
                                NULL,
                                CREATE_ALWAYS,
                                FILE_FLAG_NO_BUFFERING | FILE_FLAG_OVERLAPPED,
                                NULL);
        if(hFileToCopyIn != INVALID_HANDLE_VALUE)
        {
            LARGE_INTEGER liFileSize;
            DWORD clusterSize = GetClusterMinSizeWithFileDrive(sBuff.c_str());
            DWORD blockSize;
            int operations;
            cout << "Input block size: " << clusterSize << "*";
            cin >> blockSize;
            blockSize *= clusterSize;

            cout << "Input operations amount: ";
            do
            {
                cin >> operations;
                if(operations < 1) 
                    cout << "Operations amount can't be negative!\n";
            } while(operations < 1);
            DWORD start = GetTickCount();
            if(CopyFile_(hFileToCopy, hFileToCopyIn, &liFileSize, blockSize, operations))
            {
                SetFilePointerEx(hFileToCopyIn, liFileSize, NULL, FILE_BEGIN);
                SetEndOfFile(hFileToCopyIn);
            }
            cout << "The copying took " << ((double)(GetTickCount()-start)/1000) << " seconds\n";
            if(!CloseHandle(hFileToCopyIn))
            {
                cout << "Closing handle failed! Error code is " << GetLastError() << '\n';
            }
        }
        else
        {
            cout << "Opening file failed! Error code is " << GetLastError() << '\n';
        }
        if(!CloseHandle(hFileToCopy))
        {
            cout << "Closing handle failed! Error code is " << GetLastError() << '\n';
        }
    }
    system("pause");
}

bool CopyFile_(HANDLE hFileToCopy, HANDLE hFileToCopyIn, LARGE_INTEGER* liFileSize, DWORD blockSize, int operations)
{
    LARGE_INTEGER   liFileSizeRead,
                    liFileSizeWrite;  //Используется, чтобы ловить конец считывания
    if(!GetFileSizeEx(hFileToCopy, liFileSize))
    {
        cout << "Can't get file size. Error code is " << GetLastError() << '\n';
        return false;
    }
    liFileSizeWrite = liFileSizeRead = *liFileSize;

    OVERLAPPED* overRead = new OVERLAPPED[operations];
    OVERLAPPED* overWrite = new OVERLAPPED[operations];
    liShift.QuadPart = 0LL;

    char** buffer = new char*[operations];
    for(int i = 0; i < operations; i++)
    {
        buffer[i] = new char[(int)blockSize];

        overRead[i].Offset = overWrite[i].Offset = liShift.LowPart;
        overRead[i].OffsetHigh = overWrite[i].OffsetHigh = (DWORD)liShift.HighPart;
        liShift.QuadPart += (long long)blockSize;
    }

    bool allGood = true;

    do 
    {
        for(int i = 0; i < operations; i++)
        {
            if(liFileSizeRead.QuadPart > 0LL)
            {
                if(!ReadFileEx(hFileToCopy, buffer[i], blockSize, &overRead[i], ReadEnd))
                {
                    cout << "Reading failed! Error code is " << GetLastError() << '\n';
                    allGood = false;
                    break;
                }
                SleepEx(SLEEP_TIME, true);
                liFileSizeRead.QuadPart -= (long long)blockSize;
            }
        }

        if(allGood)
        {
            for(int i = 0; i < operations; i++)
            {
                if(liFileSizeWrite.QuadPart > 0LL)
                {
                    if(!WriteFileEx(hFileToCopyIn, buffer[i], blockSize, &overWrite[i], WriteEnd))
                    {
                        cout << "Writing failed! Error code is " << GetLastError() << '\n';
                        allGood = false;
                        break;
                    }
                    SleepEx(SLEEP_TIME, true);
                    liFileSizeWrite.QuadPart -= blockSize;
                }
            }
        }
        for(int i = 0; i < operations; i++)
        {
            overRead[i].Offset = overWrite[i].Offset = liShift.LowPart;
            overRead[i].OffsetHigh = overWrite[i].OffsetHigh = (DWORD)liShift.HighPart;
            liShift.QuadPart += (long long)blockSize;
        }
    } while(allGood && liFileSizeRead.QuadPart > 0LL);

    for(int i = 0; i < operations; i++)
        delete buffer[i];

    delete buffer;
    delete overRead;
    delete overWrite;

    return allGood;
}

DWORD GetClusterMinSizeWithFileDrive(LPCSTR lpFileName)
{
    char* sBuff = new char[MAX_PATH];
    GetFullPathNameA(lpFileName, MAX_PATH, sBuff, NULL);
    sBuff[3] = 0;
    DWORD dwMinSize = GetClusterMinSize(sBuff);
    delete sBuff;
    return dwMinSize;
}

DWORD GetClusterMinSize(LPCSTR drive)
{
    DWORD   dwSectorsPerCluster,
            dwBytesPerSector;
    if(!GetDiskFreeSpaceA(drive, &dwSectorsPerCluster, &dwBytesPerSector, NULL, NULL))
    {
        cout << "Getting disk cluster info failed! Error code is " << GetLastError() << ',';
    }
    return dwBytesPerSector;
}

DWORD GetBlockSize(DWORD dwBufferSize, DWORD dwClusterSize)
{
    DWORD dwBlockSize = dwClusterSize*(dwBufferSize/dwClusterSize+1);
    return dwBlockSize;
}

    //=========== CALLBACKS ===========//

void ReadEnd(DWORD dwErrorCode, DWORD dwNumberOfBytesTransfered, LPOVERLAPPED lpOverlapped)
{
    if(dwErrorCode != 0UL)
    {
        cout << "Read failed! Error code is " << GetLastError() << " | " << dwErrorCode << '\n';
    }
    else if(dwNumberOfBytesTransfered != 0)
    {
        lpOverlapped->Offset += dwNumberOfBytesTransfered;
    }
}

void WriteEnd(DWORD dwErrorCode, DWORD dwNumberOfBytesTransfered, LPOVERLAPPED lpOverlapped)
{
    if(dwErrorCode != 0UL)
    {
        cout << "Write failed! Error code is " << GetLastError() << " | " << dwErrorCode << '\n';
    }
    else if(dwNumberOfBytesTransfered != 0)
    {
        lpOverlapped->Offset += dwNumberOfBytesTransfered;
    }
}