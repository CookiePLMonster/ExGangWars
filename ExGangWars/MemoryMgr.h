#ifndef __MEMORYMGR
#define __MEMORYMGR

enum
{
	PATCH_CALL,
	PATCH_JUMP,
	PATCH_NOTHING,
};

// This function initially detects SA version then chooses the address basing on game version
template<typename T>
inline T AddressByVersion(uint32_t address10, uint32_t address11, uint32_t addressSteam)
{
	static int8_t		bVer = -1;

	if ( bVer == -1 )
	{
		if (*(uint32_t*)0x8252FC == 0x94BF || *(uint32_t*)0x82533C == 0x94BF) bVer = 1;
		else if (*(uint32_t*)0x85EC4A == 0x94BF) bVer = 2;
		else bVer = 0;
	}
	switch ( bVer )
	{
	case 1:
		return (T)address11;
	case 2:
		return (T)addressSteam;
	default:
		return (T)address10;
	}
}

// Memory hacking functions for code places/EXEs without Virtual Protect
namespace Memory
{
	template<typename T, typename AT>
	inline void		Patch(AT address, T value)
	{ *(T*)address = value; }

	template<typename AT>
	inline void		Nop(AT address, uint32_t nCount)
	{ memset((void*)address, 0x90, nCount); }

	template<typename AT, typename HT>
	inline void		InjectHook(AT address, HT hook, uint32_t nType=PATCH_NOTHING)
	{
		switch ( nType )
		{
		case PATCH_JUMP:
			*(uint8_t*)address = 0xE9;
			break;
		case PATCH_CALL:
			*(uint8_t*)address = 0xE8;
			break;
		}
		*(uint32_t*)((uint32_t)address + 1) = (uint32_t)hook - (uint32_t)address - 5;
	}
};

// Memory hacking functions for code places/EXEs with Virtual Protect
namespace MemoryVP
{
	template<typename T, typename AT>
	inline void		Patch(AT address, T value)
	{
		DWORD	dwProtect[2];
		VirtualProtect((void*)address, sizeof(T), PAGE_EXECUTE_READWRITE, &dwProtect[0]);
		*(T*)address = value;
		VirtualProtect((void*)address, sizeof(T), dwProtect[0], &dwProtect[1]);
	}

	template<typename AT>
	inline void		Nop(AT address, uint32_t nCount)
	{
		DWORD		dwProtect[2];
		VirtualProtect((void*)address, nCount, PAGE_EXECUTE_READWRITE, &dwProtect[0]);
		memset((void*)address, 0x90, nCount);
		VirtualProtect((void*)address, nCount, dwProtect[0], &dwProtect[1]);
	}

	template<typename AT, typename HT>
	inline void		InjectHook(AT address, HT hook, uint32_t nType=PATCH_NOTHING)
	{
		DWORD		dwProtect[2];
		switch ( nType )
		{
		case PATCH_JUMP:
			VirtualProtect((void*)address, 5, PAGE_EXECUTE_READWRITE, &dwProtect[0]);
			*(BYTE*)address = 0xE9;
			break;
		case PATCH_CALL:
			VirtualProtect((void*)address, 5, PAGE_EXECUTE_READWRITE, &dwProtect[0]);
			*(BYTE*)address = 0xE8;
			break;
		default:
			VirtualProtect((void*)((uint32_t)address + 1), 4, PAGE_EXECUTE_READWRITE, &dwProtect[0]);
			break;
		}
		*(uint32_t*)((uint32_t)address + 1) = (uint32_t)hook - (uint32_t)address - 5;
		if ( nType == PATCH_NOTHING )
			VirtualProtect((void*)((uint32_t)address + 1), 4, dwProtect[0], &dwProtect[1]);
		else
			VirtualProtect((void*)address, 5, dwProtect[0], &dwProtect[1]);
	}
};

#endif