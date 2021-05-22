#include "common.h"
#include "init.h"
#include "message.h"
#include "hook.h"

#include <detours.h>

typedef NTSTATUS(__stdcall*PFNNTRESUMEPROCESS)(HANDLE hProcess);

static PFNNTRESUMEPROCESS pfnNtResumeProcess;

DWORD __stdcall DeferredInit(LPVOID lpData)
{
	// Get NtResumeProcess
	HMODULE hNtDll = GetModuleHandleW(L"ntdll.dll");
	if (hNtDll == NULL) {
		MessageBoxW(NULL, L"Critical Error: Unable to get NtResumeProcess function.", L"Error", MB_OK | MB_ICONERROR);
		ExitProcess(-1);
	}
	pfnNtResumeProcess = reinterpret_cast<PFNNTRESUMEPROCESS>(GetProcAddress(hNtDll, "NtResumeProcess"));

	// Init
	AttachToDaemon(g_stInfo.pRpcInfo.get());
	
	DoGdiHook();
	DoDirectWriteHook();
	FlushInstructionCache(GetCurrentProcess(), NULL, NULL);

	// Resume Process
	pfnNtResumeProcess(GetCurrentProcess());

	return 0;
}

void CleanUpHookLib()
{
	LONG lResult = 0;
	
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	for (auto& func : g_stInfo.vHookedFunc) {
		DetourDetach(func.first, func.second);
	}
	lResult = DetourTransactionCommit();
	FlushInstructionCache(GetCurrentProcess(), NULL, NULL);

	DetachFromDaemon(g_stInfo.pRpcInfo.get());
}
