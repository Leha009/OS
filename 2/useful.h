#include <string>

#define FLAG_CONVERTBYTES 1
#define FLAG_HELPINPUT 2
#define FLAG_SHOWADDRESSES 4

/*Flags
1 - convert bytes to kb/mb/gb/tb
2 - help input (output contants where it needed)
4 - print all allocated addresses
*/
#define ConvertBytes(flags) (flags & FLAG_CONVERTBYTES) > 0
#define HelpInput(flags) (flags & FLAG_HELPINPUT) > 0
#define ShowAllAddressesInProccess(flags) (flags & FLAG_SHOWADDRESSES) > 0

template <typename T>
std::string ConvertBytesToMaximum(T numberToConvert)
{
    int iFileSizeIndex;
    char sFileSize[5][3] = {
        "B",
        "KB",
        "MB",
        "GB",
        "TB"
    };
    double dNumber = (double)numberToConvert;
    for(iFileSizeIndex = 0; dNumber > 1024.0 && iFileSizeIndex < 4; ++iFileSizeIndex)
    {
        dNumber /= 1024.0;
    }
    std::string result = std::to_string((int)(dNumber)) + "." + std::to_string((int)(dNumber*100) % 100) + " " + sFileSize[iFileSizeIndex];
    return result;
}