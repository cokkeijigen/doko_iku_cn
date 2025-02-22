﻿#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <windows.h>
#include <patch.h>
#include <console.hpp>
#include <FontManager.hpp>
#include <PackManger.hpp>
#include <Utils.hpp>

namespace Hook {
    
    static HFONT DefualtSymbolFont{ NULL },  TextCharacterFont{ NULL };
    static std::unique_ptr<Utils::FontManager> FontManager { nullptr };

    static DWORD WINAPI GetGlyphOutlineA(HDC hdc, UINT uChar, UINT fuf, LPGLYPHMETRICS lpgm, DWORD cjbf, LPVOID pvbf, MAT2* lpmat) {
        auto result = static_cast<DWORD>(GDI_ERROR);
        if (uChar == 0xA1EC) {
            //uChar = 0x81F4; // 替换♪
            ::SelectObject(hdc, Hook::DefualtSymbolFont);
            result = ::GetGlyphOutlineW(hdc, L'♪', fuf, lpgm, cjbf, pvbf, lpmat);
        }
        else if (uChar == 0xA7A4) {
            ::SelectObject(hdc, Hook::DefualtSymbolFont);
            result = ::GetGlyphOutlineW(hdc, L'❤', fuf, lpgm, cjbf, pvbf, lpmat);
        }
        else if (uChar == 0xA7A5) {
            ::SelectObject(hdc, Hook::DefualtSymbolFont);
            result = ::GetGlyphOutlineW(hdc, L'♡', fuf, lpgm, cjbf, pvbf, lpmat);
        }
        else {
            // 一些字符的替换
            if (uChar == 0x8140) uChar = 0xA1A1; // 全角空格
            else if (uChar == 0x8145) uChar = 0xA1A4;  // ·
            else if (uChar == 0x8175) uChar = 0xA1B8;  // 「
            else if (uChar == 0x8179) uChar = 0xA1BE;  // 【
            else uChar = Utils::UCharFull2Half(uChar); // 替换半角字符
            char text[3]{ char(uChar& 0xFF), char(uChar>>8)};
            //console::fmt::write("uChar: %s \n", text);
            ::SelectObject(hdc, Hook::TextCharacterFont);
            result = Patch::Hooker::Call<Hook::GetGlyphOutlineA>(hdc, uChar, fuf, lpgm, cjbf, pvbf, lpmat);
        }
        *reinterpret_cast<int32_t*>(0x4537F8) = lpgm->gmCellIncX + 1; // 设置字符宽度，要半宽显示半角字符这点很重要！
        return result;
    }

    static int SetLineBreak() {
        auto&& result = reinterpret_cast<decltype(Hook::SetLineBreak)*>(0x40B2B0)();
        *reinterpret_cast<int32_t*>(0x6C1B50) = 0x18; // 设置换行缩进宽度
        if (*reinterpret_cast<uint16_t*>(0x6C7938) == 0x7581) {
            *reinterpret_cast<int32_t*>(0x6C1B50) += *reinterpret_cast<int32_t*>(0x4537F8);
        }
        return result;
    }

    static int __fastcall ReadGameFile(void* m_this, int edx, const char* name) {
        static Utils::PackManger GamePackManger("doko_iku_cn.pak");
        int result = 0;
        if (int index = GamePackManger.GetFileIndex(name); index != -1) {
            *(DWORD*)(uintptr_t(m_this) + 0xEA718) = DWORD(GamePackManger.GetFileSize(index));
            auto&& free   = reinterpret_cast<decltype(std::free)*>  (0x44394A);
            auto&& malloc = reinterpret_cast<decltype(std::malloc)*>(0x443E4F);
            if (*(DWORD*)(uintptr_t(m_this) + 0x0C)) {
                free(*(void**)(uintptr_t(m_this) + 0xC));
            }
            if (void* buffer = malloc(*(DWORD*)(uintptr_t(m_this) + 0xEA718))) {
                *(DWORD*)(uintptr_t(m_this) + 0x04) = DWORD(buffer);
                *(DWORD*)(uintptr_t(m_this) + 0x08) = DWORD(buffer);
                *(DWORD*)(uintptr_t(m_this) + 0x0C) = DWORD(buffer);
                result = GamePackManger.GetFileData(index, buffer);
            }
        }
        //console::fmt::write("[Replace] %s -> %s\n", name, (result ? "true" : "false"));
        return result ? result : Patch::Hooker::Call<Hook::ReadGameFile>(m_this, edx, name);
    }

    static LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
        
        if (uMsg == WM_CREATE) {
            static const auto nTitle { u8"【桐敷学园宣发部】那一天，去往何方 - Ver.1.01" };
            if (CP_UTF8 == Utils::OsCurrentCodePage) {
                ::SetWindowTextA(hWnd, nTitle);
            }
            else {
                ::SetWindowTextW(hWnd, Utils::ConvertToUTF16(nTitle, CP_UTF8).c_str());
            }
            if (Hook::FontManager == nullptr && (Hook::FontManager = Utils::FontManager::CreatePtr(hWnd))) {
                static const auto UseCharSet{ static_cast<DWORD>(Utils::OsCurrentCodePage == 936 ? ANSI_CHARSET : 0x86) };
                Hook::FontManager->Init(24, Utils::FontManager::NORMAL, L"黑体", 18, 30).Load(".\\save\\chs_font.dat")
                    .OnChanged([](const Utils::FontManager* m_this) -> void {
                        if (Hook::TextCharacterFont) ::DeleteObject(Hook::TextCharacterFont);
                        Hook::TextCharacterFont = m_this->MakeFont(UseCharSet);
                        //const auto& data = m_this->GetData();
                        //console::fmt::write(L"[ChooseFont] size: %d style: 0x%X name: %s\n", data.size, int(data.style), data.name);
                    });
                Hook::TextCharacterFont = Hook::FontManager->MakeFont(UseCharSet);
                Hook::DefualtSymbolFont = Hook::FontManager->MakeDefualtFont(0x81);
            }
        }
        else if (uMsg == WM_SIZE && Hook::FontManager) {
            if (Hook::FontManager) FontManager->UpdateDisplayState();
        }
        else if (uMsg == WM_COMMAND && LOWORD(wParam) == static_cast<uint16_t>(114514)) {
            if (Hook::FontManager) Hook::FontManager->ChooseFont();
            return TRUE;
        }
        
        return Patch::Hooker::Call<Hook::WndProc>(hWnd, uMsg, wParam, lParam);
    }

    static BOOL WINAPI AppendMenuA(HMENU hMenu, UINT uFlags, UINT_PTR uIDNewItem, LPCSTR lpNewItem) {
        //console::fmt::write<936>("[AppendMenuA] %s\n", lpNewItem);
        return ::AppendMenuW(hMenu, uFlags, uIDNewItem, Utils::ConvertToUTF16(lpNewItem).c_str());
    }

    static BOOL WINAPI ModifyMenuA(HMENU hMnu, UINT uPosition, UINT uFlags, UINT_PTR uIDNewItem, LPCSTR lpNewItem) {
        //console::fmt::write<936>("[ModifyMenuA] %s\n", lpNewItem);
        return ::ModifyMenuW(hMnu, uPosition, uFlags, uIDNewItem, Utils::ConvertToUTF16(lpNewItem).c_str());
    }

    static int WINAPI MessageBoxA(HWND hWnd, LPCSTR lpText, LPCSTR lpCaption, UINT uType) {
        //console::fmt::write<936>("[MessageBoxA] %s %s\n", lpText, lpCaption);
        return ::MessageBoxW(hWnd, Utils::ConvertToUTF16(lpText).c_str(), Utils::ConvertToUTF16(lpCaption).c_str(), uType);
    }

    inline static void Init() {
        //console::make();
        Patch::Hooker::Begin();
        if (Utils::OsCurrentCodePage != 936) {
            Patch::Hooker::Add<Hook::ModifyMenuA>(::ModifyMenuA);
            Patch::Hooker::Add<Hook::AppendMenuA>(::AppendMenuA);
            Patch::Hooker::Add<Hook::MessageBoxA>(::MessageBoxA);
        }
        Patch::Hooker::Add<Hook::GetGlyphOutlineA>(::GetGlyphOutlineA);
        Patch::Hooker::Add<Hook::WndProc>(reinterpret_cast<void*>(0x425F90));
        Patch::Hooker::Add<Hook::ReadGameFile>(reinterpret_cast<void*>(0x40BF90));
        RELEASE_ONLY({ // jmp 0x40C05F, nop nop ; 跳过免封包读取的逻辑
            uint8_t jmpAsm[] = { 0xE9, 0x90, 0x00, 0x00, 0x00, 0x90, 0x90 };
            Patch::Mem::MemWriteImpl(0x40BFCA, uintptr_t(&jmpAsm), sizeof(jmpAsm));
        })
        uint8_t newAsm[] = { // hook菜单中的"字体"选项
            0xA1, 0x6C, 0x5C, 0x6C, 0x00, // mov eax, dword ptr ds:[0x006C5C6C]
            0x68, 0x34, 0x7A, 0x45, 0x00, // push 0x457A34 "字体(&T)"
            0x68, 0x52, 0xBF, 0x01, 0x00, // push 0x01BF52 (114514)
            0x6A, 0x10, 0x50, // push 0x10, push eax
            0x90, 0x90, 0x90  // nop, nop, nop
        };
        Patch::Mem::MemWriteImpl(0x4293D6, uintptr_t(&newAsm), sizeof(newAsm) - 0); // AppendMenu
        Patch::Mem::MemWriteImpl(0x42906A, uintptr_t(&newAsm), sizeof(newAsm) - 1); // AppendMenu
        uint8_t asmNew[] = { 0xB9, 0x52, 0xBF, 0x01, 0x00, 0x90 }; // mov ecx, 0x1BF52 (114514); nop
        Patch::Mem::MemWriteImpl(0x428FC9, uintptr_t(&asmNew), sizeof(asmNew) - 0); // RemoveMenu
        auto addr = reinterpret_cast<uintptr_t>(&Hook::SetLineBreak) - 0x433DB3 - 0x05;
        Patch::Mem::MemWriteImpl(0x433DB4, uintptr_t(&addr), 0x04); // hook自动换行，没卵用？孩子不懂事hook着玩
        Patch::Hooker::Commit();
    }
}

extern "C"  {
    
    __declspec(dllexport) const char* _patch_by_iTsukezigen_(void) {
        return "https://github.com/cokkeijigen/doko_iku_cn";
    }

    BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved) {
        if (ul_reason_for_call == DLL_PROCESS_ATTACH) {
            Hook::Init();
        }
        return TRUE;
    }
}
