//g++ *.cpp -lWinmm -static-libstdc++

#include <iostream>
#include <windows.h>
#include "task2.h"

using namespace std;

#define BUFF_SIZE 16384UL
//#define BUFF_SIZE 1024UL
#define SLEEP_TIME -1

DWORD GetClusterMinSizeWithFileDrive(LPCSTR);
DWORD GetClusterMinSize(LPCSTR);
DWORD GetBlockSize(DWORD, DWORD);
void ReadEnd(DWORD, DWORD, LPOVERLAPPED);
void WriteEnd(DWORD, DWORD, LPOVERLAPPED);

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
            DWORD clusterSize = GetClusterMinSizeWithFileDrive(sBuff.c_str());
            bool allGood = true;
            int32_t buffer[BUFF_SIZE];
            OVERLAPPED overlapped, overl2;
            overlapped.Offset = overl2.Offset = 0UL;
            DWORD size;
            DWORD dwBytesTrans = 0UL;
            LARGE_INTEGER fileCopySize;
            if(!GetFileSizeEx(hFileToCopy, &fileCopySize))
            {
                cout << "Getting file size failed! Error code is " << GetLastError() << '\n';
            }
            struct dwFileSize
            {
                DWORD lowPart;
                DWORD highPart;
            } udwFileSize;
            udwFileSize.lowPart = GetFileSize(hFileToCopy, &udwFileSize.highPart);
            cout << "File size is " << udwFileSize.highPart << " | " << udwFileSize.lowPart << '\n';
            DWORD start = GetTickCount();
            do
            {
                size = GetBlockSize(BUFF_SIZE < udwFileSize.lowPart ? BUFF_SIZE : udwFileSize.lowPart, clusterSize);
                
                if(udwFileSize.highPart < size)
                {
                    if(udwFileSize.highPart != 0UL)
                    {
                        udwFileSize.lowPart -= size-udwFileSize.highPart;
                        udwFileSize.highPart = 0;
                    }
                    else
                    {
                        if(udwFileSize.lowPart < size)
                            udwFileSize.lowPart = 0;
                        else
                            udwFileSize.lowPart -= size;
                    }
                }
                else
                {
                    udwFileSize.highPart -= size;
                }
                
                if(!ReadFileEx(hFileToCopy, (LPVOID)buffer, size, &overlapped, ReadEnd))
                {
                    cout << "Reading failed! Error code is " << GetLastError() << '\n';
                    allGood = false;
                    break;
                }
                SleepEx(SLEEP_TIME, true);
                if(!GetOverlappedResult(hFileToCopy, &overlapped, &dwBytesTrans, false))
                {
                    cout << GetLastError();
                    allGood = false;
                    break;
                }
                cout << "Trans " << dwBytesTrans << '\n';   //DEBUG
                if(!WriteFileEx(hFileToCopyIn, (LPVOID)buffer, size, &overl2, WriteEnd))
                {
                    cout << "Writing failed! Error code is " << GetLastError() << '\n';
                    allGood = false;
                    break;
                }
                SleepEx(SLEEP_TIME, true);
            //} while (!HasOverlappedIoCompleted(&overlapped) && !HasOverlappedIoCompleted(&overl2));
            } while (udwFileSize.lowPart != 0UL);
            if(allGood)
            {
                cout << "Set pointer at " << fileCopySize.HighPart << " | " << fileCopySize.LowPart << '\n';    //DEBUG
                SetFilePointerEx(hFileToCopyIn, fileCopySize, NULL, FILE_BEGIN);
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