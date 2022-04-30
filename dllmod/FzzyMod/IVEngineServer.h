#pragma once
#include <stdarg.h>

enum SpewType_t
{
    SPEW_MESSAGE = 0,
    SPEW_WARNING,
    SPEW_ASSERT,
    SPEW_ERROR,
    SPEW_LOG,
    SPEW_TYPE_COUNT
};

class IVEngineServer
{
public:
    struct VTable
    {
        void* unknown[7];
        __int64 (*PrecacheModel)(IVEngineServer* engineServer, const char* model);
        void* unknown2[167];
        __int64 (*SpewFunc)(IVEngineServer* engineServer, SpewType_t type, const char* format, va_list va);
    };

    VTable* m_vtable;
};