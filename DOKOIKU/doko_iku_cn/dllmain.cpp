#define _CRT_SECURE_NO_WARNINGS
#include "framework.h"

namespace Hook::type {
    typedef DWORD(WINAPI *GetGlyphOutlineA)(HDC, UINT, UINT, LPGLYPHMETRICS, DWORD, LPVOID, MAT2*);
    struct font { HFONT jis_f; HFONT gbk_f; size_t size; };
}

namespace Hook::val {
    HMODULE gdi32;
    HMODULE kernel32;
    std::vector<type::font*> fonts;
}

namespace Hook::fun {

    type::font* GetFontStruct(size_t size) {
        for (type::font* f : val::fonts) if(f->size == size) return f;
        type::font* nf = new type::font { nullptr, nullptr, size };
        nf->gbk_f = CreateFontA(size, size / 2, 0, 0, 0, 0, 0, 0, 0x86, 4, 0x20, 4, 4, "黑体");
        nf->jis_f = CreateFontA(size, size / 2, 0, 0, 0, 0, 0, 0, 0x80, 4, 0x20, 4, 4, "黑体");
        val::fonts.push_back(nf);
        return nf;
    }

    type::GetGlyphOutlineA oldGetGlyphOutlineA;
    DWORD WINAPI newGetGlyphOutlineA(HDC hdc, UINT uChar, UINT fuf, LPGLYPHMETRICS lpgm, DWORD cjbf, LPVOID pvbf, MAT2* lpmat) {
        tagTEXTMETRICA lptm;
        GetTextMetricsA(hdc, &lptm);
        type::font* font = GetFontStruct(lptm.tmHeight);
        if (uChar == 0xA1EC) { // 替换♪
            uChar = 0x81F4;
            SelectObject(hdc, font->jis_f);
        } else {
            // 一些字符的替换
            if (uChar == 0x8140) uChar = 0xA1A1; // 全角空格
            else if (uChar == 0x8145) uChar = 0xA1A4; // ·
            else if (uChar == 0x8175) uChar = 0xA1B8; // 「
            else if (uChar == 0x8179) uChar = 0xA1BE; // 【
            SelectObject(hdc, font->gbk_f);
        }
        return oldGetGlyphOutlineA(hdc, uChar, fuf, lpgm, cjbf, pvbf, lpmat);
    }

}

namespace Hook {

    void init() {
       if ((val::gdi32 = GetModuleHandleA("gdi32.dll")) != NULL) {
            fun::oldGetGlyphOutlineA = (type::GetGlyphOutlineA)GetProcAddress(val::gdi32, "GetGlyphOutlineA");
        }
    }

    void start() {
        DetourTransactionBegin();
        DetourAttach((void**)&fun::oldGetGlyphOutlineA, fun::newGetGlyphOutlineA);
        DetourUpdateThread(GetCurrentThread());
        DetourTransactionCommit();
    }
}

BOOL APIENTRY DllMain( HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved) 
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    #ifdef _DEBUG
        AllocConsole();
        freopen("CONOUT$", "w", stdout);
        freopen("CONIN$", "r", stdin);
    #endif 
        Hook::init();
        Hook::start();
        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}