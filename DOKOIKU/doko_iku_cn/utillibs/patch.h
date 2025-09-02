#pragma once
#include <detours.h>
#pragma comment(lib, "detours.lib")

#ifdef _DEBUG
#define DEBUG_ONLY(...) __VA_ARGS__
#define RELEASE_ONLY(...)
#else
#define DEBUG_ONLY(...) 
#define RELEASE_ONLY(...) __VA_ARGS__
#endif // _DEBUG

namespace Patch {

	struct Hooker
	{
		template<auto Fun>
		inline static decltype(Fun) Call;

		inline static auto Begin() -> void
		{
			::DetourTransactionBegin();
		}

		template<auto Fun>
		inline static auto Add(decltype(Fun) target) -> void
		{
			::DetourAttach({ &(Hooker::Call<Fun> = {target}) }, Fun);
		}

		template<auto Fun>
		inline static auto Add(const void* target) -> void
		{
			Hooker::Add<Fun>(reinterpret_cast<decltype(Fun)>(target));
		}

		inline static auto Commit() -> void
		{
			::DetourUpdateThread(::GetCurrentThread());
			::DetourTransactionCommit();
		}
	};
}

namespace Patch::Mem {

	extern const uintptr_t& GetBaseAddr();

	extern bool MemWriteImpl(uintptr_t Addr, uintptr_t Buf, size_t Size);

	extern bool JmpWriteImpl(uintptr_t orgAddr, uintptr_t tarAddr);

	inline const uintptr_t& BaseAddr = Patch::Mem::GetBaseAddr();

	template<uintptr_t rva>
	struct Rva {
		constexpr inline static const uintptr_t Value = rva;
		inline static const uintptr_t Ptr = rva + BaseAddr;

		template<typename T> struct Cast {
			inline static const T* Ptr = (T*)(rva + BaseAddr);
			inline static const T& Val = *(T*)&Ptr;
		};
	};

	template <typename ...Ts> struct type_check {
		constexpr static bool const value = ((std::is_convertible<Ts, uintptr_t>::value ||
			std::is_pointer<typename std::decay<Ts>::type>::value) && ...);
	};

	template<typename tptr_t, typename bptr_t, class = typename
		std::enable_if<type_check<tptr_t, bptr_t>::value>::type>
	inline static bool MemWrite(tptr_t&& Addr, bptr_t&& Buf, size_t Size) {
		return MemWriteImpl(uintptr_t(Addr), uintptr_t(Buf), Size);
	}

	template<typename bptr_t, typename rptr_t, typename bbptr_t, class = typename
		std::enable_if<type_check<bptr_t, rptr_t, bbptr_t>::value>::type>
	inline static bool MemWrite(bptr_t&& BaseAddr, rptr_t&& RvaAddr, bbptr_t&& Buf, size_t Size) {
		return MemWriteImpl(uintptr_t(BaseAddr) + uintptr_t(RvaAddr), uintptr_t(Buf), Size);
	}

	template<typename optr_t, typename tptr_t, class = typename
		std::enable_if<type_check<optr_t, tptr_t>::value>::type>
	inline static bool JmpWrite(optr_t&& orgAddr, tptr_t&& tarAddr) {
		return JmpWriteImpl(uintptr_t(orgAddr), uintptr_t(tarAddr));
	}

	template<typename bptr_t, typename rptr_t, typename tptr_t, class = typename
		std::enable_if<type_check<bptr_t, rptr_t, tptr_t>::value>::type>
	inline static bool JmpWrite(bptr_t&& BaseAddr, rptr_t&& RvaAddr, tptr_t&& tarAddr) {
		return JmpWriteImpl(uintptr_t(BaseAddr) + uintptr_t(RvaAddr), uintptr_t(tarAddr));
	}
}