#include <string>

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