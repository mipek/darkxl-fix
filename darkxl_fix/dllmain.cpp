// darkxl_fix by Michael Pekar
// v1.1
// ---------------------------
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <cstdlib>
#include <cmath>
#include "Tools.h"
#include "DarkXL.h"

typedef void (__thiscall *tLoadMap)(void *thisptr, const char *mapName);
tLoadMap oLoadMap;

typedef int (*tSoundSystemUpdate)();
tSoundSystemUpdate oSoundSystemUpdate;

bool g_bNarShadPatch = false;
ULONGLONG g_dwLastUpdateTick = 0;

static float GetDistance(const float a[3], const float b[3])
{
	return sqrt(pow((a[0] - b[0]), 2) + pow((a[1] - b[1]), 2) + pow((a[2] - b[2]), 2));
}

void __fastcall Hook_LoadMap(void *ecx, void *edx, const char* mapName)
{
	oLoadMap(ecx, mapName);

	ObjPlayer *pPlayer = *reinterpret_cast<ObjPlayer**>(OFFSET_PLAYER);
	if (pPlayer)
	{
		if (GetCfgValueBool("Reset", "health"))
		{
			pPlayer->health = 100;
		}
		if (GetCfgValueBool("Reset", "shields"))
		{
			pPlayer->shields = 100;
		}
		if (GetCfgValueBool("Reset", "lifes"))
		{
			pPlayer->lifes = 3;
		}
		if (GetCfgValueBool("Reset", "energy"))
		{
			pPlayer->energy = 100;
		}

		// I'm not actually sure if there's any difference to the sound if the patch is
		// enabled or not.. Just 'fix' it on nar shaddaa to be sure we do no harm.
		if (strcmp(mapName, "NARSHADA.LEV") == 0)
		{
			if (GetCfgValueBool("General", "narshadfix")) {
				// patch
				BYTE bytes[] = { 0x90, 0x90 };
				ApplyPatch((PBYTE)OFFSET_IMUSEBRANCH, bytes, sizeof(bytes));
				g_bNarShadPatch = true;
			}

			if (GetCfgValueBool("General", "concussionfix")) {
				pPlayer->AddWeapon(WEAPON_CONCUSSION);
			}
		}
		else {
			if (g_bNarShadPatch)
			{
				// restore
				BYTE bytes[] = { 0x75, 0x05 };
				ApplyPatch((PBYTE)OFFSET_IMUSEBRANCH, bytes, sizeof(bytes));
				g_bNarShadPatch = false;
			}
		}
	}
}

int Hook_SoundSystemUpdate()
{
	int returnValue = oSoundSystemUpdate();

	ULONGLONG current = GetTickCount64();
	if (current - g_dwLastUpdateTick > 500)
	{
		bool bMortarfix = GetCfgValueBool("General", "mortarfix");
		bool bCannonfix = GetCfgValueBool("General", "cannonfix");
		if (bMortarfix || bCannonfix)
		{
			ObjPlayer *pPlayer = ObjPlayer::GetLocalPlayer();
			if (pPlayer && pPlayer->health > 0)
			{
				Sector *pSector = SectorTable::GetInstance()->GetSector(pPlayer->sector);
				if (pSector)
				{
					size_t index = 0;
					while (pSector->HasNext(index))
					{
						Object *pObj = pSector->GetObject(index++);

						if (pObj->type == TYPE_FRAME)
						{
							ObjFrame *pFrame = reinterpret_cast<ObjFrame*>(pObj);
							if (!pFrame->HasFlag(OFLAGS_INVISIBLE) && GetDistance(pObj->pos, pPlayer->pos) <= 5.0f)
							{
								if (bMortarfix && strcmp(pFrame->name, "IMORTAR") == 0)
								{
									// TODO: screen flash/text?
									pPlayer->AddWeapon(WEAPON_MORTAR);
									pFrame->SetFlag(OFLAGS_INVISIBLE);
								}
								else if (bCannonfix && strcmp(pFrame->name, "ICANNON") == 0)
								{
									// TODO: screen flash/text?
									pPlayer->AddWeapon(WEAPON_CANNON);
									pFrame->SetFlag(OFLAGS_INVISIBLE);
								}
							}
						}
					}
				}
			}
		}

		g_dwLastUpdateTick = current;
	}

	return returnValue;
}

DWORD CALLBACK Start(LPVOID lpArgs)
{
	oLoadMap = (tLoadMap)DetourFunction((PBYTE)OFFSET_MAPLOAD, (PBYTE)Hook_LoadMap, 5);
	oSoundSystemUpdate = (tSoundSystemUpdate)DetourFunction((PBYTE)OFFSET_SOUNDSYSUPDATE, (PBYTE)Hook_SoundSystemUpdate, 6);
	if (!oLoadMap || !oSoundSystemUpdate)
	{
		MessageBox(NULL, TEXT("Failed to setup hooks!"), TEXT("darkxl_fix"), MB_OK | MB_ICONERROR | MB_TOPMOST);
		return 1;
	}

	// Change watermark to "Alpha 9.50 Fix1.1" (so we know if the patch was loaded successfully)
	BYTE newString[] = {'A', 'l', 'p', 'h', 'a', ' ', '9', '.', '5', '0', ' ', 'F', 'i', 'x', '1', '.', '1'};
	ApplyPatch((PBYTE)OFFSET_WATERMARK, newString, sizeof(newString));

	return 0;
}

// d3d9.dll proxy func
class IDirect3D9;
typedef IDirect3D9* (WINAPI *tDirect3DCreate9)(UINT);
tDirect3DCreate9 oDirect3DCreate9 = NULL;
extern "C" __declspec(dllexport) IDirect3D9 *WINAPI Direct3DCreate9(UINT SDKVersion)
{
	return oDirect3DCreate9(SDKVersion);
}

// Returns handle to the "real" d3d9.dll
static HMODULE GetRealDX9Library()
{
	WCHAR buffer[MAX_PATH];
	GetSystemDirectoryW(buffer, MAX_PATH);
	wcscat_s(buffer, L"\\d3d9.dll");
	return LoadLibraryW(buffer);
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	DisableThreadLibraryCalls(hinstDLL);
	if(fdwReason == DLL_PROCESS_ATTACH)
	{
		HMODULE hLib = GetRealDX9Library();
		if(!hLib)
		{
			MessageBox(NULL, TEXT("Failed to load real d3d9.dll"), TEXT("darkxl_fix"), MB_OK|MB_ICONERROR|MB_TOPMOST);
			return FALSE;
		}
		oDirect3DCreate9 = (tDirect3DCreate9)GetProcAddress(hLib, TEXT("Direct3DCreate9"));

		CreateThread(0, 0, Start, 0, 0, 0);
	}
	return TRUE;
}