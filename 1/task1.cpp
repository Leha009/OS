#include <iostream>
#include <sstream>
#include <windows.h>
#include "task1.h"

using namespace std;

#define BUFSIZE 512

string GetFullDate(LPSYSTEMTIME);
void FillFileTime(LPFILETIME);
int SelectionMenu();
void PrintLogicalDrives(bool = false);
void PrintDriveType(LPCSTR);
void PrintVolumeInfo(LPCSTR);
void PrintDiskFreeSpace(LPCSTR);
void PrintDriveInfo(LPCSTR);
void CreateRemoveDirectoryMenu();
void CreateFile_();
void MoveFile_();
void CopyFile_();
void GetFileAttributes_();
void SetFileAttributes_();
void GetFileTime_();
void SetFileTime_();

void Task1Run()
{
    int iMenuItem,
        iBuff;
    string sBuff;
    
    do
    {
        iMenuItem = SelectionMenu();
        switch (iMenuItem)
        {
        case 1:
            system("cls");
            PrintLogicalDrives(true);
            system("pause");
            break;
        case 2:
            system("cls");
            cout << "Which drive do you want to know info about? Input the letter\n";
            PrintLogicalDrives();
            cin >> sBuff;
            if(sBuff.find(":\\") == -1)
            {
                sBuff += ":\\";
            }
            system("cls");
            PrintDriveInfo(sBuff.c_str());
            system("pause");
            break;
        case 3:
            CreateRemoveDirectoryMenu();
            break;
        case 4:
            system("cls");
            CreateFile_();
            system("pause");
            break;
        case 5:
            system("cls");
            CopyFile_();
            system("pause");
            break;
        case 6:
            system("cls");
            MoveFile_();
            system("pause");
            break;
        case 7:
            system("cls");
            GetFileAttributes_();
            system("pause");
            break;
        case 8:
            system("cls");
            SetFileAttributes_();
            system("pause");
            break;
        case 9:
            system("cls");
            GetFileTime_();
            system("pause");
            break;
        case 10:
            system("cls");
            SetFileTime_();
            system("pause");
            break;
        default:
            break;
        }
    } while (iMenuItem != 0);
}

int SelectionMenu()
{
    int iMenuItem;
    do
    {
        system("cls");
        cout << "1. Get drives list\n";
        cout << "2. Get drive info\n";
        cout << "3. Create/Remove directory\n";
        cout << "4. Create file\n";
        cout << "5. Copy file\n";
        cout << "6. Move file\n";
        cout << "7. Get file attributes\n";
        cout << "8. Set file attributes\n";
        cout << "9. Get file time\n";
        cout << "10. Set file time\n";
        cout << "0. Back to main menu\n";
        cout << "\nYour choice is ";
        cin >> iMenuItem;
        if(iMenuItem < 0 || iMenuItem > 10)
        {
            cout << "You inputted wrong number! Try again.";
            system("pause");
        }
    } while(iMenuItem < 0 || iMenuItem > 10);
    return iMenuItem;
}

/* ???????????????? ???????????? ???????????? //https://tinyurl.com/4rndwjw8 */
void PrintLogicalDrives(bool outputNumInMask)
{
    DWORD _LogicalDrives = GetLogicalDrives();    //???????????????? ?????????????? ?????????? ????????????
    int driveBits = 0;          //??????????????????????, ?????????? ???????????????? ???????????? ?? ???????????? ??????????, ?????????????????????????? ?????? ?????????? ?? ?????????????? ??????????
    TCHAR drives[BUFSIZE];      //???????? ???? ?????????????? ?????? ?????????????????? ?? ?????? ??????????
    TCHAR buff[2] = TEXT(":");  /*TEXT = https://tinyurl.com/2h44tvmy. ??????????, ?????????? ???????????????????????? ???????????? ??????????, ?????? :\ */
    TCHAR* p = drives;          //?????????????????? ?????????? ?????????????????? ???? ?????????? ???????????? ???? ???????? ?????????????????? ???????????? (???????????? ????????????)
    GetLogicalDriveStrings(BUFSIZE-1, drives);  //BUFSIZE-1, ??.??. ???? ?????????????????????? ????????-????????????????????
    cout << "Drives available:\n";
    do
    {
        *buff = *p;
        if(outputNumInMask)
            for(; driveBits < 32 && (_LogicalDrives >> driveBits & 1) != 1; driveBits++);
            //???????????????? ?????? ???????????? ?? ??????????????. ????????????, ?? ?????? ??????????????? ?? ??????, ?????? ???? ???????????????? ???????????? ?????? ???????? (:
        //for(; driveBits < 32 && _LogicalDrives >> driveBits & 1 != 1; driveBits++);   
        cout << buff;   //?????????????? ?????????? ??????????
        if(outputNumInMask)
            cout << '(' << ((DWORD)1 << driveBits++) << ") ";  //?????????????? ??????????, ?????????????????????????? ?????????? ?????????? ?? ?????????????? ??????????
        else
            cout << ' '; 
        while(*p++);    //???????? ???? ?????????????????? ??????????
    } while(*p);
    cout << '\n';
}

void PrintDriveInfo(LPCSTR drive)
{
    PrintDriveType(drive);
    PrintVolumeInfo(drive);
    PrintDiskFreeSpace(drive);
}

/* ???????????????? ?????? ?????????? https://tinyurl.com/wvy3mm5x */
void PrintDriveType(LPCSTR drive)
{
    DWORD dwType = GetDriveType(drive);
    cout << "Drive type:\n";
    switch (dwType)
    {
        case 0:
            cout << "\tThe drive type cannot be determined.\n";
            break;
        case 1:
            cout << "\tThe root path is invalid; for example, there is no volume mounted at the specified path.\n";
            break;
        case 2:
            cout << "\tThe drive has removable media; for example, a floppy drive, thumb drive, or flash card reader.\n";
            break;
        case 3:
            cout << "\tThe drive has fixed media; for example, a hard disk drive or flash drive.\n";
            break;
        case 4:
            cout << "\tThe drive is a remote (network) drive.\n";
            break;
        case 5:
            cout << "\tThe drive is a CD-ROM drive.\n";
            break;
        case 6:
            cout << "\tThe drive is a RAM disk.\n";
            break;
        default:
            cout << "\tHow did you get this case?! ???(??????? )???\n";
            break;
    }
}

void PrintVolumeInfo(LPCSTR drive)
{
    TCHAR   sVolumeNameBuffer[MAX_PATH+1],
            sFileSystemNameBuffer[MAX_PATH+1];
    DWORD   dwVolumeSerialNumber = 0UL,     //???????????????? ??????????
            dwMaximumComponentLength = 0UL, //????????. ?????????? ?? ???????????????? ???????????????????? ?????????? ??????????
            dwFileSystemFlags = 0UL;
    bool bSuccess = GetVolumeInformation(
                        drive,
                        sVolumeNameBuffer, 
                        MAX_PATH+1,
                        &dwVolumeSerialNumber,
                        &dwMaximumComponentLength,
                        &dwFileSystemFlags,
                        sFileSystemNameBuffer,
                        MAX_PATH+1);
    if(bSuccess)
    {
        cout << "Name: " << (strlen(sVolumeNameBuffer) > 0 ? sVolumeNameBuffer : "Local disk");
        cout << " | Serial number: " << hex << dwVolumeSerialNumber << dec << '\n';
        cout << "Max component length: " << dwMaximumComponentLength << '\n';
        cout << "fileSystemName: " << sFileSystemNameBuffer << '\n';
        cout << "\nThe specified volume:";
        if(dwFileSystemFlags & FILE_CASE_PRESERVED_NAMES)   cout << "\n\t- supports preserved case of file names when it places a name on disk";
        if(dwFileSystemFlags & FILE_CASE_SENSITIVE_SEARCH)  cout << "\n\t- supports case-sensitive file names";
        if(dwFileSystemFlags & FILE_FILE_COMPRESSION)       cout << "\n\t- supports file-based compression";
        else if(dwFileSystemFlags & FILE_VOLUME_IS_COMPRESSED)  cout << "\n\t- is a compressed volume";
        if(dwFileSystemFlags & FILE_NAMED_STREAMS)          cout << "\n\t- supports named streams";
        if(dwFileSystemFlags & FILE_PERSISTENT_ACLS)        cout << "\n\t- preserves and enforces access control lists";
        if(dwFileSystemFlags & FILE_READ_ONLY_VOLUME)       cout << "\n\t- is read-only";
        if(dwFileSystemFlags & FILE_SEQUENTIAL_WRITE_ONCE)  cout << "\n\t- supports a single sequential write";
        if(dwFileSystemFlags & FILE_SUPPORTS_ENCRYPTION)    cout << "\n\t- supports the Encrypted File System";
        if(dwFileSystemFlags & FILE_SUPPORTS_EXTENDED_ATTRIBUTES)   cout << "\n\t- supports extended attributes";
        if(dwFileSystemFlags & FILE_SUPPORTS_HARD_LINKS)    cout << "\n\t- supports hard links";
        if(dwFileSystemFlags & FILE_SUPPORTS_OBJECT_IDS)    cout << "\n\t- supports object identifiers";
        if(dwFileSystemFlags & FILE_SUPPORTS_OPEN_BY_FILE_ID)   cout << "\n\t- supports open by FileID";
        if(dwFileSystemFlags & FILE_SUPPORTS_REPARSE_POINTS)    cout << "\n\t- supports reparse points";
        if(dwFileSystemFlags & FILE_SUPPORTS_SPARSE_FILES)  cout << "\n\t- supports sparse files";
        if(dwFileSystemFlags & FILE_SUPPORTS_TRANSACTIONS)  cout << "\n\t- supports transactions";
        if(dwFileSystemFlags & FILE_SUPPORTS_USN_JOURNAL)   cout << "\n\t- supports update sequence number journals";
        if(dwFileSystemFlags & FILE_UNICODE_ON_DISK)        cout << "\n\t- supports Unicode in file names as they appear on disk";
        if(dwFileSystemFlags & FILE_VOLUME_QUOTAS)          cout << "\n\t- supports disk quotas";
        if(dwFileSystemFlags == 0UL)                        cout << "\n\t- failed during getting flags";
        cout << '\n';
    }
    else
    {
        cout << "Something went wrong with volume info getting :( Code is " << GetLastError() << '\n';
    }
}

/* ???????????????? ???????????????????? ?? ??????????, ?????????????? ?????????? ???????????????????? ?????????? https://tinyurl.com/37nm6vuc */
void PrintDiskFreeSpace(LPCSTR drive)
{
    DWORD   dwSectorsPerCluster = 0UL,
            dwBytesPerSector = 0UL,
            dwNumberOfFreeClusters = 0UL,
            dwTotalNumberOfClusters = 0UL;
    bool bSuccess = GetDiskFreeSpaceA(
                    drive, 
                    &dwSectorsPerCluster,
                    &dwBytesPerSector,
                    &dwNumberOfFreeClusters,
                    &dwTotalNumberOfClusters);
    if(bSuccess)
    {
        cout << "\n==== DISK SPACE INFO =====\nSectors per cluster: " << dwSectorsPerCluster << "\nBytes per sector: " << dwBytesPerSector << '\n';
        cout << "Number of free clusters: " << dwNumberOfFreeClusters << "\nTotal number of clusters: " << dwTotalNumberOfClusters << '\n';
        cout << "Free space: " << ((int64_t)dwSectorsPerCluster * dwBytesPerSector * dwNumberOfFreeClusters) << " bytes\n"; //???????????? ???? 1024 ?????????????? ??????, ?????????????? ???????? ?????? ???????? ?????????????? (??????????, ??????????, ??????????, ??????????)
        cout << "Total space: " << ((int64_t)dwSectorsPerCluster * dwBytesPerSector * dwTotalNumberOfClusters) << " bytes\n";
    }
    else
    {
        cout << "Something went wrong with disk free space getting :(\n";
    }
}

void CreateRemoveDirectoryMenu()
{
    system("cls");
    int iMenuItem;
    string sBuff;
    do
    {
        cout << "1. Create directory\n";
        cout << "2. Remove directory\n";
        cout << "0. Back to main menu\n";
        cin >> iMenuItem;
    } while (iMenuItem < 0 || iMenuItem > 2);
    if(iMenuItem == 1)
    {
        cout << "Input the path for new directory: ";
        cin >> sBuff;
        if(CreateDirectoryA(sBuff.c_str(), NULL))
        {
            cout << "The directory successfully created!\n";
        }
        else
        {
            cout << "Creation failed! The directory already exists or one or more intermediate directories do not exist.\n";
        }
        system("pause");
    }
    else if(iMenuItem == 2)
    {
        cout << "Input the directory's path to be removed: ";
        cin >> sBuff;
        if(RemoveDirectoryA(sBuff.c_str()))
        {
            cout << "The directory successfully removed!\n";
        }
        else
        {
            cout << "Remove failed! Error code is " << GetLastError() << '\n';
        }
        system("pause");
    }
}

void CreateFile_()
{
    string  sFileName;
    DWORD   dwDesiredAccess,
            dwShareMode;
    cout << "Input the file name: ";
    cin >> sFileName;
    cout << "Input the access for the file (1 - READ, 2 - WRITE, 3 - READ AND WRITE)\n";
    cin >> dwDesiredAccess;
    if(dwDesiredAccess == 1)
        dwDesiredAccess = GENERIC_READ;
    else if(dwDesiredAccess == 2)
        dwDesiredAccess = GENERIC_WRITE;
    else
        dwDesiredAccess = GENERIC_READ | GENERIC_WRITE;
    
    cout << "Input the share mode for the file (1 - READ, 2 - WRITE, 4 - DELETE. Sum the numbers to get combination)\n";
    cin >> dwShareMode;

    HANDLE hFile = CreateFile(sFileName.c_str(), dwDesiredAccess, dwShareMode, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
    if(hFile != INVALID_HANDLE_VALUE)
    {
        cout << "The file was successfully created!\n";
		CloseHandle(hFile);
    }
    else
    {
        cout << "Creation failed! Error code is " << GetLastError() << '\n';
    }
}

void CopyFile_()
{
    string  sFileNameToCopy,
            sNewFileName;
    bool    overwriteIfExists;
    cout << "Input the name of file to be copied: ";
    cin >> sFileNameToCopy;
    cout << "Input the name of the copy of the file: ";
    cin >> sNewFileName;
    cout << "Overwrite the file if it exists? (1 - yes, 0 - no): ";
    cin >> overwriteIfExists;
    if(CopyFile(sFileNameToCopy.c_str(), sNewFileName.c_str(), overwriteIfExists))
    {
        cout << "The file was successfully copied!\n";
    }
    else
    {
        cout << "The process of copying failed! Error code is " << GetLastError() << '\n';
    }
}

void MoveFile_()
{
    string  sFileNameToMove,
            sNewFileName;
    DWORD   dwFlags;
    cout << "Input the name of file to be moved: ";
    cin >> sFileNameToMove;
    cout << "Input new name of the file: ";
    cin >> sNewFileName;
    cout << "Overwrite the file if it exists? (1 - yes, 0 - no): ";
    cin >> dwFlags;
    if(dwFlags == 1)
        dwFlags = MOVEFILE_REPLACE_EXISTING;
    if(dwFlags == MOVEFILE_REPLACE_EXISTING && MoveFileExA(sFileNameToMove.c_str(), sNewFileName.c_str(), dwFlags))
    {
        cout << "The file was successfully moved!\n";
    }
    else if(MoveFile(sFileNameToMove.c_str(), sNewFileName.c_str()))
    {
        cout << "The file was successfully moved!\n";
    }
    else
    {
        cout << "The process of moving the file failed! Error code is " << GetLastError() << '\n';
    }
}

void GetFileAttributes_()
{
    string sFileName;
    DWORD dwFileAttributes;
    bool bByHandle;
    cout << "Input the name of file, which attributes you want to see: ";
    cin >> sFileName;

    cout << "Use handle to get the attributes? 1 - yes, 0 - no: ";
    cin >> bByHandle;

    HANDLE hFile = CreateFile(  sFileName.c_str(), 
                                    GENERIC_READ, 
                                    FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE,
                                    NULL,
                                    OPEN_EXISTING,
                                    FILE_ATTRIBUTE_NORMAL,
                                    NULL);
    if(hFile == INVALID_HANDLE_VALUE)
    {
        cout << "File opening failed! Error code is " << GetLastError() << '\n';
        return;
    }
    //???????????????? ???????????????? ?????????? HANDLE
    if(bByHandle)
    {
        BY_HANDLE_FILE_INFORMATION fileInfo;
        GetFileInformationByHandle(hFile, &fileInfo);
        dwFileAttributes = fileInfo.dwFileAttributes;
    }
    else
    {
        dwFileAttributes = GetFileAttributes(sFileName.c_str());
    }
    if(dwFileAttributes == INVALID_FILE_ATTRIBUTES)
    {
        cout << "The process of getting the file's attributes failed! Error code is " << GetLastError() << '\n';
		CloseHandle(hFile);
        return;
    }
    cout << "The file attributes:\n";
    if(dwFileAttributes & FILE_ATTRIBUTE_ARCHIVE) cout << "\tarchived\n";
    if(dwFileAttributes & FILE_ATTRIBUTE_COMPRESSED) cout << "\tcompressed\n";
    if(dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) cout << "\tdirectory\n";
    if(dwFileAttributes & FILE_ATTRIBUTE_ENCRYPTED) cout << "\tencrypted\n";
    if(dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) cout << "\thidden\n";
    if(dwFileAttributes & FILE_ATTRIBUTE_NORMAL) cout << "\tnormal\n";
    if(dwFileAttributes & FILE_ATTRIBUTE_READONLY) cout << "\tread-only\n";
    if(dwFileAttributes & FILE_ATTRIBUTE_SYSTEM) cout << "\tsystem\n";
    if(dwFileAttributes & FILE_ATTRIBUTE_TEMPORARY) cout << "\ttemporary\n";
    if(dwFileAttributes & FILE_ATTRIBUTE_VIRTUAL) cout << "\tvirtual\n";
    CloseHandle(hFile);
}

void SetFileAttributes_()	//?????????????????? ????????????????
{
    string sFileName;
    int iBuff = 0;
    DWORD dwFileAttributes = 0UL;
    cout << "Input the name of file, which attributes you want to set: ";
    cin >> sFileName;

    cout << "\n===== Change attributes =====\n";
    cout << "Input the file attributes to set in one line (separated by space). 0 is end of attributes.";
    cout << "\nList of available attributes:";
    cout << "\n1. archived";
    cout << "\n2. compressed";
    cout << "\n3. directory";
    cout << "\n4. encrypted";
    cout << "\n5. hidden";
    cout << "\n6. normal";
    cout << "\n7. read-only";
    cout << "\n8. system";
    cout << "\n9. temporary";
    cout << "\n10. virtual";
    cout << "\nInput the attributes here: ";
    do
    {
        cin >> iBuff;
        switch(iBuff)
        {
            case 0:
                break;
            case 1:
                dwFileAttributes |= FILE_ATTRIBUTE_ARCHIVE;
                break;
            case 2:
                dwFileAttributes |= FILE_ATTRIBUTE_COMPRESSED;
                break;
            case 3:
                dwFileAttributes |= FILE_ATTRIBUTE_DIRECTORY;
                break;
            case 4:
                dwFileAttributes |= FILE_ATTRIBUTE_ENCRYPTED;
                break;
            case 5:
                dwFileAttributes |= FILE_ATTRIBUTE_HIDDEN;
                break;
            case 6:
                dwFileAttributes |= FILE_ATTRIBUTE_NORMAL;
                break;
            case 7:
                dwFileAttributes |= FILE_ATTRIBUTE_READONLY;
                break;
            case 8:
                dwFileAttributes |= FILE_ATTRIBUTE_SYSTEM;
                break;
            case 9:
                dwFileAttributes |= FILE_ATTRIBUTE_TEMPORARY;
                break;
            case 10:
                dwFileAttributes |= FILE_ATTRIBUTE_VIRTUAL;
                break;
            default:
                break;
        }
    } while(iBuff != 0);
    if(dwFileAttributes == 0UL)
    {
        cout << "You didn't select any attributes, so the attributes set to normal\n";
        dwFileAttributes = FILE_ATTRIBUTE_NORMAL;
    }
    if(SetFileAttributesA(sFileName.c_str(), dwFileAttributes))
    {
        cout << "The file attributes were successfully set!\n";
    }
    else
    {
        cout << "Setting new file attributes failed! Error code is " << GetLastError() << '\n';
    }
}

void GetFileTime_()
{
    FILETIME    ftCreationTime,
                ftLastAccessTime,
                ftLastWriteTime,
                ftLocalFileTime;
    SYSTEMTIME stBuff;
    string sFileName;

    cout << "Input the name of file, which time you want to set: ";
    cin >> sFileName;

    HANDLE hFile = CreateFile(  sFileName.c_str(), 
                                    GENERIC_READ | GENERIC_WRITE, 
                                    0,
                                    NULL,
                                    OPEN_EXISTING,
                                    FILE_ATTRIBUTE_NORMAL,
                                    NULL);
    if(hFile != INVALID_HANDLE_VALUE)
    {
        GetFileTime(hFile, &ftCreationTime, &ftLastAccessTime, &ftLastWriteTime);
        FileTimeToLocalFileTime(&ftCreationTime, &ftLocalFileTime);
        FileTimeToSystemTime(&ftLocalFileTime, &stBuff);
        cout << "\nCreation time is " << GetFullDate(&stBuff) << '\n';

        FileTimeToLocalFileTime(&ftLastAccessTime, &ftLocalFileTime);
        FileTimeToSystemTime(&ftLocalFileTime, &stBuff);
        cout << "Last access time is " << GetFullDate(&stBuff) << '\n';

        FileTimeToLocalFileTime(&ftLastWriteTime, &ftLocalFileTime);
        FileTimeToSystemTime(&ftLocalFileTime, &stBuff);
        cout << "Last write time is " << GetFullDate(&stBuff) << '\n';
        CloseHandle(hFile);
    }
    else
    {
        cout << "File opening failed! Error code is " << GetLastError() << '\n';
    }
}

void SetFileTime_()
{
    FILETIME    ftCreationTime,
                ftLastAccessTime,
                ftLastWriteTime;
    string sFileName;

    cout << "Input the name of file, which time you want to set: ";
    cin >> sFileName;

    HANDLE hFile = CreateFile(  sFileName.c_str(), 
                                    GENERIC_READ | GENERIC_WRITE, 
                                    0,
                                    NULL,
                                    OPEN_EXISTING,
                                    FILE_ATTRIBUTE_NORMAL,
                                    NULL);
    if(hFile != INVALID_HANDLE_VALUE)
    {
        cout << "\n===== Creation time =====\n";
        FillFileTime(&ftCreationTime);
        cout << "\n===== Last access time =====\n";
        FillFileTime(&ftLastAccessTime);
        cout << "\n===== Last write time =====\n";
        FillFileTime(&ftLastWriteTime);
        if(SetFileTime(hFile, &ftCreationTime, &ftLastAccessTime, &ftLastWriteTime))
        {
            cout << "File time was successfully changed!\n";
        }
        else
        {
            cout << "File time changing failed! Error code is " << GetLastError() << '\n';
        }
        CloseHandle(hFile);
    }
    else
    {
        cout << "File opening failed! Error code is " << GetLastError() << '\n';
    }
}

/**
 * ??????????????, ?????????????? ?????????????????????? ?????????????????? ?????????? ?? ???????????? ???????? DAY/MONTH/YEAR HOURS:MINUTES:SECONDS
 */
string GetFullDate(LPSYSTEMTIME systemTime)
{
    stringstream ssFullDate;

    if(systemTime->wDay > 9)
        ssFullDate << systemTime->wDay << '/';
    else
        ssFullDate << "0" << systemTime->wDay << '/';
    if(systemTime->wMonth > 9)
        ssFullDate << systemTime->wMonth << '/';
    else
        ssFullDate << "0" << systemTime->wMonth << '/';

    ssFullDate << systemTime->wYear << ' ';
    
    if(systemTime->wHour > 9)
        ssFullDate << systemTime->wHour << ':';
    else
        ssFullDate << "0" << systemTime->wHour << ':';
    if(systemTime->wMinute > 9)
        ssFullDate << systemTime->wMinute << ':';
    else
        ssFullDate << "0" << systemTime->wMinute << ':';
    if(systemTime->wSecond > 9)
        ssFullDate << systemTime->wSecond;
    else
        ssFullDate << "0" << systemTime->wSecond;
    return ssFullDate.str();
}

/**
 * ??????????????, ?? ?????????????? ???????????????????????? ???????????? ???????? ???????? ?? ??????????, ?????????????? ?????????? ???????????????? ?? lpFileTime
 */
void FillFileTime(LPFILETIME lpFileTime)
{
    string  sBuff,
            sBuffPart;
    SYSTEMTIME stBuff;
    FILETIME ftLocalFileTime;
    WORD wTime;
    bool incorrectInput;
    do
    {
        cout << "Input the date (day/month/year): ";
        incorrectInput = false;
        cin >> sBuff;
        for(int i = 0; i < 2; i++)
        {
            sBuffPart = sBuff.substr(0, sBuff.find("/"));
            sBuff = sBuff.substr(sBuff.find("/")+1, sBuff.length());
            wTime = (WORD)atoi(sBuffPart.c_str());
            if(i == 0)
            {
                if(wTime < 0 || wTime > 31)
                {
                    incorrectInput = true;
                    break;
                }
                stBuff.wDay = wTime;
            }
            else if(i == 1)
            {
                if(wTime < 0 || wTime > 12)
                {
                    incorrectInput = true;
                    break;
                }
                stBuff.wMonth = wTime;
            }
        }
        if(!incorrectInput)
        {
            wTime = (WORD)atoi(sBuff.c_str());
            if(wTime < 1990 || wTime > 2021)
            {
                incorrectInput = true;
            }
            else
                stBuff.wYear = wTime;
        }
        if(incorrectInput)
            cout << "You inputted the wrong date or time, try again!\n";
    } while(incorrectInput);
    do
    {
        cout << "Input the time (hour:minute:seconds): ";
        incorrectInput = false;
        cin >> sBuff;
        for(int i = 0; i < 2; i++)
        {
            sBuffPart = sBuff.substr(0, sBuff.find(":"));
            sBuff = sBuff.substr(sBuff.find(":")+1, sBuff.length());
            wTime = (WORD)atoi(sBuffPart.c_str());
            if(i == 0)
            {
                if(wTime < 0 || wTime > 23)
                {
                    incorrectInput = true;
                    break;
                }
                stBuff.wHour = wTime;
            }
            else if(i == 1)
            {
                if(wTime < 0 || wTime > 59)
                {
                    incorrectInput = true;
                    break;
                }
                stBuff.wMinute = wTime;
            }
        }
        if(!incorrectInput)
        {
            wTime = (WORD)atoi(sBuff.c_str());
            if(wTime < 0 || wTime > 59)
            {
                incorrectInput = true;
            }
            else
                stBuff.wSecond = wTime;
        }
        if(incorrectInput)
            cout << "You inputted the wrong date or time, try again!\n";
    } while(incorrectInput);
    SystemTimeToFileTime(&stBuff, &ftLocalFileTime);
    LocalFileTimeToFileTime(&ftLocalFileTime, lpFileTime);
}