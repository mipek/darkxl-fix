#pragma once

#include <Windows.h>

/* Object types */
#define TYPE_PLAYER -1
#define TYPE_3D 0
#define TYPE_SPRITE 1
#define TYPE_FRAME 2

/* Object flags(copied from script) */
#define OFLAGS_COLLIDE_PLAYER	  1
#define OFLAGS_COLLIDE_PROJECTILE 2
#define OFLAGS_COLLIDE_OBJECTS    8
#define OFLAGS_COLLIDE_PICKUP	  16
#define OFLAGS_COLLIDE_PROXIMITY  32
#define OFLAGS_ENEMY			  64
#define OFLAGS_INVISIBLE		  1024

/* General items */
#define ITEM_REDKEY 0x1
#define ITEM_YELLOWKEY 0x2
#define ITEM_BLUEKEY 0x4
#define ITEM_PLANS 0x40
#define ITEM_PHRIK 0x80
#define ITEM_NAVA_CARD 0x100
#define ITEM_DATATAPE 0x200
#define ITEM_DT_WEAPON 0x800
#define ITEM_CODE1 0x800
#define ITEM_CODE2 0x2000
#define ITEM_CODE3 0x4000
#define ITEM_CODE4 0x8000
#define ITEM_CODE5 0x10000
#define ITEM_CODE6 0x20000
#define ITEM_CODE7 0x40000
#define ITEM_CODE8 0x80000
#define ITEM_CODE9 0x100000

/* Weapons */
#define WEAPON_FISTS 0x1
#define WEAPON_BRYAR 0x2
#define WEAPON_RIFLE 0x4
#define WEAPON_DETONATOR 0x8
#define WEAPON_REPETEAR 0x10
#define WEAPON_FUSION 0x20
#define WEAPON_MINES 0x40
#define WEAPON_MORTAR 0x80
#define WEAPON_CONCUSSION 0x100
#define WEAPON_CANNON 0x200

// Global variable that holds a pointer to the player
#define OFFSET_PLAYER 0x073A4560
// (huge ass)Sectortable
#define OFFSET_SECTORTBL 0x06F93FF4
// Function responsible for map loading --> "Loading Map: %s"
#define OFFSET_MAPLOAD 0x004943C2 // midfunchook
// iMUSE player branch instruction that is related to the nar shaddaa crash
#define OFFSET_IMUSEBRANCH 0x004B6D5D
// "Alpha Build 9.50" string
#define OFFSET_WATERMARK 0x005B217C

class Object
{
public:
char _0x0000[56];
	float pos[3]; //0x0038 
char _0x0044[94];
	WORD flags; //0x00A2 
char _0x00A4[8];
	BYTE refcount; //0x00AC 
char _0x00AD[19];
	DWORD type; //0x00C0 
char _0x00C4[108];

	void SetFlag(DWORD flag)
	{
		flags |= flag;
	}
	bool HasFlag(DWORD flag)
	{
		return flags & flag;
	}
};//Size=0x0130 (not sure)


class ObjSprite: public Object
{
public:
	DWORD unk;
	char name[32];
};

class ObjFrame: public Object
{
public:
	DWORD unk;
	char name[32];
};

class ObjPlayer //: Object
{
public:
char _0x0000[56];
	float pos[3]; //0x0038 
char _0x0044[68];
	float superchargeTime; //0x0088 
	float timeLastBeep; //0x008C 
char _0x0090[10];
	WORD health; //0x009A 
	WORD shields; //0x009C 
	WORD energy; //0x009E 
char _0x00A0[2];
	WORD N0F56840F; //0x00A2 
char _0x00A4[20];
	BYTE currentWeapon; //0x00B8 
char _0x00B9[11];
	DWORD sector; //0x00C4 
char _0x00C8[156];
	DWORD itemBits; //0x0164 
	DWORD weaponBits; //0x0168 
char _0x016C[8];
	WORD energyUnits; //0x0174 
	WORD detonatorAmmo; //0x0176 
	WORD powerCells; //0x0178 
	WORD mines; //0x017A 
	WORD mortarShells; //0x017C 
	WORD plasmaUnits; //0x017E 
	WORD rockets; //0x0180 
char _0x0182[14];
	BYTE lifes; //0x0190 
char _0x0191[1];
	WORD N0F218FF2; //0x0192 
	BYTE lights; //0x0194 
char _0x0195[2];
	BYTE N0F2A98A0; //0x0197 
char _0x0198[408];
	BYTE N0F48024C; //0x0330 
char _0x0331[27];

	void AddItem(DWORD item)
	{
		itemBits |= item;
	}

	void AddWeapon(DWORD weapon)
	{
		weaponBits |= weapon;
	}
};

// @TODO: get rid of the asm(lazy way) and do a proper class
class Sector
{
public:
	Object *GetObject(DWORD index)
	{
		__asm
		{
			mov edx, index
			mov eax, [ecx+0x7459]
			mov eax, [eax+edx*4]
		}
	}
	bool HasNext(DWORD index)
	{
		__asm
		{
			mov esi, index
			mov edx, [ecx+0x7459]
			lea edx, [edx+esi*4]
			mov eax, [ecx+0x745D]
			cmp eax, edx
			jz no
		}
		return true;
		__asm
		{
			no:
			xor eax, eax
		}
	}
	
};

Sector *GetSector(DWORD sectorId)
{
	_asm
	{
		mov eax, sectorId
		imul eax, eax, 0x74CF
		add eax, dword ptr ds:[OFFSET_SECTORTBL]
	}
}