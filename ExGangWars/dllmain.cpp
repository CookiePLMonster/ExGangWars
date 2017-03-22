// Extended Gang Wars 1.0 mod
// Made by Silent

// NOTE FOR MINGW USERS
// The plugin requires shlwapi.lib to be linked!

#include "General.h"

// SA global variables
int32_t&		TotalNumberOfNavigationZones = **AddressByVersion<int32_t**>(0x443B06, 0x443B86, 0x447806);
int16_t&		TotalNumberOfZoneInfos = **AddressByVersion<int16_t**>(0x572200, 0x572700, 0x587182);
CZone*			NavigationZoneArray = *AddressByVersion<CZone**>(0x443C1C, 0x443C9C, 0x44791F);
CZoneInfo*		ZoneInfoArray = *AddressByVersion<CZoneInfo**>(0x443C51, 0x443CD1, 0x447965);

float&			TerritoryUnderControlPercentage = **AddressByVersion<float**>(0x443EDB, 0x443F5B, 0x447C6E);
int32_t*		GangRatings = *AddressByVersion<int32_t**>(0x443F07, 0x443F87, 0x447C90);
int32_t*		GangRatingStrength = *AddressByVersion<int32_t**>(0x443F0D, 0x443F8D, 0x447C96);


// SA function pointers
auto CanPlayerStartAGangWarHere = AddressByVersion<bool(*)(CZoneInfo*)>(0x443F80, 0x444000, 0x447D10);
auto GetStatValue = AddressByVersion<float(*)(uint16_t)>(0x558E40, 0x5592E0, 0x571AF0);
auto SetStatValue = AddressByVersion<void(*)(uint16_t,float)>(0x55A070, 0x55A510, 0x572DC0);


// Custom ExGangWars variables
tGangInfo		CustomGangInfo[10];


// This function needs to remove the parameter from the stack by itself
int32_t __stdcall PickDefensiveGang(CZoneInfo* pZone)
{
	// Attacking gang is the strongest 'fightable' gang at this territory
	int32_t		nCurrentStrongestGang = -1;
	int32_t		nStrongestGangStrength = -1;

	for ( int32_t i = 0; i < 10; i++ )
	{
		if ( CustomGangInfo[i].bCanFightWith )
		{
			if ( pZone->GangDensity[i] > nStrongestGangStrength )
			{
				nStrongestGangStrength = pZone->GangDensity[i];
				nCurrentStrongestGang = i;
			}
		}
	}

	// nCurrentStrongestGang should never be -1, but if it is, we'll default to Ballas
	return nCurrentStrongestGang != -1 ? nCurrentStrongestGang : 0;
}

// This function needs to remove the parameter from the stack by itself
bool __stdcall CanNotPedtypeBeProvoked(int32_t nPedType)
{
	// Returns true if this pedtype can NOT be provoked to start a gangwar
	// Obviously, the pedtype has to be a gang first...
	if ( nPedType >= 7 && nPedType <= 16 )
		return CustomGangInfo[nPedType-7].bCanFightWith == false;
	else
		// Not a gang, can never be provoked
		return true;
}

int32_t GetRivalGangsTotalDensity(uint32_t nZoneExtraInfoID)
{
	// We need to get a total strength of all gangs player can fight with
	// so the game can decide whether to start a defensive gang war there
	int32_t		nTotalStrength = 0;

	for ( int32_t i = 0; i < 10; i++ )
	{
		if ( CustomGangInfo[i].bCanFightWith )
			nTotalStrength += ZoneInfoArray[nZoneExtraInfoID].GangDensity[i];
	}
	return nTotalStrength;
}

void FillZonesWithGangColours(bool bDontColour)
{
	if ( TotalNumberOfZoneInfos )
	{
		for ( int16_t i = 0; i < TotalNumberOfZoneInfos; i++ )
		{
			uint32_t	nTotalDensity = 0;
			uint32_t	bRedToPick = 0, bGreenToPick = 0, bBlueToPick = 0;

			for ( int j = 0; j < 10; j++ )
			{
				if ( CustomGangInfo[j].bShowOnMap )
				{
					nTotalDensity += ZoneInfoArray[i].GangDensity[j];
					bRedToPick += CustomGangInfo[j].bRed * ZoneInfoArray[i].GangDensity[j];
					bGreenToPick += CustomGangInfo[j].bGreen * ZoneInfoArray[i].GangDensity[j];
					bBlueToPick += CustomGangInfo[j].bBlue * ZoneInfoArray[i].GangDensity[j];
				}
			}

			ZoneInfoArray[i].bUseColour = nTotalDensity && !bDontColour && CanPlayerStartAGangWarHere(&ZoneInfoArray[i]);
			ZoneInfoArray[i].bInGangWar = false;

			ZoneInfoArray[i].ZoneColour.a = static_cast<uint8_t>(std::min<uint32_t>(120, 3 * nTotalDensity));	// Oh well...

			if ( nTotalDensity )
				ZoneInfoArray[i].ZoneColour.a = std::max<uint8_t>(55, ZoneInfoArray[i].ZoneColour.a);
			else
				nTotalDensity = 1;

			// The result is a simple weighted arithmetic average
			// each gang's RGB having the weight of gang's density in this area
			ZoneInfoArray[i].ZoneColour.r = static_cast<uint8_t>(bRedToPick / nTotalDensity);
			ZoneInfoArray[i].ZoneColour.g = static_cast<uint8_t>(bGreenToPick / nTotalDensity);
			ZoneInfoArray[i].ZoneColour.b = static_cast<uint8_t>(bBlueToPick / nTotalDensity);
		}
	}
}

void UpdateTerritoryUnderControlPercentage()
{
	std::pair<uint8_t,uint8_t>	vecZonesForGang[10];
	int32_t						nTotalTerritories = 0;

	// Initialise the array
	for ( uint8_t i = 1; i < 10; i++ )
		vecZonesForGang[i].first = i;

	if ( TotalNumberOfNavigationZones )
	{
		// Count the turfs belonging to each gang
		for ( int32_t i = 0; i < TotalNumberOfNavigationZones; i++ )
		{
			uint16_t		nZoneInfoIndex = NavigationZoneArray[i].nZoneInfoIndex;
			if ( nZoneInfoIndex )
			{
				// Should we even count this territory?
				bool	bCountMe = false;

				for ( uint32_t j = 0; j < 10; j++ )
				{
					if ( (j == 1 || CustomGangInfo[j].bCanFightWith) && ZoneInfoArray[nZoneInfoIndex].GangDensity[j] )
					{
						bCountMe = true;
						break;
					}
				}

				if ( bCountMe )
				{
					// Instantiate a very temporary array to find what gang has the most influence in this area
					std::pair<uint8_t,uint8_t>		vecGangPopularity[10];
					int32_t							nArrIndex = 0;

					for ( uint32_t j = 0; j < 10; j++ )
					{
						if ( j == 1 || CustomGangInfo[j].bCanFightWith )
							vecGangPopularity[nArrIndex++] = std::make_pair(j, ZoneInfoArray[nZoneInfoIndex].GangDensity[j]);
					}

					// Sort it!
					std::sort(vecGangPopularity, vecGangPopularity+10, [] (const std::pair<uint8_t,uint8_t>& Left, const std::pair<uint8_t,uint8_t>& Right)
							{ return Left.second > Right.second; } );

					// Add to gang's territory counter
					vecZonesForGang[vecGangPopularity[0].first].second++;
					nTotalTerritories++;
				}
			}
		}
	}

	// Update the stats
	SetStatValue(236, vecZonesForGang[1].second);										// NUMBER_TERRITORIES_HELD
	SetStatValue(237, std::max<float>(vecZonesForGang[1].second, GetStatValue(237)));	// HIGHEST_NUMBER_TERRITORIES_HELD

	if ( nTotalTerritories )
	{
		TerritoryUnderControlPercentage = static_cast<float>(vecZonesForGang[1].second) / nTotalTerritories;

		// Sort the array to find top 3 gangs
		std::sort(vecZonesForGang, vecZonesForGang+10, [] (const std::pair<uint8_t,uint8_t>& Left, const std::pair<uint8_t,uint8_t>& Right)
				{ return Left.second > Right.second; } );

		GangRatings[0] = vecZonesForGang[0].first;
		GangRatings[1] = vecZonesForGang[1].first;
		GangRatings[2] = vecZonesForGang[2].first;

		GangRatingStrength[0] = vecZonesForGang[0].second;
		GangRatingStrength[1] = vecZonesForGang[1].second;
		GangRatingStrength[2] = vecZonesForGang[2].second;
	}
	else
		TerritoryUnderControlPercentage = 0.0f;
}

void Patch_SA_10()
{
	using namespace Memory;

	InjectHook(0x572440, FillZonesWithGangColours, PATCH_JUMP);
	InjectHook(0x44665D, UpdateTerritoryUnderControlPercentage);

	// cmp eax, 1 \ jz     loc_4463EC
	Nop(0x446272, 4);
	Patch<uint8_t>(0x446278, 1);
	Patch<uint8_t>(0x44627A, 0x84);

	// push edx \ call CanNotPedtypeBeProvoked \ test al, al
	Patch<uint8_t>(0x443968, 0x52);
	Patch<uint16_t>(0x44396E, 0xC084);
	InjectHook(0x443969, CanNotPedtypeBeProvoked, PATCH_CALL);

	// Allow defensive gang wars in entire state
	Patch<uint16_t>(0x443B9D, 0x65EB);

	// push edx \ push ebx \ call GetRivalGangsTotalDensity \ add esp, 4 \ pop edx \ imul ebx, 11h \ cmp eax, 14h
	Patch<uint16_t>(0x443B55, 0x5352);
	InjectHook(0x443B55 + 2, GetRivalGangsTotalDensity, PATCH_CALL);
	Patch<uint32_t>(0x443B55 + 7, 0x5A04C483);
	Patch<uint32_t>(0x443B55 + 11, 0x8311DB6B);
	Patch<uint32_t>(0x443B55 + 15, 0x909014F8);
	Nop(0x443B55 + 19, 3);

	// lea edx, CTheZones::ZoneInfoArray[ebx] \ push edx \ call PickDefensiveGang
	Patch<uint16_t>(0x443C3B, 0x938D);
	Patch<uint8_t>(0x443C65, 0x52);
	Nop(0x443C6B, 2);
	InjectHook(0x443C66, PickDefensiveGang, PATCH_CALL);

	// mov ecx, CustomGangInfo.nBlipIndex[ecx*8] \ push ecx
	Patch<uint16_t>(0x4443EB, 0x0C8B);
	Patch<uint8_t>(0x4443ED, 0xCD);
	Patch<void*>(0x4443EE, &CustomGangInfo->nBlipIndex);
	Patch<uint8_t>(0x4443F2, 0x51);
	Patch<uint16_t>(0x4443F3, 0x0DEB);

	// mov edx, CustomGangInfo.nColour[eax*8]
	Nop(0x44438C, 6);
	Nop(0x4443B0, 3);
	Patch<uint16_t>(0x444399, 0x148B);
	Patch<uint8_t>(0x44439B, 0xC5);
	Patch<void*>(0x44439C, &CustomGangInfo->bRed);
	Patch<uint16_t>(0x4443A0, 0x09EB);
}

void Patch_SA_11()
{
	using namespace Memory;

	InjectHook(0x572940, FillZonesWithGangColours, PATCH_JUMP);
	InjectHook(0x4466DD, UpdateTerritoryUnderControlPercentage);

	// cmp eax, 1 \ jz     loc_4463EC
	Nop(0x4462F2, 4);
	Patch<uint8_t>(0x4462F8, 1);
	Patch<uint8_t>(0x4462FA, 0x84);

	// push edx \ call CanNotPedtypeBeProvoked \ test al, al
	Patch<uint8_t>(0x4439E8, 0x52);
	Patch<uint16_t>(0x4439EE, 0xC084);
	InjectHook(0x4439E9, CanNotPedtypeBeProvoked, PATCH_CALL);

	// Allow defensive gang wars in entire state
	Patch<uint16_t>(0x443C1D, 0x65EB);

	// push edx \ push ebx \ call GetRivalGangsTotalDensity \ add esp, 4 \ pop edx \ imul ebx, 11h \ cmp eax, 14h
	Patch<uint16_t>(0x443BD5, 0x5352);
	InjectHook(0x443BD5 + 2, GetRivalGangsTotalDensity, PATCH_CALL);
	Patch<uint32_t>(0x443BD5 + 7, 0x5A04C483);
	Patch<uint32_t>(0x443BD5 + 11, 0x8311DB6B);
	Patch<uint32_t>(0x443BD5 + 15, 0x909014F8);
	Nop(0x443BD5 + 19, 3);

	// lea edx, CTheZones::ZoneInfoArray[ebx] \ push edx \ call PickDefensiveGang
	Patch<uint16_t>(0x443CBB, 0x938D);
	Patch<uint8_t>(0x443CE5, 0x52);
	Nop(0x443CEB, 2);
	InjectHook(0x443CE6, PickDefensiveGang, PATCH_CALL);

	// mov ecx, CustomGangInfo.nBlipIndex[ecx*8] \ push ecx
	Patch<uint16_t>(0x44446B, 0x0C8B);
	Patch<uint8_t>(0x44446D, 0xCD);
	Patch<void*>(0x44446E, &CustomGangInfo->nBlipIndex);
	Patch<uint8_t>(0x444472, 0x51);
	Patch<uint16_t>(0x444473, 0x0DEB);

	// mov edx, CustomGangInfo.nColour[eax*8]
	Nop(0x44440C, 6);
	Nop(0x444430, 3);
	Patch<uint16_t>(0x444419, 0x148B);
	Patch<uint8_t>(0x44441B, 0xC5);
	Patch<void*>(0x44441C, &CustomGangInfo->bRed);
	Patch<uint16_t>(0x444420, 0x09EB);
}

void Patch_SA_Steam()
{
	using namespace Memory;

	InjectHook(0x587400, FillZonesWithGangColours, PATCH_JUMP);
	InjectHook(0x44A4ED, UpdateTerritoryUnderControlPercentage);

	// cmp ecx, 1 \ jz     loc_44A2DA
	Nop(0x44A17C, 4);
	Patch<uint8_t>(0x44A182, 1);
	Patch<uint8_t>(0x44A184, 0x84);

	// push esi \ call CanNotPedtypeBeProvoked \ test al, al
	Patch<uint8_t>(0x447627, 0x56);
	Patch<uint16_t>(0x44762D, 0xC084);
	InjectHook(0x447628, CanNotPedtypeBeProvoked, PATCH_CALL);

	// Allow defensive gang wars in entire state
	Patch<uint16_t>(0x4478AA, 0x55EB);

	// add ebp, eax \ push edx \ push eax \ call GetRivalGangsTotalDensity \ add esp, 4 \ pop edx \ cmp eax, 14h
	Patch<uint32_t>(0x44785A, 0x5052C501);
	Patch<uint32_t>(0x447863, 0x5A04C483);
	Patch<uint8_t>(0x44786E, 0xF8);
	Nop(0x447867, 6);
	InjectHook(0x44785E, GetRivalGangsTotalDensity, PATCH_CALL);

	// lea edx, CTheZones::ZoneInfoArray[ebp] \ push edx \ call PickDefensiveGang
	Patch<uint16_t>(0x447944, 0x958D);
	Patch<uint8_t>(0x447956, 0x52);
	Nop(0x44795C, 2);
	InjectHook(0x447957, PickDefensiveGang, PATCH_CALL);

	// mov ecx, CustomGangInfo.nBlipIndex[ecx*8] \ push ecx
	Patch<uint16_t>(0x4481B7, 0x0C8B);
	Patch<uint8_t>(0x4481B9, 0xCD);
	Patch<void*>(0x4481BA, &CustomGangInfo->nBlipIndex);
	Patch<uint8_t>(0x4481BE, 0x51);
	Patch<uint16_t>(0x4481BF, 0x0DEB);

	// mov ecx, CustomGangInfo.nColour[eax*8]
	Nop(0x448157, 6);
	Nop(0x44817C, 3);
	Patch<uint16_t>(0x44816B, 0x0C8B);
	Patch<uint8_t>(0x44816D, 0xC5);
	Patch<void*>(0x44816E, &CustomGangInfo->bRed);
	Nop(0x448172, 4);
}

// extern "C" is not really needed there - it's only added for MinGW's sake
// When using Visual Studio, no need to use it
extern "C" BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID lpReserved)
{
	UNREFERENCED_PARAMETER(hModule);
	UNREFERENCED_PARAMETER(lpReserved);
	if(reason == DLL_PROCESS_ATTACH)
	{
		ScopedUnprotect::Section Protect( GetModuleHandle( nullptr ), ".text" );

		if (*(uint32_t*)0x82457C == 0x94BF || *(uint32_t*)0x8245BC == 0x94BF) Patch_SA_10();
		else if (*(uint32_t*)0x8252FC == 0x94BF || *(uint32_t*)0x82533C == 0x94BF) Patch_SA_11();
		else if (*(uint32_t*)0x85EC4A == 0x94BF) Patch_SA_Steam();
		else return TRUE;

		// Parse the INI file
		for ( int32_t i = 0; i < 10; i++ )
		{
			char		sectionName[64];
			char		hexColourBuf[32];
			uint32_t	turfColour;

			_snprintf(sectionName, sizeof(sectionName), "Gang%d", i+1);

			if ( i != 1 )
				CustomGangInfo[i].bCanFightWith = GetPrivateProfileInt(sectionName, "CanFightWith", TRUE, ".\\ExGangWars.ini") != FALSE;
			else	// For Groves, you can't fight with them, no matter what the player sets
				CustomGangInfo[i].bCanFightWith = false;

			// Get turf RGB colours
			GetPrivateProfileString(sectionName, "TurfColour", nullptr, hexColourBuf, sizeof(hexColourBuf), ".\\ExGangWars.ini");

			StrToIntEx(hexColourBuf, STIF_SUPPORT_HEX, reinterpret_cast<int*>(&turfColour));

			// Is nonzero?
			if ( turfColour )
			{
				CustomGangInfo[i].bRed = static_cast<uint8_t>(turfColour >> 16);
				CustomGangInfo[i].bGreen = (turfColour >> 8) & 0xFF;
				CustomGangInfo[i].bBlue = turfColour & 0xFF;

				CustomGangInfo[i].bShowOnMap = true;
			}
			else
				CustomGangInfo[i].bShowOnMap = false;

			// Get an appropriate blip ID, default to 19 if not present in the INI
			CustomGangInfo[i].nBlipIndex = GetPrivateProfileInt(sectionName, "AttackBlip", 19, ".\\ExGangWars.ini");
		}

	}
	return TRUE;
}
