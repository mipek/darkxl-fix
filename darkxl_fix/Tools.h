#pragma once

static bool GetCfgValueBool(const char *section, const char *key)
{
	char buffer[32];
	DWORD len = GetPrivateProfileStringA(section, key, "", buffer, 32, ".\\darkxl_fix.ini");
	if(len > 0)
	{
		if (_stricmp(buffer, "true") == 0 || _stricmp(buffer, "enable") == 0 ||
			_stricmp(buffer, "yes") == 0 || _stricmp(buffer, "y") == 0)
		{
			return true;
		}

		return atoi(buffer) != 0 ? true : false;
	}

	return false;
}

static void ApplyPatch(PBYTE addr, PBYTE bytes, size_t len)
{
	DWORD dwOldProt;
	VirtualProtect(addr, len, PAGE_EXECUTE_READWRITE, &dwOldProt);
	memcpy(addr, bytes, len);
	VirtualProtect(addr, len, dwOldProt, &dwOldProt);
}

static DWORD InjectJMP(PBYTE addr, DWORD to, DWORD len)
{
	// patch in jump
	DWORD oldProt;
	VirtualProtect(addr, len, PAGE_EXECUTE_READWRITE, &oldProt);
	*addr = 0xE9; // JMP
	*((DWORD*)(addr+1)) = (to - (DWORD)addr) - 5; // rel32
	VirtualProtect(addr, len, oldProt, &oldProt);
	return (DWORD)addr + len;
}