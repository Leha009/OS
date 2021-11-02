#include <iostream>
#include "useful.h"
#include <windows.h>
#include <math.h>
#include <vector>

#ifdef _UNICODE
#define STRING std::wstring
#define STRSIZE sizeof(WCHAR)
#else
#define STRING std::string
#define STRSIZE sizeof(CHAR)
#endif

int SelectMenu();
DWORD GetPageSize();
DWORD GetMemoryStateEx(LPCVOID, PSIZE_T = NULL, PDWORD = NULL);
void GetSystemInfo_();
void GetMemoryStatus(bool);
bool GetPartMemoryStatus(LPCVOID, bool);
void ReserveVirtualMemory(int);
void ReserveCommitVirtualMemory(int);
void InputDataToMemory(int);
void ProtectVirtualMemory(int);
void FreeVirtualMemory(int);

void ShowAllAddresses(bool);
void PushUpdateVector(LPVOID);
void RemoveFromVector(LPVOID);
bool AddressInVector(LPVOID);
bool FreeAllAddresses();

std::vector<LPVOID> vVirtualMemories;

int main(int argc, char* argv[])  //Потом убрать надо
{
    int iMenuItem;
    int flags = 0;  //See useful.h
    for(int i = 0; i < argc; ++i)
    {
        if(!strcmp(argv[i], "-cb")) //Конвертация байтов в кб/мб/гб/тб
            flags |= FLAG_CONVERTBYTES;
        else if(!strcmp(argv[i], "-hi")) //Помощь в вводе (вывод констант там, где надо)
            flags |= FLAG_HELPINPUT;
        else if(!strcmp(argv[i], "-sa")) //Вывод адрессов, которые резервировали через VirtualAlloc
            flags |= FLAG_SHOWADDRESSES;
    }
    do
    {
        system("cls");
        iMenuItem = SelectMenu();
        if(iMenuItem == 1)
        {
            system("cls");
            GetSystemInfo_();
            system("pause");
        }
        else if(iMenuItem == 2)
        {
            system("cls");
            GetMemoryStatus(ConvertBytes(flags));
            system("pause");
        }
        else if(iMenuItem == 3)
        {
            system("cls");
            LPVOID lpAddress;
            if(ShowAllAddressesInProccess(flags))
                ShowAllAddresses(ConvertBytes(flags));
            std::cout << "Input the pointer (0x...) of desired part of memory: ";
            std::cin >> lpAddress;
            if(!GetPartMemoryStatus(lpAddress, ConvertBytes(flags)))
            {
                std::cout << "Failed to get info. Error code is " << GetLastError() << '\n';
            }
            system("pause");
        }
        else if(iMenuItem == 4)
        {
            system("cls");
            ReserveVirtualMemory(flags);
            system("pause");
        }
        else if(iMenuItem == 5)
        {
            system("cls");
            ReserveCommitVirtualMemory(flags);
            system("pause");
        }
        else if(iMenuItem == 6)
        {
            system("cls");
            InputDataToMemory(flags);
            system("pause");
        }
        else if(iMenuItem == 7)
        {
            system("cls");
            if(vVirtualMemories.size() > 0)
                ProtectVirtualMemory(flags);
            else
                std::cout << "No addresses allocated yet! Allocate first (options 4, 5)\n";
            system("pause");
        }
        else if(iMenuItem == 8)
        {
            system("cls");
            if(vVirtualMemories.size() > 0)
                FreeVirtualMemory(flags);
            else
                std::cout << "No addresses allocated yet! Allocate first (options 4, 5)\n";
            system("pause");
        }
        else if(iMenuItem == 9)
        {
            system("cls");
            ShowAllAddresses(ConvertBytes(flags));
            system("pause");
        }
    } while(iMenuItem != 0);
    return 0;
}

int SelectMenu()
{
    int iMenuItem;
    std::cout << "Select a menu item:\n";
    std::cout << "1 - Get system info\n";
    std::cout << "2 - Get memory status\n";
    std::cout << "3 - Get status of a specific part of memory\n";
    std::cout << "4 - Reserve virtual memory\n";
    std::cout << "5 - Reserve virtual memory and (or) commit physical memory\n";
    std::cout << "6 - Input data to memory\n";
    std::cout << "7 - Set protection flags for virtual memory\n";
    std::cout << "8 - Free virtual memory\n";
    std::cout << "9 - Show all addresses of the virtual memory in this process\n";
    std::cout << "0 - Exit\n";
    do
    {
        std::cin >> iMenuItem;
        if(iMenuItem < 0 || iMenuItem > 9) 
            std::cout << "There is no such menu item! Try again.\n";
    } while(iMenuItem < 0 || iMenuItem > 9);
    return iMenuItem;
}

DWORD GetPageSize()
{
    SYSTEM_INFO sInfo;
    GetSystemInfo(&sInfo);
    return sInfo.dwPageSize;
}

DWORD GetMemoryStateEx(LPCVOID lpAddress, PSIZE_T RegionSize, PDWORD dwProtect)
{
    MEMORY_BASIC_INFORMATION memoryInfo;
    if(VirtualQuery(lpAddress, &memoryInfo, sizeof(memoryInfo)) == ERROR_INVALID_PARAMETER)
    {
        return 0UL;
    }
    if(RegionSize != NULL)
        *RegionSize = memoryInfo.RegionSize;
    if(dwProtect != NULL)
        *dwProtect = memoryInfo.Protect;
    return memoryInfo.State;
}

//https://tinyurl.com/fsjr5zu4
void GetSystemInfo_()
{
    SYSTEM_INFO sInfo;
    GetSystemInfo(&sInfo);
    std::cout << "System info:\n";
    std::cout << "Processor architecture: ";
    switch(sInfo.wProcessorArchitecture)
    {
        case PROCESSOR_ARCHITECTURE_AMD64:
            std::cout << "x64 (AMD or Intel)\n";
            break;
        case PROCESSOR_ARCHITECTURE_ARM:
            std::cout << "ARM\n";
            break;
        case PROCESSOR_ARCHITECTURE_IA64:
            std::cout << "Intel Itanium-based\n";
            break;
        case PROCESSOR_ARCHITECTURE_INTEL:
            std::cout << "x86\n";
            break;
        case PROCESSOR_ARCHITECTURE_UNKNOWN:
            std::cout << "Unknown\n";
            break;
        default:
            std::cout << "ARM64?\n";
    }
    std::cout << "Page size: " << sInfo.dwPageSize << '\n';
    std::cout << "OEM ID: " << sInfo.dwOemId << '\n';
    std::cout << "Minimum application address: " << sInfo.lpMinimumApplicationAddress << '\n';
    std::cout << "Maximum application address: " << sInfo.lpMaximumApplicationAddress << '\n';
    std::cout << "A mask representing the set of processors configured into the system:\n";
    for(int i = 0; i < 32; ++i)
    {
        std::cout << ((sInfo.dwActiveProcessorMask >> i) & 1) << ' ';
    }
    std::cout << "\nThe number of logical processors in the current group: " << sInfo.dwNumberOfProcessors << '\n';
    std::cout << "The granularity for the starting address at which virtual memory can be allocated: " << sInfo.dwAllocationGranularity << '\n';
    std::cout << "The architecture-dependent processor level: " << sInfo.wProcessorLevel << '\n';
    //Processor features: https://tinyurl.com/6r8k392d
    std::cout << "Processor features:\n";
    if(IsProcessorFeaturePresent(PF_ARM_64BIT_LOADSTORE_ATOMIC)) 
        std::cout << "\tThe 64-bit load/store atomic instructions are available\n";
    if(IsProcessorFeaturePresent(PF_ARM_DIVIDE_INSTRUCTION_AVAILABLE)) 
        std::cout << "\tThe divide instructions are available\n";
    if(IsProcessorFeaturePresent(PF_ARM_EXTERNAL_CACHE_AVAILABLE)) 
        std::cout << "\tThe external cache is available\n";
    if(IsProcessorFeaturePresent(PF_ARM_FMAC_INSTRUCTIONS_AVAILABLE)) 
        std::cout << "\tThe floating-point multiply-accumulate instruction is available\n";
    if(IsProcessorFeaturePresent(PF_ARM_VFP_32_REGISTERS_AVAILABLE)) 
        std::cout << "\tThe VFP/Neon: 32 x 64bit register bank is present\n";
    if(IsProcessorFeaturePresent(PF_3DNOW_INSTRUCTIONS_AVAILABLE)) 
        std::cout << "\tThe 3D-Now instruction set is available\n";
    if(IsProcessorFeaturePresent(PF_CHANNELS_ENABLED)) 
        std::cout << "\tThe processor channels are enabled\n";
    if(IsProcessorFeaturePresent(PF_COMPARE_EXCHANGE_DOUBLE)) 
        std::cout << "\tThe atomic compare and exchange operation (cmpxchg) is available\n";
    if(IsProcessorFeaturePresent(PF_COMPARE_EXCHANGE128)) 
        std::cout << "\tThe atomic compare and exchange 128-bit operation (cmpxchg16b) is available\n";
    if(IsProcessorFeaturePresent(PF_COMPARE64_EXCHANGE128)) 
        std::cout << "\tThe atomic compare 64 and exchange 128-bit operation (cmp8xchg16) is available\n";
    if(IsProcessorFeaturePresent(PF_FASTFAIL_AVAILABLE)) 
        std::cout << "\t_fastfail() is available\n";
    if(IsProcessorFeaturePresent(PF_FLOATING_POINT_EMULATED)) 
        std::cout << "\tFloating-point operations are emulated using a software emulator\n";
    if(IsProcessorFeaturePresent(PF_FLOATING_POINT_PRECISION_ERRATA)) 
        std::cout << "\tOn a Pentium, a floating-point precision error can occur in rare circumstances\n";
    if(IsProcessorFeaturePresent(PF_MMX_INSTRUCTIONS_AVAILABLE)) 
        std::cout << "\tThe MMX instruction set is available\n";
    if(IsProcessorFeaturePresent(PF_NX_ENABLED)) 
        std::cout << "\tData execution prevention is enabled\n";
    if(IsProcessorFeaturePresent(PF_PAE_ENABLED)) 
        std::cout << "\tThe processor is PAE-enabled\n";
    if(IsProcessorFeaturePresent(PF_RDTSC_INSTRUCTION_AVAILABLE)) 
        std::cout << "\tThe RDTSC instruction is available\n";
    if(IsProcessorFeaturePresent(PF_RDWRFSGSBASE_AVAILABLE)) 
        std::cout << "\tRDFSBASE, RDGSBASE, WRFSBASE, and WRGSBASE instructions are available\n";
    if(IsProcessorFeaturePresent(PF_SECOND_LEVEL_ADDRESS_TRANSLATION)) 
        std::cout << "\tSecond Level Address Translation is supported by the hardware\n";
    if(IsProcessorFeaturePresent(PF_SSE3_INSTRUCTIONS_AVAILABLE)) 
        std::cout << "\tThe SSE3 instruction set is available\n";
    if(IsProcessorFeaturePresent(PF_VIRT_FIRMWARE_ENABLED)) 
        std::cout << "\tVirtualization is enabled in the firmware and made available by the operating system\n";
    if(IsProcessorFeaturePresent(PF_XMMI_INSTRUCTIONS_AVAILABLE)) 
        std::cout << "\tThe SSE instruction set is available\n";
    if(IsProcessorFeaturePresent(PF_XMMI64_INSTRUCTIONS_AVAILABLE)) 
        std::cout << "\tThe SSE2 instruction set is available\n";
    if(IsProcessorFeaturePresent(PF_XSAVE_ENABLED)) 
        std::cout << "\tThe processor implements the XSAVE and XRSTOR instructions\n";
    //if(IsProcessorFeaturePresent(PF_ARM_V8_INSTRUCTIONS_AVAILABLE)) 
    if(IsProcessorFeaturePresent(29)) 
        std::cout << "\tThis ARM processor implements the the ARM v8 instructions set\n";
    //if(IsProcessorFeaturePresent(PF_ARM_V8_CRYPTO_INSTRUCTIONS_AVAILABLE)) 
    if(IsProcessorFeaturePresent(30)) 
        std::cout << "\tThis ARM processor implements the ARM v8 extra cryptographic instructions (i.e. AES, SHA1 and SHA2)\n";
    //if(IsProcessorFeaturePresent(PF_ARM_V8_CRC32_INSTRUCTIONS_AVAILABLE)) 
    if(IsProcessorFeaturePresent(31)) 
        std::cout << "\tThis ARM processor implements the ARM v8 extra CRC32 instructions\n";
    //if(IsProcessorFeaturePresent(PF_ARM_V81_ATOMIC_INSTRUCTIONS_AVAILABLE)) 
    if(IsProcessorFeaturePresent(34)) 
        std::cout << "\tThis ARM processor implements the ARM v8.1 atomic instructions (e.g. CAS, SWP)\n";
    //Processor features END
    std::cout << "The architecture-dependent processor revision: " << std::hex << sInfo.wProcessorRevision << std::dec << '\n';
}

//https://tinyurl.com/fn8rac99
void GetMemoryStatus(bool bConvertToMaximum)
{
    MEMORYSTATUS memoryStatus;
    GlobalMemoryStatus(&memoryStatus);
    //std::cout << "The size of the MEMORYSTATUS data structure: " << memoryStatus.dwLength << '\n';
    std::cout << "The approximate percentage of physical memory that is in use: " << memoryStatus.dwMemoryLoad << '\n';
    std::cout << "The amount of actual physical memory: ";
    if(bConvertToMaximum)
        std::cout << ConvertBytesToMaximum(memoryStatus.dwTotalPhys) << '\n';
    else
        std::cout << memoryStatus.dwTotalPhys << " B\n";
    std::cout << "The amount of physical memory currently available: ";
    if(bConvertToMaximum)
        std::cout << ConvertBytesToMaximum(memoryStatus.dwAvailPhys) << '\n';
    else
        std::cout << memoryStatus.dwAvailPhys << " B\n";
    std::cout << "The current size of the committed memory limit: ";
    if(bConvertToMaximum)
        std::cout << ConvertBytesToMaximum(memoryStatus.dwTotalPageFile) << '\n';
    else
        std::cout << memoryStatus.dwTotalPageFile << " B\n";
    std::cout << "The maximum amount of memory the current process can commit: ";
    if(bConvertToMaximum)
        std::cout << ConvertBytesToMaximum(memoryStatus.dwAvailPageFile) << '\n';
    else
        std::cout << memoryStatus.dwAvailPageFile << " B\n";
    std::cout << "The size of the user-mode portion of the virtual address space of the calling process: ";
    if(bConvertToMaximum)
        std::cout << ConvertBytesToMaximum(memoryStatus.dwTotalVirtual) << '\n';
    else
        std::cout << memoryStatus.dwTotalVirtual << " B\n";
    std::cout << "The amount of unreserved and uncommitted memory currently in the user-mode portion of ";
    std::cout << "\n\tthe virtual address space of the calling process: ";
    if(bConvertToMaximum)
        std::cout << ConvertBytesToMaximum(memoryStatus.dwAvailVirtual) << '\n';
    else
        std::cout << memoryStatus.dwAvailVirtual << " B\n";
}

/**
 * Get status of some part of the memory
 * 
 * @param LPCVOID lpAddress - address of the memory
 * @param bool convertBytes - flag if program should convert bytes to something more
 * 
 * @see https://tinyurl.com/udvw5uma
 * 
 * @return true if succeed, false otherwise
 */
bool GetPartMemoryStatus(LPCVOID lpAddress, bool convertBytes)
{
    MEMORY_BASIC_INFORMATION memoryInfo;
    if(VirtualQuery(lpAddress, &memoryInfo, sizeof(memoryInfo)) == ERROR_INVALID_PARAMETER)
    {
        return false;
    }
    
    std::cout << "A pointer to the base address of the region of pages: " << memoryInfo.BaseAddress << '\n';
    std::cout << "A pointer to the base address of a range of pages allocated by the VirtualAlloc function: " << memoryInfo.AllocationBase << '\n';
    std::cout << "The memory protection option when the region was initially allocated: " << memoryInfo.AllocationProtect << '\n';
    //Тут, если что, можно вывести в виде текста: https://tinyurl.com/d28h342b
    std::cout << "The size of the region in which all pages have identical attributes: ";
    if(convertBytes)
        std::cout << ConvertBytesToMaximum(memoryInfo.RegionSize) << '\n';
    else
        std::cout << memoryInfo.RegionSize << " B\n";

    std::cout << "Memory state: ";
    if(memoryInfo.State == MEM_COMMIT)
        std::cout << "committed pages for which physical storage has been allocated, either in memory or in the paging file on disk\n";
    else if(memoryInfo.State == MEM_FREE)
        std::cout << "free pages not accessible to the calling process and available to be allocated\n";
    else if(memoryInfo.State == MEM_RESERVE)
        std::cout << "reserved pages where a range of the process's virtual address space is reserved without any physical storage being allocated\n";
    else
        std::cout << "Unknown\n";

    std::cout << "The access protection of the pages in the region: 0x" << std::hex << memoryInfo.Protect << std::dec << '\n';
    //Тут, если что, можно вывести в виде текста: https://tinyurl.com/d28h342b
    std::cout << "The type of pages in the region: ";
    if(memoryInfo.Type == MEM_IMAGE)
        std::cout << "the memory pages within the region are mapped into the view of an image section\n";
    else if(memoryInfo.Type == MEM_MAPPED)
        std::cout << "the memory pages within the region are mapped into the view of a section\n";
    else if(memoryInfo.Type == MEM_PRIVATE)
        std::cout << "the memory pages within the region are private (that is, not shared by other processes)\n";
    else
        std::cout << "Unknown\n";
    return true;
}

void ReserveVirtualMemory(int iFlags)
{
    LPVOID  lpAddress = NULL,
            lpResult = NULL;
    DWORDLONG dwSize;
    DWORD   flAllocationType,
            flProtect,
            dwPageSize = GetPageSize();
    if(ShowAllAddressesInProccess(iFlags))
        ShowAllAddresses(ConvertBytes(iFlags));
    std::cout << "Input the pointer (0x...) of the beginning of the region to reserve memory (input 0 for auto mode): ";
    std::cin >> lpAddress;
    std::cout << "\nInput the number of the region in bytes: " << dwPageSize << '*';
    do
    {
        std::cin >> dwSize;
        if(dwSize < 1ULL)
            std::cout << "The number must be more than 0!\n";
    } while(dwSize < 1ULL);
    dwSize *= dwPageSize;
    lpResult = VirtualAlloc(lpAddress, dwSize, MEM_RESERVE, PAGE_READWRITE);
    if(lpResult == NULL)
    {
        std::cout << "Failed to reserve virtual memory. Error code is " << GetLastError() << '\n';
    }
    else
    {
        std::cout << "The base address of the allocated region of pages: " << lpResult << '\n';
        PushUpdateVector(lpResult);
    }
}

void ReserveCommitVirtualMemory(int iFlags)
{
    LPVOID  lpAddress = NULL,
            lpResult = NULL;
    DWORDLONG dwSize;
    DWORD   flAllocationType,
            flProtect,
            dwMemoryState,
            dwPageSize = GetPageSize();
    bool    bStop = false;
    if(ShowAllAddressesInProccess(iFlags))
        ShowAllAddresses(ConvertBytes(iFlags));
    std::cout << "Input the pointer (0x...) of the beginning of the region to reserve and (or) commit memory (input 0 for auto mode): ";
    do
    {
        std::cin >> std::hex >> lpAddress >> std::dec;
        dwMemoryState = GetMemoryStateEx((LPVOID)lpAddress, &dwSize);
        if(dwMemoryState == 0UL)
        {
            std::cout << "You inputted wrong address, input the new one!\n";
        }
        else if(dwMemoryState & MEM_COMMIT)
        {
            std::cout << "This address is already commited, input the new one!\n";
        }
        else if(dwMemoryState == MEM_RESERVE)
        {
            std::cout << "This address will be commited!\n";
            flAllocationType = MEM_COMMIT;
            bStop = true;
        }
        else
        {
            flAllocationType = MEM_RESERVE | MEM_COMMIT;
            bStop = true;
        }
    } while(!bStop);
    if(flAllocationType == (MEM_RESERVE | MEM_COMMIT))
    {
        std::cout << "\nInput the size of the region in bytes: " << dwPageSize << '*';
        do
        {
            std::cin >> dwSize;
            if(dwSize < 1ULL)
                std::cout << "The number must be more than 0!\n";
        } while(dwSize < 1ULL);
        dwSize *= 4096UL;
    }
    lpResult = VirtualAlloc(lpAddress, dwSize, flAllocationType, PAGE_READWRITE);
    if(lpResult == NULL)
    {
        std::cout << "Failed to reserve and (or) commit virtual memory. Error code is " << GetLastError() << '\n';
    }
    else
    {
        std::cout << "The base address of the allocated region of pages: " << lpResult << '\n';
        PushUpdateVector(lpResult);
    }
}

void InputDataToMemory(int iFlags)
{
    STRING sData;
    LPVOID lpAddress;
    bool bIsAddressInVector = false;
    if(ShowAllAddressesInProccess(iFlags))
        ShowAllAddresses(ConvertBytes(iFlags));
    std::cout << "Input the pointer (0x...) of the beginning of the region of pages of some virtual memory: ";
    std::cin >> std::hex >> lpAddress >> std::dec;
    std::cout << "Input the string to write to memory:\n";
    std::cin.ignore();
    std::getline(std::cin, sData);
    if(lpAddress != NULL)
    {
        if(sData.length() < 1)
        {
            std::cout << "You didn't input the string, nothing to write to memory!\n";
        }
        else
        {
            DWORD dwProtect;
            GetMemoryStateEx((LPVOID)lpAddress, NULL, &dwProtect);
            if(dwProtect & (PAGE_EXECUTE_READWRITE | PAGE_EXECUTE_WRITECOPY | PAGE_READWRITE | PAGE_WRITECOPY))
            {
                CopyMemory(lpAddress, sData.c_str(), sData.length()*STRSIZE);
                std::cout << std::hex << lpAddress << std::dec << " address filled with this your string:\n";
                char* spAddress = (char*)lpAddress;
                for(size_t i = 0; i < sData.length(); ++i)
                    std::cout << spAddress[i];
                std::cout << '\n';
            }
            else
            {
                std::cout << "You can't write any data using this address!\n";
            }
        }
    }
}

void ProtectVirtualMemory(int iFlags)
{
    LPVOID lpAddress = NULL;
    DWORDLONG dwSize;
    DWORD flNewProtect,
          flOldProtect;
    bool    bIsAddressInVector = false;
    if(ShowAllAddressesInProccess(iFlags))
        ShowAllAddresses(ConvertBytes(iFlags));
    std::cout << "Input the pointer (0x...) of the beginning of the region of pages of some virtual memory: ";
    do
    {
        std::cin >> std::hex >> lpAddress >> std::dec;
        bIsAddressInVector = AddressInVector(lpAddress);
        if(!bIsAddressInVector)
            std::cout << "This address isn't in list of virtual addresses of this process! Input again\n";
    } while(!bIsAddressInVector);
    GetMemoryStateEx(lpAddress, &dwSize);
    std::cout << "Input the new protection flag (0x...): ";
    if(HelpInput(iFlags))
    {
        std::cout << "\nAvailable flags:"
        << "\n\tPAGE_EXECUTE - 0x10 | Enables execute access to the COMMITTED region of pages"
        << "\n\tPAGE_EXECUTE_READ - 0x20 | Enables execute or read-only access to the COMMITTED region of pages"
        << "\n\tPAGE_EXECUTE_READWRITE - 0x40 | Enables execute, read-only, or read/write access to the COMMITTED region of pages"
        << "\n\tPAGE_NOACCESS - 0x01 | Disables all access to the COMMITTED region of pages"
        << "\n\tPAGE_READONLY - 0x02 | Enables read-only access to the COMMITTED region of pages"
        << "\n\tPAGE_READWRITE - 0x04 | Enables read-only or read/write access to the COMMITTED region of pages"
        << "\nInput the new protection flag here: ";
    }
    std::cin >> std::hex >> flNewProtect >> std::dec;
    if(!VirtualProtect(lpAddress, dwSize, flNewProtect, &flOldProtect))
    {
        std::cout << "Failed to set a new protect flag. Error code is " << GetLastError() << '\n';
    }
    else
    {
        std::cout << "You changed protection flag from 0x" << std::hex << flOldProtect << " to 0x" << flNewProtect
        << "\nChanged address: " << lpAddress << std::dec << '\n';
    }
}

void FreeVirtualMemory(int iFlags)
{
    LPVOID lpAddress;
    SIZE_T dwSize;
    DWORD dwFreeType,
          dwState;
    bool bIsAddressInVector = true,
         bDecommitAndRelease;
    if(ShowAllAddressesInProccess(iFlags))
        ShowAllAddresses(ConvertBytes(iFlags));
    std::cout << "Input the pointer (0x...) of the beginning of the region of pages of some virtual memory (input 0 to free all regions of pages): ";
    do
    {
        std::cin >> std::hex >> lpAddress >> std::dec;
        if(lpAddress != NULL)
        {
            bIsAddressInVector = AddressInVector(lpAddress);
            if(!bIsAddressInVector)
                std::cout << "This address isn't in list of virtual addresses of this process! Input again\n";
        }
    } while(!bIsAddressInVector);
    if(lpAddress == NULL)   //FREE ALL REGIONS
    {
        if(!FreeAllAddresses())
            std::cout << "Failed to free all regioins. Error code is " << GetLastError() << '\n';
        else
            std::cout << "All regions successfully freed!\n";
    }
    else
    {
        dwState = GetMemoryStateEx(lpAddress, &dwSize);
        if(dwState == MEM_COMMIT)
        {
            std::cout << "Do you want to decommit and release this memory? Input 1 for yes, 0 otherwise: ";
            std::cin >> bDecommitAndRelease;
            if(bDecommitAndRelease)
            {
                dwFreeType = MEM_RELEASE;
                dwSize = 0ULL;
            }
            else
                dwFreeType = MEM_DECOMMIT;
        }
        else
        {
            dwFreeType = MEM_DECOMMIT;
        }
        if(!VirtualFree(lpAddress, dwSize, dwFreeType))
        {
            std::cout << "Failed to free virtual memory. Error code is " << GetLastError() << '\n';
        }
        else
        {
            std::cout << "Memory at "<< std::hex << lpAddress << std::dec << " successfully freed!\n";
            if(!(dwFreeType == MEM_DECOMMIT && dwState == MEM_COMMIT))
                RemoveFromVector(lpAddress);
        }
    }
}

    //============== FOR VECTOR ==============//

void ShowAllAddresses(bool bConvertBytes)
{
    if(vVirtualMemories.size() < 1)
    {
        std::cout << "No addresses allocated yet\n";
    }
    else
    {
        DWORD dwState;
        SIZE_T dwSize;
        std::cout << "Allocated addresses in this process:\n";
        for(LPVOID address : vVirtualMemories)
        {
            dwState = GetMemoryStateEx(address, &dwSize);
            std::cout << "Address is " << std::hex << address
            << (dwState == MEM_COMMIT ? " (committed memory)" : " (not committed memory)") << std::dec;
            if(bConvertBytes)
            {
                std::cout << " Size is " << ConvertBytesToMaximum(dwSize) << '\n';
            }
            else
            {
                std::cout << " Size is " << dwSize << " bytes\n";
            }
            
        }
    }
}

void PushUpdateVector(LPVOID lpAddress)
{
    for(LPVOID address : vVirtualMemories)
    {
        if(address == lpAddress)
        {
            return;
        }
    }
    vVirtualMemories.push_back(lpAddress);
}

void RemoveFromVector(LPVOID lpAddress)
{
    for(auto it = vVirtualMemories.begin(); it != vVirtualMemories.end();)
    {
        if(*it == lpAddress)
        {
            it = vVirtualMemories.erase(it);
            return;
        }
        else
        {
            ++it;
        }
    }
}

bool AddressInVector(LPVOID lpAddress)
{
    for(LPVOID address : vVirtualMemories)
    {
        if(address == lpAddress)
        {
            return true;
        }
    }
    return false;
}

bool FreeAllAddresses()
{
    SIZE_T  dwSize;
    DWORD   dwState,
            dwFreeType;
    for(LPVOID lpAddress : vVirtualMemories)
    {
        dwState = GetMemoryStateEx(lpAddress, &dwSize);
        if(dwState == MEM_COMMIT)
        {
            dwFreeType = MEM_RELEASE;
            dwSize = 0UL;
        }
        else
            dwFreeType = MEM_DECOMMIT;
        if(!VirtualFree(lpAddress, dwSize, dwFreeType))
        {
            return false;
        }
    }
    vVirtualMemories.clear();
    return true;
}