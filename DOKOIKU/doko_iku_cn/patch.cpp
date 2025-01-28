#include <iostream>
#include <windows.h>
#include <patch.h>

namespace Patch::Mem {

	bool Patch::Mem::MemWriteImpl(uintptr_t Addr, uintptr_t Buf, size_t Size) {
		DWORD OldPro = NULL; SIZE_T wirteBytes = NULL;
		if (VirtualProtect((VOID*)Addr, Size, PAGE_EXECUTE_READWRITE, &OldPro)) {
			WriteProcessMemory(INVALID_HANDLE_VALUE, (VOID*)Addr, (VOID*)Buf, Size, &wirteBytes);
			VirtualProtect((VOID*)Addr, Size, OldPro, &OldPro);
			return Size == wirteBytes;
		}
		return false;
	}

	bool Patch::Mem::JmpWriteImpl(uintptr_t orgAddr, uintptr_t tarAddr) {
		uint8_t jmp_write[5] = { 0xE9, 0x0, 0x0, 0x0, 0x0 };
		*(uintptr_t*)(jmp_write + 1) = tarAddr - orgAddr - 5;
		return MemWriteImpl(orgAddr, (uintptr_t)&jmp_write, 5);
	}

	const uintptr_t& Patch::Mem::GetBaseAddr() {
		static const uintptr_t BaseAddr = (uintptr_t)GetModuleHandleW(NULL);
		return BaseAddr;
	}

}
