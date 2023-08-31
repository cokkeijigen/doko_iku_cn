#define _CRT_SECURE_NO_WARNINGS
#include "framework.h"

namespace Hook::Mem {

    bool MemWrite(uintptr_t Addr, uint8_t* Buf, size_t Size) {
        DWORD oldPro;
        SIZE_T wirteBytes = 0;
        if (VirtualProtect((VOID*)Addr, Size, PAGE_EXECUTE_READWRITE, &oldPro)) {
            WriteProcessMemory(GetCurrentProcess(), (VOID*)Addr, Buf, Size, &wirteBytes);
            VirtualProtect((VOID*)Addr, Size, oldPro, &oldPro);
            return Size == wirteBytes;
        }
        return false;
    }

    bool MemWrite(uintptr_t Addr, uint8_t _byte) {
        return MemWrite(Addr, &_byte, 1);
    }

    bool JmpWrite(uintptr_t orgAddr, uintptr_t tarAddr) {
        uint8_t jmp_write[5] = { 0xE9, 0x0, 0x0, 0x0, 0x0 };
        tarAddr = tarAddr - orgAddr - 5;
        memcpy(jmp_write + 1, &tarAddr, 4);
        return MemWrite(orgAddr, jmp_write, sizeof(jmp_write));
    }
}

namespace Hook::type {
    typedef DWORD(WINAPI *ptrGetGlyphOutlineA)(HDC, UINT, UINT, LPGLYPHMETRICS, DWORD, LPVOID, MAT2*);
    typedef HANDLE(WINAPI *ptrCreateFileA)(LPCSTR, DWORD, DWORD, LPSECURITY_ATTRIBUTES, DWORD, DWORD, HANDLE);
    struct font { HFONT jis_f; HFONT gbk_f; size_t size; };
}

namespace Hook::val {
    HMODULE   gdi32_dll;
    HMODULE   kernel32_dll;
    std::vector<type::font *> fonts;
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

    type::ptrGetGlyphOutlineA oldGetGlyphOutlineA;
    DWORD WINAPI newGetGlyphOutlineA(HDC hdc, UINT uChar, UINT fuFormat, LPGLYPHMETRICS lpgm, DWORD cjBuffer, LPVOID pvBuffer, MAT2* lpmat) {
        tagTEXTMETRICA lptm;
        GetTextMetricsA(hdc, &lptm);
        type::font* font = GetFontStruct(lptm.tmHeight);
        if (uChar == 0xA1EC) { // 替换♪
            uChar = 0x81F4;
            SelectObject(hdc, font->jis_f);
        } else {
            // 一些字符的替换
            if (uChar == 0x8140 ) uChar = 0xA1A1; // 全角空格
            else if (uChar == 0x8145) uChar = 0xA1A4; // ·
            else if (uChar == 0x8175) uChar = 0xA1B8; // 「
            else if (uChar == 0x8179) uChar = 0xA1BE; // 【
            SelectObject(hdc, font->gbk_f);
        }
        return oldGetGlyphOutlineA(hdc, uChar, fuFormat, lpgm, cjBuffer, pvBuffer, lpmat);
    }

    type::ptrCreateFileA oldCreateFileA;
    HANDLE WINAPI newCreateFileA(LPCSTR lpfn, DWORD dwda, DWORD dwsm, LPSECURITY_ATTRIBUTES lpsa, DWORD dwcd, DWORD dwfa, HANDLE htf) {
        return oldCreateFileA(!strcmp(lpfn, "update.pak") ? "doko_iku_cn.pak" : lpfn, dwda, dwsm, lpsa, dwcd, dwfa, htf);
    }
}

namespace Hook {
    
    void init() {
        if ((val::gdi32_dll = GetModuleHandle(L"gdi32.dll")) != NULL) {
            fun::oldGetGlyphOutlineA = (type::ptrGetGlyphOutlineA)GetProcAddress(val::gdi32_dll, "GetGlyphOutlineA");
        }
        if ((val::kernel32_dll = GetModuleHandle(L"kernel32.dll")) != NULL) {
            fun::oldCreateFileA = (type::ptrCreateFileA)GetProcAddress(val::kernel32_dll, "CreateFileA");
        }
    }

    void start() {
        DetourTransactionBegin();
        DetourAttach((void**)&fun::oldGetGlyphOutlineA, fun::newGetGlyphOutlineA);
        DetourAttach((void**)&fun::oldCreateFileA, fun::newCreateFileA);
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
        break;
    case DLL_THREAD_DETACH:
        break;
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}