#pragma once

#include "APIHook.hpp"

#include <string>
#include <unordered_map>

#include <fstream>

static std::unordered_map<int, std::string> s_font_stack;

//////////////////////////////////////////////////////////////////////////
// 定义类型
typedef decltype(CreateFontA) (*PFN_CreateFontA);
typedef decltype(CreateFontW) (*PFN_CreateFontW);
typedef decltype(CreateFontIndirectA) (*PFN_CreateFontIndirectA);
typedef decltype(CreateFontIndirectW) (*PFN_CreateFontIndirectW);
typedef decltype(DeleteObject) (*PFN_DeleteObject);
typedef decltype(CreateFontIndirectExA) (*PFN_CreateFontIndirectExA);
typedef decltype(CreateFontIndirectExW) (*PFN_CreateFontIndirectExW);


//////////////////////////////////////////////////////////////////////////
// 定义对象
static api_hook::APIHookInfo<PFN_CreateFontA> s_PFN_CreateFontA;
static api_hook::APIHookInfo<PFN_CreateFontW> s_PFN_CreateFontW;
static api_hook::APIHookInfo<PFN_CreateFontIndirectA> s_PFN_CreateFontIndirectA;
static api_hook::APIHookInfo<PFN_CreateFontIndirectW> s_PFN_CreateFontIndirectW;
static api_hook::APIHookInfo<PFN_DeleteObject> s_PFN_DeleteObject;
static api_hook::APIHookInfo<PFN_CreateFontIndirectExA> s_PFN_CreateFontIndirectExA;
static api_hook::APIHookInfo<PFN_CreateFontIndirectExW> s_PFN_CreateFontIndirectExW;

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

	s_font_stack[(INT_PTR)hfont] = api_hook::StackWalker::DumpStack();

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

	s_font_stack[(INT_PTR)hfont] = api_hook::StackWalker::DumpStack();

    return hfont;
}

BOOL WINAPI _PFN_DeleteObject(
    _In_ HGDIOBJ hObject
    )
{
    s_PFN_DeleteObject.CallBefore();
    BOOL ret = s_PFN_DeleteObject.hookfunc(hObject);
    s_PFN_DeleteObject.CallAfter();

    auto it = s_font_stack.find((INT_PTR)hObject);
    if (it != s_font_stack.end())
    {
		s_font_stack.erase(it);
    }

    return ret;
}

HFONT WINAPI _PFN_CreateFontIndirectA(__in CONST LOGFONTA *lplf)
{
    s_PFN_CreateFontIndirectA.CallBefore();
    HFONT hfont = s_PFN_CreateFontIndirectA.hookfunc(lplf);
    s_PFN_CreateFontIndirectA.CallAfter();

	s_font_stack[(INT_PTR)hfont] = api_hook::StackWalker::DumpStack();

    return hfont;
}
HFONT WINAPI _PFN_CreateFontIndirectW(__in CONST LOGFONTW *lplf)
{
    s_PFN_CreateFontIndirectW.CallBefore();
    HFONT hfont = s_PFN_CreateFontIndirectW.hookfunc(lplf);
    s_PFN_CreateFontIndirectW.CallAfter();

	s_font_stack[(INT_PTR)hfont] = api_hook::StackWalker::DumpStack();

    return hfont;
}

HFONT WINAPI _PFN_CreateFontIndirectExA(
	_In_ const ENUMLOGFONTEXDVA *penumlfex
)
{
	s_PFN_CreateFontIndirectExA.CallBefore();
	HFONT hfont = s_PFN_CreateFontIndirectExA.hookfunc(penumlfex);
	s_PFN_CreateFontIndirectExA.CallAfter();

	s_font_stack[(INT_PTR)hfont] = api_hook::StackWalker::DumpStack();

	return hfont;
}

HFONT WINAPI _PFN_CreateFontIndirectExW(
	_In_ const ENUMLOGFONTEXDVW *penumlfex
)
{
	s_PFN_CreateFontIndirectExW.CallBefore();
	HFONT hfont = s_PFN_CreateFontIndirectExW.hookfunc(penumlfex);
	s_PFN_CreateFontIndirectExW.CallAfter();

	s_font_stack[(INT_PTR)hfont] = api_hook::StackWalker::DumpStack();

	return hfont;
}

//////////////////////////////////////////////////////////////////////////
static void hook_font_api()
{
    s_PFN_CreateFontA.Hook("gdi32.dll", "CreateFontW", _PFN_CreateFontA);
    s_PFN_CreateFontW.Hook("gdi32.dll", "CreateFontW", _PFN_CreateFontW);
    s_PFN_CreateFontIndirectA.Hook("gdi32.dll", "CreateFontIndirectA", _PFN_CreateFontIndirectA);
    s_PFN_CreateFontIndirectW.Hook("gdi32.dll", "CreateFontIndirectW", _PFN_CreateFontIndirectW);
    s_PFN_DeleteObject.Hook("gdi32.dll", "DeleteObject", _PFN_DeleteObject);

	s_PFN_CreateFontIndirectExA.Hook("gdi32.dll", "CreateFontIndirectExA", _PFN_CreateFontIndirectExA);
	s_PFN_CreateFontIndirectExW.Hook("gdi32.dll", "CreateFontIndirectExW", _PFN_CreateFontIndirectExW);
}

static void unhook_font_api()
{
	s_PFN_CreateFontA.UnHook();
	s_PFN_CreateFontW.UnHook();
	s_PFN_CreateFontIndirectA.UnHook();
	s_PFN_CreateFontIndirectW.UnHook();
	s_PFN_DeleteObject.UnHook();
}

static void clear_font_stack()
{
	s_font_stack.clear();
}

static void dump_font_stack()
{
	std::ofstream file("gdi_font.leak");

	file << "-----------------------------Begin----------------------------\n";
	file << "Found " << s_font_stack.size() << " leak\n";
	file << "--------------------------------------------------------------\n";
	int index = 0;
	for (auto it = s_font_stack.begin(); it != s_font_stack.end(); it++)
	{
		file << "Font leak " << ++index << "\n";

		file << it->second;

		file << "\n\n";
	}
	file << "-----------------------------End----------------------------\n";

	file.close();
}