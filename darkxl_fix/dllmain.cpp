// darkxl_fix by donrevan
// v1.0
// ------------------------
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <cstdlib>
#include <cmath>
#include "Tools.h"
#include "DarkXL.h"

bool g_bNarShadPatch = false;

// Called on mapchange
void __stdcall OnMapChange(const char *pszMapName)
{
	ObjPlayer *pPlayer = *reinterpret_cast<ObjPlayer**>(OFFSET_PLAYER);
	if(pPlayer)
	{
		if(GetCfgValueBool("Reset", "health"))
		{
			pPlayer->health = 100;
		}
		if(GetCfgValueBool("Reset", "shields"))
		{
			pPlayer->shields = 100;
		}
		if(GetCfgValueBool("Reset", "lifes"))
		{
			pPlayer->lifes = 3;
		}
		if(GetCfgValueBool("Reset", "energy"))
		{
			pPlayer->energy = 100;
		}

		// I'm not actually sure if there's any difference to the sound if the patch is
		// enabled or not.. Just 'fix' it on nar shaddaa to be sure we do no harm.
		if(strcmp(pszMapName, "NARSHADA.LEV") == 0)
		{
			if(GetCfgValueBool("General", "narshadfix")) {
				// patch
				BYTE bytes[] = {0x90, 0x90};
				ApplyPatch((PBYTE)OFFSET_IMUSEBRANCH, bytes, sizeof(bytes));
				g_bNarShadPatch = true;
			}

			if(GetCfgValueBool("General", "concussionfix")) {
				pPlayer->AddWeapon(WEAPON_CONCUSSION);
			}
		} else {
			if(g_bNarShadPatch)
			{
				// restore
				BYTE bytes[] = {0x75, 0x05};
				ApplyPatch((PBYTE)OFFSET_IMUSEBRANCH, bytes, sizeof(bytes));
				g_bNarShadPatch = false;
			}
		}
	}
}

/*
004943C2 | D9 45 90                 | fld dword ptr ss:[ebp-70]               |
004943C5 | D9 1C 24                 | fstp dword ptr ss:[esp]                 |
004943C8 | 8B 45 8C                 | mov eax,dword ptr ss:[ebp-74]           | <-- place hook here(len=6)
004943CB | 8B 48 08                 | mov ecx,dword ptr ds:[eax+8]            |
004943CE | E8 FD 0E 02 00           | call darkxl.4B52D0                      |
004943D3 | 68 18 18 5B 00           | push darkxl.5B1818                      | ;5B1818:"Map loaded successfully."
004943D8 | 68 44 03 00 00           | push 344                                |
004943DD | 68 34 18 5B 00           | push darkxl.5B1834                      | ;5B1834:"Game::LoadMap"
*/
DWORD dwJMPBack1;
__declspec(naked) void mapload_patch()
{
	__asm
	{
		pushad
		mov eax, [ebp+8]
		push eax
		call OnMapChange
		popad
		; exec original (overwritten) instructions
		mov eax,dword ptr ss:[ebp-74]
		mov ecx,dword ptr ds:[eax+8]
		jmp dwJMPBack1
	}
}

static float GetDistance(const float a[3], const float b[3])
{
	return sqrt(pow((a[0] - b[0]), 2) + pow((a[1] - b[1]), 2) + pow((a[2] - b[2]), 2));
}

DWORD CALLBACK Start(LPVOID lpArgs)
{
	dwJMPBack1 = InjectJMP((PBYTE)OFFSET_MAPLOAD, (DWORD)mapload_patch, 6);

	// "Alpha 9.50 DarkFX"
	BYTE newString[] = {0x41, 0x6C, 0x70, 0x68, 0x61, 0x20, 0x39, 0x2E, 0x35, 0x30, 0x20, 0x44, 0x61, 0x72, 0x6B, 0x46, 0x58};
	ApplyPatch((PBYTE)OFFSET_WATERMARK, newString, sizeof(newString));

	while(true)
	{
		bool bMortarfix = GetCfgValueBool("General", "mortarfix");
		bool bCannonfix = GetCfgValueBool("General", "cannonfix");
		if(bMortarfix || bCannonfix)
		{
			ObjPlayer *pPlayer = *reinterpret_cast<ObjPlayer**>(OFFSET_PLAYER);
			if(pPlayer && pPlayer->health > 0)
			{
				Sector *pSector = GetSector(pPlayer->sector);
				if(pSector)
				{
					DWORD i=0;
					while(pSector->HasNext(i))
					{
						Object *pObj = pSector->GetObject(i++);
							
						if(pObj->type == TYPE_FRAME)
						{
							ObjFrame *pFrame = reinterpret_cast<ObjFrame*>(pObj);
							if(!pFrame->HasFlag(OFLAGS_INVISIBLE) && GetDistance(pObj->pos, pPlayer->pos) <= 5.0f)
							{
								if(bMortarfix && strcmp(pFrame->name, "IMORTAR") == 0)
								{
									// TODO: screen flash/text?
									pPlayer->AddWeapon(WEAPON_MORTAR);
									pFrame->SetFlag(OFLAGS_INVISIBLE);
								} else if(bCannonfix && strcmp(pFrame->name, "ICANNON") == 0)
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

		Sleep(500);
	}
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
	char buffer[MAX_PATH];
	GetSystemDirectoryA(buffer, MAX_PATH);
	strcat_s(buffer, "\\d3d9.dll");
	return LoadLibraryA(buffer);
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	DisableThreadLibraryCalls(hinstDLL);
	if(fdwReason == DLL_PROCESS_ATTACH)
	{
		HMODULE hLib = GetRealDX9Library();
		if(!hLib) {
			MessageBox(NULL, TEXT("Failed to load real d3d9.dll"), TEXT("darkxl_fix"), MB_OK|MB_ICONERROR|MB_TOPMOST);
			return FALSE;
		}
		oDirect3DCreate9 = (tDirect3DCreate9)GetProcAddress(hLib, TEXT("Direct3DCreate9"));

		CreateThread(0, 0, Start, 0, 0, 0);
	}
	return TRUE;
}