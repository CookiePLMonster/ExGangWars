#define WIN32_LEAN_AND_MEAN
#define _CRT_SECURE_NO_WARNINGS
#define NOMINMAX		// Fixes windef.h conflicting with STL min/max

// Target Windows 2000
#define WINVER 0x0500
#define _WIN32_WINNT 0x0500

#include <windows.h>
#include <shlwapi.h>
#include <cstdio>
#include <algorithm>
#include <cstdint>
#include "MemoryMgr.h"

// SA classes
class CRGBA
{
public:
	uint8_t r, g, b, a;

	inline CRGBA() {}

	inline CRGBA(const CRGBA& in)
		: r(in.r), g(in.g), b(in.b), a(in.a)
	{}

	inline CRGBA(const CRGBA& in, uint8_t alpha)
		: r(in.r), g(in.g), b(in.b), a(alpha)
	{}


	inline CRGBA(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha = 255)
		: r(red), g(green), b(blue), a(alpha)
	{}
};

class CZone
{
public:
	char					Name[8];
	char					TranslatedName[8];
	int16_t					x1, y1, z1;
	int16_t					x2, y2, z2;
	uint16_t				nZoneInfoIndex;
	uint8_t					bType;
	uint8_t					bTownID;
};

class CZoneInfo
{
public:
	uint8_t					GangDensity[10];
	uint8_t					DrugDealerCounter;
	CRGBA					ZoneColour;
	bool					unk1 : 1;
	bool					unk2 : 1;
	bool					unk3 : 1;
	bool					unk4 : 1;
	bool					unk5 : 1;
	bool					bUseColour : 1;
	bool					bInGangWar : 1;
	bool					bNoCops : 1;
	uint8_t					flags;
};

// Custom ExGangWars structure
struct tGangInfo
{
	bool					bCanFightWith : 1;
	bool					bShowOnMap	  : 1;
	uint8_t					bRed, bGreen, bBlue;
	uint32_t				nBlipIndex;
};