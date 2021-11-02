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
    std::cout << "Input the name of the file to create map view: ";
    std::getline(std::cin, sBuff);
    HANDLE hFile = CreateFile(sBuff.c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if(hFile != NULL)
    {
        TCHAR sFileMappingName[MAX_PATH];
        std::cout << "Input the name of the file mapping: ";
        std::cin >> sFileMappingName;
        HANDLE hFileMapping = CreateFileMapping(hFile, NULL, PAGE_READWRITE, 0UL, MAPPING_LOWMAX, sFileMappingName);
        if(hFileMapping != NULL)
        {
            LPVOID lpMapView = MapViewOfFile(hFileMapping, FILE_MAP_ALL_ACCESS, 0UL, 0UL, 0UL);
            if(lpMapView != NULL)
            {
                std::cout << "Input the data to write to the mapping file: ";
                std::cin.ignore();
                std::getline(std::cin, sBuff);
                CopyMemory(lpMapView, sBuff.c_str(), sBuff.length()*STRSIZE);
                std::cout << "Do not press any key until the reader program stops reading!"
                << "\nWhen it's done, press any key!\n";
                system("pause");
                UnmapViewOfFile(lpMapView);
                lpMapView = NULL;
            }
            else
            {
                std::cout << "Failed to map a view of a file mapping. Error code is " << GetLastError() << '\n';
                system("pause");
            }
            CloseHandle(hFileMapping);
        }
        else
        {
            std::cout << "Failed to create file mapping. Error code is " << GetLastError() << '\n';
            system("pause");
        }
        CloseHandle(hFile);
    }
    return 0;
}