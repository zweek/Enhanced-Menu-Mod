#include "Util.h"
#include <Windows.h>
#include <string>
#include <vector>
#include "tlhelp32.h"

namespace Util
{

    // This will convert some data like "Hello World" to "48 65 6C 6C 6F 20 57 6F 72 6C 64"
    // Taken mostly from https://stackoverflow.com/a/3382894
    std::string DataToHex(const char* input, size_t len)
    {
        static const char* const lut = "0123456789ABCDEF";

        std::string output;
        output.reserve(2 * len);
        for (size_t i = 0; i < len; i++)
        {
            const unsigned char c = input[i];
            output.push_back(lut[c >> 4]);
            output.push_back(lut[c & 15]);
        }

        return output;
    }

    void FindAndReplaceAll(std::string& data, const std::string& search, const std::string& replace)
    {
        size_t pos = data.find(search);
        while (pos != std::string::npos)
        {
            data.replace(pos, search.size(), replace);
            pos = data.find(search, pos + replace.size());
        }
    }

    void* ResolveLibraryExport(const char* module, const char* exportName)
    {
        HMODULE hModule = GetModuleHandle(module);
        if (!hModule)
        {
            //throw std::runtime_error(fmt::sprintf("GetModuleHandle failed for %s (Error = 0x%X)", module, GetLastError()));
        }

        void* exportPtr = GetProcAddress(hModule, exportName);
        if (!exportPtr)
        {
            //throw std::runtime_error(
              //  fmt::sprintf("GetProcAddress failed for %s (Error = 0x%X)", exportName, GetLastError()));
        }

        return exportPtr;
    }

    void FixSlashes(char* pname, char separator)
    {
        while (*pname)
        {
            if (*pname == '\\' || *pname == '/')
            {
                *pname = separator;
            }
            pname++;
        }
    }
    
    bool IsMemoryReadable(const uintptr_t nAddress, const size_t nSize)
    {
        static SYSTEM_INFO sysInfo;
        if (!sysInfo.dwPageSize)
            GetSystemInfo(&sysInfo);

        MEMORY_BASIC_INFORMATION memInfo;
        if (!VirtualQuery(reinterpret_cast<LPCVOID>(nAddress), &memInfo, sizeof(memInfo)))
            return false;

        return memInfo.RegionSize >= nSize && memInfo.State & MEM_COMMIT && !(memInfo.Protect & PAGE_NOACCESS);
    }

} // namespace Util