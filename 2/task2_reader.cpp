#include <iostream>
#include <windows.h>

#ifdef _UNICODE
#define MAPPING_LOWMAX 512UL    //WCHAR
#define STRING std::wstring
#define STRSIZE sizeof(WCHAR)
#else
#define MAPPING_LOWMAX 256UL    //CHAR
#define STRING std::string
#define STRSIZE sizeof(CHAR)
#endif

int main()
{
    STRING sBuff;
    TCHAR sFileMappingName[MAX_PATH];
    std::cout << "Input the name of the file mapping: ";
    std::cin >> sFileMappingName;
    HANDLE hFileMapping = OpenFileMapping(FILE_MAP_READ, false, sFileMappingName);
    if(hFileMapping != NULL)
    {
        LPVOID lpMapView = MapViewOfFile(hFileMapping, FILE_MAP_READ, 0UL, 0UL, 0UL);
        if(lpMapView != NULL)
        {
            std::cout << "Data in this file mapping:\n";
            std::cout << (TCHAR*)lpMapView << '\n';
            UnmapViewOfFile(lpMapView);
            lpMapView = NULL;
        }
        else
        {
            std::cout << "Failed to map a view of a file mapping. Error code is " << GetLastError() << '\n';
        }
        CloseHandle(hFileMapping);
    }
    else
    {
        std::cout << "Failed to open file mapping. Error code is " << GetLastError() << '\n';
    }
    system("pause");
    return 0;
}