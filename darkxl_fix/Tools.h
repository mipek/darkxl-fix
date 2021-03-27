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

static void *DetourFunction(PBYTE addr, PBYTE callback, DWORD len)
{
	PBYTE trampoline = (PBYTE)VirtualAlloc(NULL, len + 5, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
	if (trampoline) {
		// copy first few bytes into trampoline
		memcpy(trampoline, addr, len);
		// jump from trampoline to original function (skip hook)
		PBYTE trampolineJmp = trampoline + len;
		*(trampolineJmp) = 0xE9; // JMP
		*(LPDWORD)(trampolineJmp + 1) = (addr + len - trampolineJmp) - 5;

		// patch original function to jump to our callback
		DWORD oldProt;
		VirtualProtect(addr, len, PAGE_EXECUTE_READWRITE, &oldProt);
		*addr = 0xE9; // JMP
		*(LPDWORD)(addr + 1) = (callback - addr) - 5;
		VirtualProtect(addr, len, oldProt, &oldProt);

		return trampoline;
	}
	return nullptr;
}