#include <iostream>
#include <windows.h>

#ifdef _UNICODE
#define STRING std::wstring
#else
#define STRING std::string
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
            std::cout << "Written data at address " << lpMapView << "\n";
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