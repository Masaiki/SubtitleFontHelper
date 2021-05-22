
#include "common.h"

#include "init.h"

bool ShouldHookFunctions() {
	return GetEnvironmentVariableW(L"__HookHelperProcess", nullptr, 0) == 0;
}

BOOL APIENTRY DllMain(HMODULE hModule,
	DWORD  dwReason,
	LPVOID lpReserved
)
{
	switch (dwReason)
	{
	case DLL_PROCESS_ATTACH:
		if (ShouldHookFunctions()) {
			HANDLE hInitThread = NULL;
			hInitThread = CreateThread(NULL, 0,
				&DeferredInit, nullptr,
				0, NULL);
			// The newly create thread would block until DllMain returns, 
			// since DllMain requires a Loader Lock
			if (hInitThread) {
				// no longer needed handle
				CloseHandle(hInitThread);
			}
		}
		break;
	case DLL_THREAD_ATTACH:
		break;
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
		CleanUpHookLib();
		break;
	default:
		break;
	}
	return TRUE;
}

