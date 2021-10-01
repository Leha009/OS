//g++ *.cpp -lWinmm

#include <iostream>
#include <windows.h>
#include "task2.h"

using namespace std;

DWORD GetClusterMinSize(LPCSTR);

void Task2Run()
{
    string sBuff;
    DWORD dwMilliseconds;
    HANDLE  hFileToCopy,
            hFileReadEvent,
            hFileToCopyIn,
            hFileWriteEvent;
    
    cout << "Input the file name to copy: ";
    cin >> sBuff;
    hFileToCopy = CreateFile(  sBuff.c_str(),
                                GENERIC_READ | GENERIC_WRITE,
                                0,
                                NULL,
                                OPEN_ALWAYS,
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
            sBuff = sBuff.at(0) + ":\\";
            cout << "cluster size is " << GetClusterMinSize(sBuff.c_str());
            CloseHandle(hFileToCopyIn);
        }
        else
        {
            cout << "Opening file failed! Error code is " << GetLastError() << '\n';
        }
        CloseHandle(hFileToCopy);
    }
    /*DWORD dwStart = timeGetTime();
    SleepEx(5000, false);
    cout << ((timeGetTime() - dwStart) / (DWORD)1000) << " seconds\n";*/
    system("pause");
}

DWORD GetClusterMinSize(LPCSTR sFileName)
{
    DWORD   dwSectorsPerCluster,
            dwBytesPerSector;
    TCHAR    sBuffer[3];
    GetFullPathNameA(sFileName, 3, sBuffer, NULL);
    cout << "TEST : " << sBuffer << '\n';
    /*if(!GetDiskFreeSpaceA(diskName, &dwSectorsPerCluster, &dwBytesPerSector, NULL, NULL))
    {
        cout << "Getting disk cluster info failed! Error code is " << GetLastError() << '\n';
    }*/
    return dwSectorsPerCluster*dwBytesPerSector;
}