
#include <Windows.h>

#include <detours.h>
#include "Common.h"
#include "Hook.h"

HFONT(WINAPI* True_CreateFontIndirectA)(_In_ CONST LOGFONTA* lplf) = CreateFontIndirectA;
HFONT(WINAPI* True_CreateFontIndirectW)(_In_ CONST LOGFONTW* lplf) = CreateFontIndirectW;
HFONT(WINAPI* True_CreateFontA)(_In_ int cHeight, _In_ int cWidth, _In_ int cEscapement, _In_ int cOrientation, _In_ int cWeight, _In_ DWORD bItalic,
	_In_ DWORD bUnderline, _In_ DWORD bStrikeOut, _In_ DWORD iCharSet, _In_ DWORD iOutPrecision, _In_ DWORD iClipPrecision,
	_In_ DWORD iQuality, _In_ DWORD iPitchAndFamily, _In_opt_ LPCSTR pszFaceName) = CreateFontA;
HFONT(WINAPI* True_CreateFontW)(_In_ int cHeight, _In_ int cWidth, _In_ int cEscapement, _In_ int cOrientation, _In_ int cWeight, _In_ DWORD bItalic,
	_In_ DWORD bUnderline, _In_ DWORD bStrikeOut, _In_ DWORD iCharSet, _In_ DWORD iOutPrecision, _In_ DWORD iClipPrecision,
	_In_ DWORD iQuality, _In_ DWORD iPitchAndFamily, _In_opt_ LPCWSTR pszFaceName) = CreateFontW;
HFONT(WINAPI* True_CreateFontIndirectExA)(_In_ CONST ENUMLOGFONTEXDVA*) = CreateFontIndirectExA;
HFONT(WINAPI* True_CreateFontIndirectExW)(_In_ CONST ENUMLOGFONTEXDVW*) = CreateFontIndirectExW;
int  (WINAPI* True_EnumFontFamiliesW)( _In_ HDC hdc, _In_opt_ LPCWSTR lpLogfont, _In_ FONTENUMPROCW lpProc, _In_ LPARAM lParam) = EnumFontFamiliesW;

#include <string>
#include <memory>

static bool should_detour = true;

BOOL WINAPI DllMain(
	HINSTANCE hinstDLL,  // handle to DLL module
	DWORD fdwReason,     // reason for calling function
	LPVOID lpReserved)  // reserved
{
	// Perform actions based on the reason for calling.
	LONG detour_error;

	switch (fdwReason)
	{
	case DLL_PROCESS_ATTACH:
		// Initialize once for each new process.
		// Return FALSE to fail DLL load.
		should_detour = getenv("NODETOUR") == nullptr;
		if (GetHModulePath(NULL).find(L"rundll32.exe") != std::wstring::npos)should_detour = false;
		if (should_detour) {
			//MessageBoxW(nullptr, L"Injected", L"Info", MB_OK);
			DetourTransactionBegin();
			DetourUpdateThread(GetCurrentThread());
			DetourAttach(&(PVOID&)True_CreateFontW, HookedCreateFontW);
			DetourAttach(&(PVOID&)True_CreateFontA, HookedCreateFontA);
			DetourAttach(&(PVOID&)True_CreateFontIndirectW, HookedCreateFontIndirectW);
			DetourAttach(&(PVOID&)True_CreateFontIndirectA, HookedCreateFontIndirectA);
			DetourAttach(&(PVOID&)True_CreateFontIndirectExW, HookedCreateFontIndirectExW);
			DetourAttach(&(PVOID&)True_CreateFontIndirectExA, HookedCreateFontIndirectExA);
            DetourAttach(&(PVOID&)True_EnumFontFamiliesW, HookedEnumFontFamiliesW);
			detour_error = DetourTransactionCommit();
			InjectNotification(detour_error == NOERROR);
			if (detour_error != NO_ERROR) {
				return FALSE;
			}
		}
		break;

	case DLL_THREAD_ATTACH:
		// Do thread-specific initialization.
		break;

	case DLL_THREAD_DETACH:
		// Do thread-specific cleanup.
		break;

	case DLL_PROCESS_DETACH:
		// Perform any necessary cleanup.
		if (should_detour) {
			DetourTransactionBegin();
			DetourUpdateThread(GetCurrentThread());
			DetourDetach(&(PVOID&)True_CreateFontW, HookedCreateFontW);
			DetourDetach(&(PVOID&)True_CreateFontA, HookedCreateFontA);
			DetourDetach(&(PVOID&)True_CreateFontIndirectW, HookedCreateFontIndirectW);
			DetourDetach(&(PVOID&)True_CreateFontIndirectA, HookedCreateFontIndirectA);
			DetourDetach(&(PVOID&)True_CreateFontIndirectExW, HookedCreateFontIndirectExW);
			DetourDetach(&(PVOID&)True_CreateFontIndirectExA, HookedCreateFontIndirectExA);
            DetourDetach(&(PVOID&)True_EnumFontFamiliesW, HookedEnumFontFamiliesW);
			DetourTransactionCommit();
		}
		break;
	}
	return TRUE;  // Successful DLL_PROCESS_ATTACH.
}

std::wstring GetDLLSelfPath() {
	HMODULE hModule = NULL;
	if (GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
		(wchar_t*)DllMain, &hModule) == 0) {
		return std::wstring();
	}
	return GetHModulePath(hModule);
}

int AdjustPrivilege()
{
	int error_code = ERROR_SUCCESS;
	HANDLE h_token = NULL;
	if (OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &h_token)) {
		LUID dbg_luid;
		if (LookupPrivilegeValueW(nullptr, L"SeDebugPrivilege", &dbg_luid)) {
			TOKEN_PRIVILEGES token_priv;
			token_priv.PrivilegeCount = 1;
			token_priv.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
			token_priv.Privileges[0].Luid = dbg_luid;
			if (AdjustTokenPrivileges(h_token, FALSE, &token_priv, sizeof(TOKEN_PRIVILEGES), nullptr, nullptr)) {
				error_code = GetLastError();
			}
		}
		else {
			error_code = GetLastError();
		}
		CloseHandle(h_token);
	}
	else {
		error_code = GetLastError();
	}
	return error_code;
}

extern "C" void CALLBACK DoInject(HWND hwnd, HINSTANCE hinst, LPSTR lpszCmdLine, int nCmdShow) {
	//MessageBoxW(nullptr, L"Launched", L"Info", MB_OK);
	DWORD target_pid;
	try {
		target_pid = std::stoul(lpszCmdLine);
	}
	catch (std::exception e) {
		return;
	}

	// try to get SeDebugPrivilege
	AdjustPrivilege();

	HANDLE hProcess = NULL;
	HANDLE hThread = NULL;
	HMODULE hMod = NULL;
	LPVOID pRemoteBuf = NULL;
	auto dll_path = GetDLLSelfPath();

	LPTHREAD_START_ROUTINE pThreadProc;

	if (dll_path.empty())return;

	if (!(hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, target_pid))) {
		return;
	}

	size_t buf_size = (dll_path.size() + 1) * sizeof(wchar_t);

	pRemoteBuf = VirtualAllocEx(hProcess, NULL, buf_size, MEM_COMMIT, PAGE_READWRITE);

	WriteProcessMemory(hProcess, pRemoteBuf, (LPVOID)dll_path.c_str(), buf_size, NULL);

	hMod = GetModuleHandleW(L"kernel32.dll");
	pThreadProc = (LPTHREAD_START_ROUTINE)GetProcAddress(hMod, "LoadLibraryW");

	hThread = CreateRemoteThread(hProcess, NULL, 0, pThreadProc, pRemoteBuf, 0, NULL);

	WaitForSingleObject(hThread, INFINITE);
	CloseHandle(hThread);
	CloseHandle(hProcess);

	return;
}