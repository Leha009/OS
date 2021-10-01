//g++ *.cpp -lWinmm -static-libstdc++

#include <iostream>
#include <windows.h>
#include "task2.h"

using namespace std;

DWORD GetClusterMinSize(LPCSTR);
DWORD GetBlockSize(DWORD, DWORD);
void ReadEnd(DWORD, DWORD, LPOVERLAPPED);
void WriteEnd(DWORD, DWORD, LPOVERLAPPED);

void Task2Run()
{
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
                                OPEN_ALWAYS,
                                FILE_FLAG_NO_BUFFERING | FILE_FLAG_OVERLAPPED,
                                NULL);
        if(hFileToCopyIn != INVALID_HANDLE_VALUE)
        {
            char* sBuff2 = new char[MAX_PATH];
            GetFullPathNameA(sBuff.c_str(), MAX_PATH, sBuff2, NULL);
            sBuff2[3] = 0;
            DWORD clusterSize = GetClusterMinSize(sBuff2);
            cout << "cluster size is " << clusterSize << '\n';
            delete sBuff2;
            bool allGood = true;
            int32_t buffer[4096];
            OVERLAPPED overlapped, overl2;
            DWORD size;
            DWORD dwBytesTrans = 0UL;
            LARGE_INTEGER fileCopySize;
            GetFileSizeEx(hFileToCopy, &fileCopySize);
            struct dwFileSize
            {
                DWORD lowPart;
                DWORD highPart;
            } udwFileSize;
            udwFileSize.lowPart = GetFileSize(hFileToCopy, &udwFileSize.highPart);
            //overlapped.Offset = 0UL;
            do
            {
                if(udwFileSize.lowPart == 0UL)
                    break;
                size = GetBlockSize(4096UL < udwFileSize.lowPart ? 4096UL : udwFileSize.lowPart, clusterSize);
                if(udwFileSize.lowPart < size)
                    udwFileSize.lowPart = 0;
                else
                    udwFileSize.lowPart -= size;
                cout << "To read: " << size << '\n';
                if(!ReadFileEx(hFileToCopy, (LPVOID)buffer, size, &overlapped, ReadEnd))
                {
                    cout << "Reading failed! Error code is " << GetLastError() << '\n';
                    allGood = false;
                    break;
                }
                SleepEx(1000UL, true);
                if(!GetOverlappedResult(hFileToCopy, &overlapped, &dwBytesTrans, false))
                {
                    cout << GetLastError();
                    allGood = false;
                    break;
                }
                cout << "Trans " << dwBytesTrans << '\n';
                if(!WriteFileEx(hFileToCopyIn, (LPVOID)buffer, GetBlockSize(dwBytesTrans, clusterSize), &overl2, WriteEnd))
                {
                    cout << "Writing failed! Error code is " << GetLastError() << '\n';
                    allGood = false;
                    break;
                }
                SleepEx(1000UL, true);
            } while (HasOverlappedIoCompleted(&overlapped));
            if(allGood)
            {
                SetFilePointerEx(hFileToCopyIn, fileCopySize, NULL, FILE_BEGIN);
                SetEndOfFile(hFileToCopyIn);
            }
            cout << "Close copy in handle\n";
            CloseHandle(hFileToCopyIn);
        }
        else
        {
            cout << "Opening file failed! Error code is " << GetLastError() << '\n';
        }
        cout << "Close copy from handle\n";
        CloseHandle(hFileToCopy);
    }
    /*DWORD dwStart = timeGetTime();
    SleepEx(5000, false);
    cout << ((timeGetTime() - dwStart) / (DWORD)1000) << " seconds\n";*/
    system("pause");
}

DWORD GetClusterMinSize(LPCSTR drive)
{
    DWORD   dwSectorsPerCluster,
            dwBytesPerSector;
    if(!GetDiskFreeSpaceA(drive, &dwSectorsPerCluster, &dwBytesPerSector, NULL, NULL))
    {
        cout << "Getting disk cluster info failed! Error code is " << GetLastError() << ',';
    }
    //return dwSectorsPerCluster*dwBytesPerSector;
    return dwBytesPerSector;
}

DWORD GetBlockSize(DWORD dwBufferSize, DWORD dwClusterSize)
{
    DWORD dwBlockSize = dwClusterSize*(dwBufferSize/dwClusterSize);
    if(dwBlockSize == 0UL)
        dwBlockSize = dwClusterSize;
    return dwBlockSize;
}

    //=========== CALLBACKS ===========//

void ReadEnd(DWORD dwErrorCode, DWORD dwNumberOfBytesTransfered, LPOVERLAPPED lpOverlapped)
{
    DWORD LastError = GetLastError();
    if(dwErrorCode != 0UL)// && LastError != 0UL)
    {
        cout << "Read failed! Error code is " << GetLastError() << " | " << dwErrorCode << '\n';
    }
    else if(dwNumberOfBytesTransfered != 0)
    {
        /*lpOverlapped->Offset += dwNumberOfBytesTransfered;
        cout << "Read " << dwNumberOfBytesTransfered << '\n';
        /*int32_t buffer[1024];
        if(!WriteFileEx(hFileToCopyIn, (LPVOID)buffer, 1024UL, lpOverlapped, WriteEnd))
        {
            cout << "Writing2 failed! Error code is " << GetLastError() << '\n';
        }
        SleepEx(1000UL, true);*/
    }
}

void WriteEnd(DWORD dwErrorCode, DWORD dwNumberOfBytesTransfered, LPOVERLAPPED lpOverlapped)
{
    DWORD LastError = GetLastError();
    if(dwErrorCode != 0UL)// && LastError != 0UL)
    {
        cout << "Write failed! Error code is " << LastError << " | " << dwErrorCode << '\n';
    }
    else if(dwNumberOfBytesTransfered != 0)
    {
        /*lpOverlapped->Offset += dwNumberOfBytesTransfered;
        cout << "Wrote " << dwNumberOfBytesTransfered << '\n';
        /*int32_t buffer[1024];
        //lpOverlapped->Offset += dwNumberOfBytesTransfered;
        if(!ReadFileEx(hFileToCopy, (LPVOID)buffer, 1024UL, lpOverlapped, ReadEnd))
        {
            cout << "Read2 failed! Error code is " << GetLastError() << '\n';
        }
        SleepEx(1000UL, true);*/
    }
}