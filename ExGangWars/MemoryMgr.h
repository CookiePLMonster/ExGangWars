#ifndef __MEMORYMGR
#define __MEMORYMGR

// Switches:
// _MEMORY_NO_CRT - don't include anything "complex" like ScopedUnprotect or memset
// _MEMORY_DECLS_ONLY - don't include anything but macroes

#define WRAPPER __declspec(naked)
#define DEPRECATED __declspec(deprecated)
#define EAXJMP(a) { _asm mov eax, a _asm jmp eax }
#define VARJMP(a) { _asm jmp a }
#define WRAPARG(a) ((int)a)

#define NOVMT __declspec(novtable)
#define SETVMT(a) *((uintptr_t*)this) = (uintptr_t)a

#ifndef _MEMORY_DECLS_ONLY

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <cstdint>

enum
{
	PATCH_CALL,
	PATCH_JUMP
};

inline signed char* GetVer()
{
	static signed char	bVer = -1;
	return &bVer;
}

inline bool* GetEuropean()
{
	static bool			bEuropean;
	return &bEuropean;
}

inline void* GetDummy()
{
	static uintptr_t		dwDummy;
	return &dwDummy;
}

template<typename AT>
inline AT DynBaseAddress(AT address)
{
	return (ptrdiff_t)GetModuleHandle(nullptr) - 0x400000 + address;
}

#if defined _GTA_III

inline void InitializeVersions()
{
	signed char*	bVer = GetVer();

	if ( *bVer == -1 )
	{
		if (*(uint32_t*)0x5C1E70 == 0x53E58955) *bVer = 0;
		else if (*(uint32_t*)0x5C2130 == 0x53E58955) *bVer = 1;
		else if (*(uint32_t*)0x5C6FD0 == 0x53E58955) *bVer = 2;
	}
}

// This function initially detects III version then chooses the address basing on game version
template<typename T>
inline T AddressByVersion(uintptr_t address10, uintptr_t address11, uintptr_t addressSteam)
{
	InitializeVersions();

	signed char		bVer = *GetVer();

	switch ( bVer )
	{
	case 1:
#ifdef assert
		assert(address11);
#endif
		return (T)address11;
	case 2:
#ifdef assert
		assert(addressSteam);
#endif
		return (T)addressSteam;
	default:
#ifdef assert
		assert(address10);
#endif
		return (T)address10;
	}
}

#elif defined _GTA_VC

inline void InitializeVersions()
{
	signed char*	bVer = GetVer();

	if ( *bVer == -1 )
	{
		if (*(uint32_t*)0x667BF0 == 0x53E58955) *bVer = 0;
		else if (*(uint32_t*)0x667C40 == 0x53E58955) *bVer = 1;
		else if (*(uint32_t*)0x666BA0 == 0x53E58955) *bVer = 2;
	}
}

// This function initially detects VC version then chooses the address basing on game version
template<typename T>
inline T AddressByVersion(uintptr_t address10, uintptr_t address11, uintptr_t addressSteam)
{
	InitializeVersions();

	signed char	bVer = *GetVer();

	switch ( bVer )
	{
	case 1:
#ifdef assert
		assert(address11);
#endif
		return (T)address11;
	case 2:
#ifdef assert
		assert(addressSteam);
#endif
		return (T)addressSteam;
	default:
#ifdef assert
		assert(address10);
#endif
		return (T)address10;
	}
}

#elif defined _GTA_SA

inline void InitializeVersions()
{
	signed char*	bVer = GetVer();
	bool*			bEuropean = GetEuropean();

	if ( *bVer == -1 )
	{
		if ( *(uint32_t*)DynBaseAddress(0x82457C) == 0x94BF )
		{
			// 1.0 US
			*bVer = 0;
			*bEuropean = false;
		}
		else if ( *(uint32_t*)DynBaseAddress(0x8245BC) == 0x94BF )
		{
			// 1.0 EU
			*bVer = 0;
			*bEuropean = true;
		}
		else if ( *(uint32_t*)DynBaseAddress(0x8252FC) == 0x94BF )
		{
			// 1.01 US
			*bVer = 1;
			*bEuropean = false;
		}
		else if ( *(uint32_t*)DynBaseAddress(0x82533C) == 0x94BF )
		{
			// 1.01 EU
			*bVer = 1;
			*bEuropean = true;
		}
		else if (*(uint32_t*)DynBaseAddress(0x85EC4A) == 0x94BF )
		{
			// 3.0
			*bVer = 2;
			*bEuropean = false;
		}

		else if ( *(uint32_t*)DynBaseAddress(0x858D21) == 0x3539F633 )
		{
			// newsteam r1
			*bVer = 3;
			*bEuropean = false;
		}
		else if ( *(uint32_t*)DynBaseAddress(0x858D51) == 0x3539F633 )
		{
			// newsteam r2
			*bVer = 4;
			*bEuropean = false;
		}
		else if ( *(uint32_t*)DynBaseAddress(0x858C61) == 0x3539F633 )
		{
			// newsteam r2 lv
			*bVer = 5;
			*bEuropean = false;
		}
	}
}

inline void InitializeRegion_10()
{
	bool*			bEuropean = GetEuropean();
	signed char*	bVer = GetVer();

	if ( *bVer == -1 )
	{
		if ( *(uint32_t*)0x82457C == 0x94BF )
		{
			*bVer = 0;
			*bEuropean = false;
		}
		else if ( *(uint32_t*)0x8245BC == 0x94BF )
		{
			*bVer = 0;
			*bEuropean = true;
		}
		else
		{
#ifdef assert
			assert(!"AddressByRegion_10 on non-1.0 EXE!");
#endif
		}
	}
}

inline void InitializeRegion_11()
{
	bool*			bEuropean = GetEuropean();
	signed char*	bVer = GetVer();

	if ( *bVer == -1 )
	{
		if ( *(uint32_t*)0x8252FC == 0x94BF )
		{
			*bVer = 1;
			*bEuropean = false;
		}
		else if ( *(uint32_t*)0x82533C == 0x94BF )
		{
			*bVer = 1;
			*bEuropean = true;
		}
		else
		{
#ifdef assert
			assert(!"AddressByRegion_11 on non-1.01 EXE!");
#endif
		}
	}
}

// This function initially detects SA version then chooses the address basing on game version
template<typename T>
inline T AddressByVersion(uintptr_t address10, uintptr_t address11, uintptr_t addressSteam)
{
	InitializeVersions();

	signed char	bVer = *GetVer();
	bool			bEuropean = *GetEuropean();

	switch ( bVer )
	{
	case 1:
#ifdef assert
		assert(address11);
#endif

		// Safety measures - if null, return dummy var pointer to prevent a crash
		if ( !address11 )
			return (T)GetDummy();

		// Adjust to US if needed
		if ( !bEuropean && address11 > 0x746FA0 )
		{
			if ( address11 < 0x7BB240 )
				address11 -= 0x50;
			else
				address11 -= 0x40;
		}
		return (T)address11;
	case 2:
#ifdef assert
		assert(addressSteam);
#endif
		// Safety measures - if null, return dummy var pointer to prevent a crash
		if ( !addressSteam )
			return (T)GetDummy();

		return (T)addressSteam;
	case 3:
	case 4:
	case 5:
		// TODO: DO
		return (T)GetDummy();
	default:
#ifdef assert
		assert(address10);
#endif
		// Adjust to EU if needed
		if ( bEuropean && address10 > 0x7466D0 )
		{
			if ( address10 < 0x7BA940 )
				address10 += 0x50;
			else
				address10 += 0x40;
		}
		return (T)address10;
	}
}

template<typename T>
inline T AddressByVersion(uintptr_t address10, uintptr_t address11, uintptr_t addressSteam, uintptr_t addressNewsteamR2, uintptr_t addressNewsteamR2_LV)
{
	InitializeVersions();

	signed char	bVer = *GetVer();
	bool			bEuropean = *GetEuropean();

	switch ( bVer )
	{
	case 1:
#ifdef assert
		assert(address11);
#endif

		// Safety measures - if null, return dummy var pointer to prevent a crash
		if ( !address11 )
			return (T)GetDummy();

		// Adjust to US if needed
		if ( bEuropean && address11 > 0x746FA0 )
		{
			if ( address11 < 0x7BB240 )
				address11 -= 0x50;
			else
				address11 -= 0x40;
		}
		return (T)address11;
	case 2:
#ifdef assert
		assert(addressSteam);
#endif
		// Safety measures - if null, return dummy var pointer to prevent a crash
		if ( !addressSteam )
			return (T)GetDummy();

		return (T)addressSteam;
	case 3:
		return (T)GetDummy();
	case 4:
#ifdef assert
		assert(addressNewsteamR2);
#endif
		if ( !addressNewsteamR2 )
			return (T)GetDummy();

		return (T)DynBaseAddress(addressNewsteamR2);
	case 5:
#ifdef assert
		assert(addressNewsteamR2_LV);
#endif
		if ( !addressNewsteamR2_LV )
			return (T)GetDummy();

		return (T)DynBaseAddress(addressNewsteamR2_LV);
	default:
#ifdef assert
		assert(address10);
#endif
		// Adjust to EU if needed
		if ( bEuropean && address10 > 0x7466D0 )
		{
			if ( address10 < 0x7BA940 )
				address10 += 0x50;
			else
				address10 += 0x40;
		}
		return (T)address10;
	}
}

template<typename T>
inline T AddressByRegion_10(uintptr_t address10)
{
	InitializeRegion_10();

	bool			bEuropean = *GetEuropean();

	// Adjust to EU if needed
	if ( bEuropean && address10 > 0x7466D0 )
	{
		if ( address10 < 0x7BA940 )
			address10 += 0x50;
		else
			address10 += 0x40;
	}
	return (T)address10;
}

template<typename T>
inline T AddressByRegion_11(uintptr_t address11)
{
	InitializeRegion_11();

	bool			bEuropean = *GetEuropean();

	// Adjust to US if needed
	if ( !bEuropean && address11 > 0x746FA0 )
	{
		if ( address11 < 0x7BB240 )
			address11 -= 0x50;
		else
			address11 -= 0x40;
	}
	return (T)address11;
}

#endif

namespace Memory
{
	template<typename T, typename AT>
	inline void		Patch(AT address, T value)
	{*(T*)address = value; }

	template<typename AT>
	inline void		Nop(AT address, size_t count)
#ifndef _MEMORY_NO_CRT
	{ memset((void*)address, 0x90, count); }
#else
	{ do {
		*(uint8_t*)address++ = 0x90;
	} while ( --count != 0 ); }
#endif

	template<typename AT, typename HT>
	inline void		InjectHook(AT address, HT hook)
	{
		intptr_t		dwHook;
		_asm
		{
			mov		eax, hook
			mov		dwHook, eax
		}

		*(ptrdiff_t*)((intptr_t)address + 1) = dwHook - (intptr_t)address - 5;
	}

	template<typename AT, typename HT>
	inline void		InjectHook(AT address, HT hook, unsigned int nType)
	{
		intptr_t		dwHook;
		_asm
		{
			mov		eax, hook
			mov		dwHook, eax
		}

		*(uint8_t*)address = nType == PATCH_JUMP ? 0xE9 : 0xE8;

		*(ptrdiff_t*)((intptr_t)address + 1) = dwHook - (intptr_t)address - 5;
	}

	template<typename Func, typename AT>
	inline void		ReadCall(AT address, Func& func)
	{
		func = Func(*(ptrdiff_t*)((intptr_t)address+1) + (intptr_t)address + 5);
	}

	namespace DynBase
	{
		template<typename T, typename AT>
		inline void		Patch(AT address, T value)
		{
			Memory::Patch(DynBaseAddress(address), value);
		}

		template<typename AT>
		inline void		Nop(AT address, size_t count)
		{
			Memory::Nop(DynBaseAddress(address), count);
		}

		template<typename AT, typename HT>
		inline void		InjectHook(AT address, HT hook)
		{
			Memory::InjectHook(DynBaseAddress(address), hook);
		}

		template<typename AT, typename HT>
		inline void		InjectHook(AT address, HT hook, unsigned int nType)
		{
			Memory::InjectHook(DynBaseAddress(address), hook, nType);
		}

		template<typename Func, typename AT>
		inline void		ReadCall(AT address, Func& func)
		{
			Memory::ReadCall(DynBaseAddress(address), func);
		}
	};

	namespace VP
	{
		template<typename T, typename AT>
		inline void		Patch(AT address, T value)
		{
			DWORD		dwProtect[2];
			VirtualProtect((void*)address, sizeof(T), PAGE_EXECUTE_READWRITE, &dwProtect[0]);
			Memory::Patch( address, value );
			VirtualProtect((void*)address, sizeof(T), dwProtect[0], &dwProtect[1]);
		}

		template<typename AT>
		inline void		Nop(AT address, size_t count)
		{
			DWORD		dwProtect[2];
			VirtualProtect((void*)address, count, PAGE_EXECUTE_READWRITE, &dwProtect[0]);
			Memory::Nop( address, count );
			VirtualProtect((void*)address, count, dwProtect[0], &dwProtect[1]);
		}

		template<typename AT, typename HT>
		inline void		InjectHook(AT address, HT hook)
		{
			DWORD		dwProtect[2];

			VirtualProtect((void*)((DWORD)address + 1), 4, PAGE_EXECUTE_READWRITE, &dwProtect[0]);
			Memory::InjectHook( address, hook );
			VirtualProtect((void*)((DWORD)address + 1), 4, dwProtect[0], &dwProtect[1]);
		}

		template<typename AT, typename HT>
		inline void		InjectHook(AT address, HT hook, unsigned int nType)
		{
			DWORD		dwProtect[2];

			VirtualProtect((void*)address, 5, PAGE_EXECUTE_READWRITE, &dwProtect[0]);
			Memory::InjectHook( address, hook, nType );
			VirtualProtect((void*)address, 5, dwProtect[0], &dwProtect[1]);
		}

		template<typename Func, typename AT>
		inline void		ReadCall(AT address, Func& func)
		{
			Memory::ReadCall(address, func);
		}

		namespace DynBase
		{
			template<typename T, typename AT>
			inline void		Patch(AT address, T value)
			{
				VP::Patch(DynBaseAddress(address), value);
			}

			template<typename AT>
			inline void		Nop(AT address, size_t count)
			{
				VP::Nop(DynBaseAddress(address), count);
			}

			template<typename AT, typename HT>
			inline void		InjectHook(AT address, HT hook)
			{
				VP::InjectHook(DynBaseAddress(address), hook);
			}

			template<typename AT, typename HT>
			inline void		InjectHook(AT address, HT hook, unsigned int nType)
			{
				VP::InjectHook(DynBaseAddress(address), hook, nType);
			}

			template<typename Func, typename AT>
			inline void		ReadCall(AT address, Func& func)
			{
				Memory::ReadCall(DynBaseAddress(address), func);
			}
		};
	};
};

#ifndef _MEMORY_NO_CRT

#include <forward_list>
#include <tuple>

class ScopedUnprotect
{
public:
	class Section
	{
	public:
		Section( HINSTANCE hInstance, const char* name )
		{
			IMAGE_NT_HEADERS*		ntHeader = (IMAGE_NT_HEADERS*)((BYTE*)hInstance + ((IMAGE_DOS_HEADER*)hInstance)->e_lfanew);
			IMAGE_SECTION_HEADER*	pSection = IMAGE_FIRST_SECTION(ntHeader);

			DWORD VirtualAddress = MAXDWORD;
			SIZE_T VirtualSize = MAXDWORD;
			for ( SIZE_T i = 0, j = ntHeader->FileHeader.NumberOfSections; i < j; ++i, ++pSection )
			{
				if ( strncmp( (const char*)pSection->Name, name, IMAGE_SIZEOF_SHORT_NAME ) == 0 )
				{
					VirtualAddress = (DWORD)hInstance + pSection->VirtualAddress;
					VirtualSize = pSection->Misc.VirtualSize;
					break;
				}
			}

			if ( VirtualAddress == MAXDWORD )
				return;

			SIZE_T QueriedSize = 0;
			while ( QueriedSize < VirtualSize )
			{
				MEMORY_BASIC_INFORMATION MemoryInf;
				DWORD dwOldProtect;

				VirtualQuery( (LPCVOID)(VirtualAddress + QueriedSize), &MemoryInf, sizeof(MemoryInf) );
				VirtualProtect( MemoryInf.BaseAddress, MemoryInf.RegionSize, PAGE_EXECUTE_READWRITE, &dwOldProtect );
				m_queriedProtects.emplace_front( MemoryInf.BaseAddress, MemoryInf.RegionSize, MemoryInf.Protect );
				QueriedSize += MemoryInf.RegionSize;
			}
		};

		~Section()
		{
			for ( auto& it : m_queriedProtects )
			{
				DWORD dwOldProtect;
				VirtualProtect( std::get<0>(it), std::get<1>(it), std::get<2>(it), &dwOldProtect );
			}
		}

	private:
		std::forward_list< std::tuple< LPVOID, SIZE_T, DWORD > >	m_queriedProtects;
	};
};

#endif

#endif

#endif