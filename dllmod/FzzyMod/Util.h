#pragma once
#include <Windows.h>
#include <string>
#include <vector>

template <typename T, T, typename U, U> struct MemberWrapper;
template <typename T, T& (*pObjGet)(), typename RT, typename... Args, RT(T::* pF)(Args...)>
struct MemberWrapper<RT(T::*)(Args...), pF, T& (*)(), pObjGet>
{
    /*static RT Call(Args&&... args)
    {
        return ((pObjGet()).*pF)(std::forward<Args>(args)...);
    }*/

    static RT Call(Args... args)
    {
        return ((pObjGet()).*pF)(args...);
    }
};

namespace Util
{
    std::string DataToHex(const char* input, size_t len);

    void FindAndReplaceAll(std::string& data, const std::string& search, const std::string& replace);
    void* ResolveLibraryExport(const char* module, const char* exportName);
    void FixSlashes(char* pname, char separator);
} // namespace Util
