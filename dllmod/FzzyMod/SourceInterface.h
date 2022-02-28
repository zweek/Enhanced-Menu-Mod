#pragma once
#include "Util.h"

typedef void* (*CreateInterfaceFn)(const char* pName, int* pReturnCode);

template <typename T> class SourceInterface
{
    T* m_interface;

public:
    SourceInterface(const char* moduleName, const char* interfaceName)
    {
        CreateInterfaceFn createInterface =
            (CreateInterfaceFn)Util::ResolveLibraryExport(moduleName, "CreateInterface");
        m_interface = (T*)createInterface(interfaceName, NULL);
        if (m_interface == nullptr)
        {
            //throw std::runtime_error(
              //  fmt::sprintf("Failed to call CreateInterface for %s in %s", interfaceName, moduleName));
        }
    }

    T* operator->() const
    {
        return m_interface;
    }

    operator T* () const
    {
        return m_interface;
    }
};