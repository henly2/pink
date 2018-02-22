#pragma once

#include <string>
#include <sstream>
#include <vector>
#include <unordered_map>

namespace hook{

    //////////////////////////////////////////////////////////////////////////
    // tools 
    static void spliter_by_char(const std::string& str, char splitter, std::vector<std::string>& vec)
    {
        std::stringstream ss;
        ss << str;
        std::string tmp;
        while (getline(ss, tmp, splitter)){
            vec.push_back(tmp.c_str());
        }
    }

    static std::string GetModuleDir(HMODULE hModule)
    {
        std::string dir;

        char path[MAX_PATH] = { 0 };
        DWORD len = GetModuleFileNameA(hModule, path, MAX_PATH);
        if (len > 0)
        {
            dir.append(path, len);
        }

        size_t pos = dir.find_last_of('\\');
        if (pos != std::string::npos)
        {
            dir = dir.substr(0, pos);
        }

        return dir;
    }

    static std::string GetModuleName(HMODULE hModule)
    {
        std::string name;

        char path[MAX_PATH] = { 0 };
        DWORD len = GetModuleFileNameA(hModule, path, MAX_PATH);
        if (len > 0)
        {
            name.append(path, len);
        }

        size_t pos = name.find_last_of('\\');
        if (pos != std::string::npos)
        {
            name = name.substr(pos + 1, name.length() - pos - 1);
        }

        return name;
    }

    //////////////////////////////////////////////////////////////////////////
#define DEF_GDI_ITEM_ID(item) Gi_##item
#define DEF_GDI_ITEM_NAME(item) #item
#define INIT_GDI_ITEM(item) \
g_gdiid_map[DEF_GDI_ITEM_NAME(item)] = DEF_GDI_ITEM_ID(item);\
g_all_gdiitems += "|";\
g_all_gdiitems += #item;

    enum GDIID{
        DEF_GDI_ITEM_ID(nil) = 0,
        DEF_GDI_ITEM_ID(all) = 1 << 0,
        DEF_GDI_ITEM_ID(bitmap) = 1 << 1,
        DEF_GDI_ITEM_ID(brush) = 1 << 2,
        DEF_GDI_ITEM_ID(dc) = 1 << 3,
        DEF_GDI_ITEM_ID(extpen) = 1 << 4,
        DEF_GDI_ITEM_ID(font) = 1 << 5,
        DEF_GDI_ITEM_ID(palette) = 1 << 6,
        DEF_GDI_ITEM_ID(pen) = 1 << 7,
        DEF_GDI_ITEM_ID(region) = 1 << 8,
    };

    static std::string g_all_gdiitems;
    static std::unordered_map<std::string, GDIID> g_gdiid_map;
    static void init_gdiid()
    {
        INIT_GDI_ITEM(all);
        INIT_GDI_ITEM(bitmap);
        INIT_GDI_ITEM(brush);
        INIT_GDI_ITEM(dc);
        INIT_GDI_ITEM(extpen);
        INIT_GDI_ITEM(font);
        INIT_GDI_ITEM(palette);
        INIT_GDI_ITEM(pen);
        INIT_GDI_ITEM(region);
    }
    static GDIID getgdiid_bystring(const std::string& item)
    {
        auto it = g_gdiid_map.find(item);
        if (it != g_gdiid_map.end())
        {
            return it->second;
        }

        return DEF_GDI_ITEM_ID(nil);
    }

    static int compose_gdiids(const std::string& str)
    {
        int value = 0;
        std::vector<std::string> vec;
        spliter_by_char(str, '|', vec);

        for each (auto var in vec)
        {
            GDIID gid = getgdiid_bystring(var);
            value |= gid;
        }

        return value;
    }

    static bool is_gdiid(int value, int id)
    {
        return (value & id) > 0;
    }

#define IS_GDIID(value, item) \
    is_gdiid(value, DEF_GDI_ITEM_ID(item))

    //////////////////////////////////////////////////////////////////////////
#define DEF_FUNC_ITEM_ID(item) Func_##item
#define DEF_FUNC_ITEM_NAME(item) #item
#define INIT_FUNC_ITEM(item) \
g_funcid_map[DEF_FUNC_ITEM_NAME(item)] = DEF_FUNC_ITEM_ID(item);\

    enum FUNCID{
        DEF_FUNC_ITEM_ID(nil) = 0,
        DEF_FUNC_ITEM_ID(listprocess),
        DEF_FUNC_ITEM_ID(attach),
        DEF_FUNC_ITEM_ID(detach),
        DEF_FUNC_ITEM_ID(quit),
        DEF_FUNC_ITEM_ID(he),
        DEF_FUNC_ITEM_ID(hd),
        DEF_FUNC_ITEM_ID(clear),
        DEF_FUNC_ITEM_ID(dump),
        DEF_FUNC_ITEM_ID(me),
        DEF_FUNC_ITEM_ID(md),
    };

    static std::unordered_map<std::string, FUNCID> g_funcid_map;
    static void init_funcid()
    {
        INIT_FUNC_ITEM(listprocess);
        INIT_FUNC_ITEM(attach);
        INIT_FUNC_ITEM(detach);
        INIT_FUNC_ITEM(quit);
        INIT_FUNC_ITEM(he);
        INIT_FUNC_ITEM(hd);
        INIT_FUNC_ITEM(clear);
        INIT_FUNC_ITEM(dump);

        INIT_FUNC_ITEM(me);
        INIT_FUNC_ITEM(md);
    }
    static FUNCID getfuncid_bystring(const std::string& func)
    {
        auto it = g_funcid_map.find(func);
        if (it != g_funcid_map.end())
        {
            return it->second;
        }

        return DEF_FUNC_ITEM_ID(nil);
    }

    //////////////////////////////////////////////////////////////////////////
    static const char* CLASS_NAME_HOST = "SimpleHwndMessage_host";
    static const char* CLASS_NAME_DLL = "SimpleHwndMessage_dll";

#define WM_IPC_TODLL (WM_USER + 10)
#define WM_IPC_TOHOST (WM_USER + 11)
#define WM_IPC_TOHOST2 (WM_USER + 12)

}