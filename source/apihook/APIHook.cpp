#include "StdAfx.h"
#include "APIHook.h"

#include <fstream>
#include <string>
#include <unordered_map>

//////////////////////////////////////////////////////////////////////////
CAPIHook g_oHook;
CAPIHook::CAPIHook(void)
{
    StackDumper::init();
}

CAPIHook::~CAPIHook(void)
{
    
}

//////////////////////////////////////////////////////////////////////////
HFONT WINAPI _PFN_CreateFontA(
    _In_ int     nHeight,
    _In_ int     nWidth,
    _In_ int     nEscapement,
    _In_ int     nOrientation,
    _In_ int     fnWeight,
    _In_ DWORD   fdwItalic,
    _In_ DWORD   fdwUnderline,
    _In_ DWORD   fdwStrikeOut,
    _In_ DWORD   fdwCharSet,
    _In_ DWORD   fdwOutputPrecision,
    _In_ DWORD   fdwClipPrecision,
    _In_ DWORD   fdwQuality,
    _In_ DWORD   fdwPitchAndFamily,
    _In_ LPCSTR lpszFace
    )
{
    s_PFN_CreateFontA.CallBefore();

    HFONT hfont = s_PFN_CreateFontA.hookfunc(nHeight, nWidth, nEscapement,
        nOrientation, fnWeight, fdwItalic, fdwUnderline,
        fdwStrikeOut, fdwCharSet, fdwOutputPrecision, fdwClipPrecision,
        fdwQuality, fdwPitchAndFamily, lpszFace);

    s_PFN_CreateFontA.CallAfter();

    g_stacks[(INT_PTR)hfont] = new StackDumper;

    return hfont;
}
HFONT WINAPI _PFN_CreateFontW(
    _In_ int     nHeight,
    _In_ int     nWidth,
    _In_ int     nEscapement,
    _In_ int     nOrientation,
    _In_ int     fnWeight,
    _In_ DWORD   fdwItalic,
    _In_ DWORD   fdwUnderline,
    _In_ DWORD   fdwStrikeOut,
    _In_ DWORD   fdwCharSet,
    _In_ DWORD   fdwOutputPrecision,
    _In_ DWORD   fdwClipPrecision,
    _In_ DWORD   fdwQuality,
    _In_ DWORD   fdwPitchAndFamily,
    _In_ LPCWSTR lpszFace
    )
{
    s_PFN_CreateFontW.CallBefore();

    HFONT hfont = s_PFN_CreateFontW.hookfunc(nHeight, nWidth, nEscapement,
        nOrientation, fnWeight, fdwItalic, fdwUnderline,
        fdwStrikeOut, fdwCharSet, fdwOutputPrecision, fdwClipPrecision,
        fdwQuality, fdwPitchAndFamily, lpszFace);

    s_PFN_CreateFontW.CallAfter();

    g_stacks[(INT_PTR)hfont] = new StackDumper;

    return hfont;
}

BOOL WINAPI _PFN_DeleteObject(
    _In_ HGDIOBJ hObject
    )
{
    s_PFN_DeleteObject.CallBefore();

    BOOL ret = s_PFN_DeleteObject.hookfunc(hObject);

    s_PFN_DeleteObject.CallAfter();

    auto it = g_stacks.find((INT_PTR)hObject);
    if (it != g_stacks.end())
    {
        delete it->second;
        g_stacks.erase(it);
    }

    return ret;
}

HFONT WINAPI _PFN_CreateFontIndirectA(__in CONST LOGFONTA *lplf)
{
    s_PFN_CreateFontIndirectA.CallBefore();
    HFONT hfont = s_PFN_CreateFontIndirectA.hookfunc(lplf);
    s_PFN_CreateFontIndirectA.CallAfter();

    g_stacks[(INT_PTR)hfont] = new StackDumper;

    return hfont;
}
HFONT WINAPI _PFN_CreateFontIndirectW(__in CONST LOGFONTW *lplf)
{
    s_PFN_CreateFontIndirectW.CallBefore();
    HFONT hfont = s_PFN_CreateFontIndirectW.hookfunc(lplf);
    s_PFN_CreateFontIndirectW.CallAfter();

    g_stacks[(INT_PTR)hfont] = new StackDumper;

    return hfont;
}

//////////////////////////////////////////////////////////////////////////
bool CAPIHook::Start()
{
    s_PFN_CreateFontA.Hook("gdi32.dll", "CreateFontW", _PFN_CreateFontA);
    s_PFN_CreateFontW.Hook("gdi32.dll", "CreateFontW", _PFN_CreateFontW);
    s_PFN_CreateFontIndirectA.Hook("gdi32.dll", "CreateFontIndirectA", _PFN_CreateFontIndirectA);
    s_PFN_CreateFontIndirectW.Hook("gdi32.dll", "CreateFontIndirectW", _PFN_CreateFontIndirectW);
    s_PFN_DeleteObject.Hook("gdi32.dll", "DeleteObject", _PFN_DeleteObject);

    return true;
}
void CAPIHook::Stop()
{

}
void CAPIHook::Reset()
{
    for (auto it = g_stacks.begin(); it != g_stacks.end(); it++)
    {
        delete it->second;
    }
    g_stacks.clear();
}
void CAPIHook::Dump()
{
    std::ofstream file("gdi.txt");

    std::string log;
    log += "======================\n";
    log += std::to_string(g_stacks.size()) + " left\n";
    log += "======================\n";
    OutputDebugStringA(log.c_str());
    file << log;
    for (auto it = g_stacks.begin(); it != g_stacks.end(); it++)
    {
        OutputDebugStringA("======================\n");
        OutputDebugStringA(it->second->out.c_str());
        file << it->second->out;
    }

    OutputDebugStringA("======================\n");
    file << "======================\n";
    file.close();
}