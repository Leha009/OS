#include <iostream>
#include "useful.h"
#include <windows.h>
#include <math.h>

int SelectMenu();
void GetSystemInfo_();
void GetMemoryStatus(bool);
bool GetPartMemoryStatus(LPCVOID, bool);
void ReserveVirtualMemoryInput(bool);

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
            std::cout << "Input the pointer of desired part of memory: ";
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
            ReserveVirtualMemoryInput(HelpInput(flags));
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
    std::cout << "0 - Back to the main menu\n";
    do
    {
        std::cin >> iMenuItem;
        if(iMenuItem < 0 || iMenuItem > 4) 
            std::cout << "There is no such menu item! Try again.\n";
    } while(iMenuItem < 0 || iMenuItem > 4);
    return iMenuItem;
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
    std::cout << "A mask representing the set of processors configured into the system: " << sInfo.dwActiveProcessorMask << '\n';
    std::cout << "The number of logical processors in the current group: " << sInfo.dwNumberOfProcessors << '\n';
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
    SIZE_T dwLength;
    if(VirtualQuery(lpAddress, &memoryInfo, dwLength) == ERROR_INVALID_PARAMETER)
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

    std::cout << "The access protection of the pages in the region: " << memoryInfo.Protect << '\n';
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

void ReserveVirtualMemoryInput(bool helpInput)
{
    LPVOID  lpAddress = NULL,
            lpResult = NULL;
    DWORDLONG dwSize;
    DWORD   flAllocationType,
            flProtect;
    std::cout << "Input the pointer of the beginning of the region to reserve memory (input 0 for auto mode): ";
    std::cin >> lpAddress;
    std::cout << "\nInput the size of the region in bytes: ";
    std::cin >> dwSize;
    std::cout << "\nInput the type of memory allocation: ";
    if(helpInput)
    {
        std::cout << "\n\tMEM_COMMIT - 0x00001000 | MEM_RESERVE - 0x00002000 | MEM_RESET - 0x00080000 | MEM_RESET_UNDO - 0x1000000\n";
        std::cout << "Input here: ";
    }
    std::cin >> std::hex >> flAllocationType;
    std::cout << "\nInput the memory protection for the region of pages to be allocated: ";
    if(helpInput)
    {
        std::cout << "\n\tPAGE_EXECUTE - 0x10 | PAGE_EXECUTE_READ - 0x20 | PAGE_EXECUTE_READWRITE - 0x40\n";
        std::cout << "\tPAGE_EXECUTE_WRITECOPY - 0x80 | PAGE_NOACCESS - 0x01 | PAGE_READONLY - 0x02\n";
        std::cout << "\tPAGE_READWRITE - 0x04 | PAGE_WRITECOPY - 0x08\n";
        std::cout << "\tPAGE_TARGETS_INVALID - 0x40000000 | PAGE_TARGETS_NO_UPDATE - 0x40000000\n";
        std::cout << "\n\t\tMODIFIERS\n";
        std::cout << "\tPAGE_GUARD - 0x100 | This value cannot be used with PAGE_NOACCESS\n";
        std::cout << "\tPAGE_NOCACHE - 0x200 | This value cannot be used with PAGE_NOACCESS\n";
        std::cout << "\tPAGE_GUARD - 0x100 | This flag cannot be used with the PAGE_GUARD, PAGE_NOACCESS, or PAGE_WRITECOMBINE flags\n";
        std::cout << "\tPAGE_WRITECOMBINE - 0x400 | This flag cannot be specified with the PAGE_NOACCESS, PAGE_GUARD and PAGE_NOCACHE flags\n";
        std::cout << "Input here: ";
    }
    std::cin >> std::hex >> flProtect;
    lpResult = VirtualAlloc(lpAddress, dwSize, flAllocationType, flProtect);
    if(lpResult == NULL)
    {
        std::cout << "Failed to reserve virtual memory. Error code is " << GetLastError() << '\n';
    }
    else
    {
        std::cout << "The base address of the allocated region of pages: " << lpResult << '\n';
    }
}