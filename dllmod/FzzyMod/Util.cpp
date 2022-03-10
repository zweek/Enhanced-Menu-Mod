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

    void WriteBytes(void* ptr, int byte, int size) {
        DWORD curProtection;
        VirtualProtect(ptr, size, PAGE_EXECUTE_READWRITE, &curProtection);
        memset(ptr, byte, size);
        DWORD temp;
        VirtualProtect(ptr, size, curProtection, &temp);
    }

    void WriteBytes(uintptr_t ptr, std::vector<int> bytes) {
        for (int i = 0; i < bytes.size(); i++) {
            int b = bytes[i];
            void* current = (void*)(ptr + i);
            DWORD curProtection;
            VirtualProtect(current, 1, PAGE_EXECUTE_READWRITE, &curProtection);
            memset(current, b, 1);
            DWORD temp;
            VirtualProtect(current, 1, curProtection, &temp);
        }
    }

    bool SRMM_GetSetting(int pos) {
        // voice_forcemicrecord convar
        uintptr_t srmmSettingBase = (uintptr_t)GetModuleHandle("engine.dll") + 0x8A159C;
        uintptr_t srmmSetting = *(uintptr_t*)srmmSettingBase;
        // check for a 1 in the binary of srmmSetting at pos
        return (srmmSetting & ((unsigned long long)1 << pos)) > 0;
    }

    bool FindDMAAddy(uintptr_t ptr, std::vector<unsigned int> offsets, uintptr_t& addr)
    {
        addr = ptr;
        MEMORY_BASIC_INFORMATION mbi;
        for (unsigned int i = 0; i < offsets.size(); ++i)
        {
            VirtualQuery((LPCVOID)addr, &mbi, sizeof(MEMORY_BASIC_INFORMATION));
            if (mbi.Protect != 0x4) return false;
            addr = *(uintptr_t*)addr;
            addr += offsets[i];
        }
        return true;
    }

    uintptr_t FindDMAAddy(uintptr_t ptr, std::vector<unsigned int> offsets)
    {
        uintptr_t addr = ptr;
        MEMORY_BASIC_INFORMATION mbi;
        for (unsigned int i = 0; i < offsets.size(); ++i)
        {
            VirtualQuery((LPCVOID)addr, &mbi, sizeof(MEMORY_BASIC_INFORMATION));
            if (mbi.Protect != 0x4) return false;
            addr = *(uintptr_t*)addr;
            addr += offsets[i];
        }
        return addr;
    }

} // namespace Util