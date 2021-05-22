#include "common.h"
#include "hook.h"

#include <detours.h>

// real function pointers

static HFONT(WINAPI* pfnRealCreateFontIndirectW)(
	_In_ CONST LOGFONTW* lplf
	);

static HFONT(WINAPI* pfnRealCreateFontIndirectA)(
	_In_ CONST LOGFONTA* lplf
	);

static HFONT(WINAPI* pfnRealCreateFontA)(
	int cHeight,
	int cWidth,
	int cEscapement,
	int cOrientation,
	int cWeight,
	DWORD bItalic,
	DWORD bUnderline,
	DWORD bStrikeOut,
	DWORD iCharSet,
	DWORD iOutPrecision,
	DWORD iClipPrecision,
	DWORD iQuality,
	DWORD iPitchAndFamily,
	LPCSTR pszFaceName
	);

static HFONT(WINAPI* pfnRealCreateFontW)(
	int cHeight,
	int cWidth,
	int cEscapement,
	int cOrientation,
	int cWeight,
	DWORD bItalic,
	DWORD bUnderline,
	DWORD bStrikeOut,
	DWORD iCharSet,
	DWORD iOutPrecision,
	DWORD iClipPrecision,
	DWORD iQuality,
	DWORD iPitchAndFamily,
	LPCWSTR pszFaceName
	);

static HFONT(WINAPI* pfnRealCreateFontIndirectExA)(
	_In_ CONST ENUMLOGFONTEXDVA* lpelfexdv
	);

static HFONT(WINAPI* pfnRealCreateFontIndirectExW)(
	_In_ CONST ENUMLOGFONTEXDVW* lpelfexdv
	);

// detour functions

static HFONT WINAPI HookedCreateFontIndirectW(
	_In_ CONST LOGFONTW* lplf
) {
	TryLoadFont(lplf->lfFaceName);
	HFONT ret = pfnRealCreateFontIndirectW(lplf);
	return ret;
}

static HFONT WINAPI HookedCreateFontIndirectA(
	_In_ CONST LOGFONTA* lplf
) {
	TryLoadFont(lplf->lfFaceName);
	HFONT ret = pfnRealCreateFontIndirectA(lplf);
	return ret;
}
static HFONT WINAPI HookedCreateFontA(
	int cHeight,
	int cWidth,
	int cEscapement,
	int cOrientation,
	int cWeight,
	DWORD bItalic,
	DWORD bUnderline,
	DWORD bStrikeOut,
	DWORD iCharSet,
	DWORD iOutPrecision,
	DWORD iClipPrecision,
	DWORD iQuality,
	DWORD iPitchAndFamily,
	LPCSTR pszFaceName
) {
	TryLoadFont(pszFaceName);
	HFONT ret = pfnRealCreateFontA(
		cHeight, cWidth, cEscapement,
		cOrientation, cWeight, bItalic,
		bUnderline, bStrikeOut, iCharSet,
		iOutPrecision, iClipPrecision, iQuality,
		iPitchAndFamily, pszFaceName);
	return ret;
}

static HFONT WINAPI HookedCreateFontW(int cHeight,
	int cWidth,
	int cEscapement,
	int cOrientation,
	int cWeight,
	DWORD bItalic,
	DWORD bUnderline,
	DWORD bStrikeOut,
	DWORD iCharSet,
	DWORD iOutPrecision,
	DWORD iClipPrecision,
	DWORD iQuality,
	DWORD iPitchAndFamily,
	LPCWSTR pszFaceName
) {
	TryLoadFont(pszFaceName);
	HFONT ret = pfnRealCreateFontW(
		cHeight, cWidth, cEscapement,
		cOrientation, cWeight, bItalic,
		bUnderline, bStrikeOut, iCharSet,
		iOutPrecision, iClipPrecision, iQuality,
		iPitchAndFamily, pszFaceName);
	return ret;
}

static HFONT WINAPI HookedCreateFontIndirectExA(
	_In_ CONST ENUMLOGFONTEXDVA* lpelfexdv
) {
	TryLoadFont((const char*)&lpelfexdv->elfEnumLogfontEx.elfFullName[0]);
	HFONT ret = pfnRealCreateFontIndirectExA(lpelfexdv);
	return ret;
}

static HFONT WINAPI HookedCreateFontIndirectExW(
	_In_ CONST ENUMLOGFONTEXDVW* lpelfexdv
) {
	TryLoadFont((const wchar_t*)&lpelfexdv->elfEnumLogfontEx.elfFullName[0]);
	HFONT ret = pfnRealCreateFontIndirectExW(lpelfexdv);
	return ret;
}

// Hook install function

#define GetFunctionAddressAndHook(hModule, Func, vHookedFunc) do { \
		pfnReal##Func = reinterpret_cast<decltype(pfnReal##Func)>(GetProcAddress(hModule, #Func));		        \
		if(pfnReal##Func == nullptr) {													        				\
/*																												\
			DetourTransactionAbort();																			\
			return false;																						\
*/																												\
			break;																								\
		}																										\
		DetourAttach(&(PVOID&)pfnReal##Func, Hooked##Func);														\
		vHookedFunc.push_back(std::make_pair((LPVOID*)&pfnReal##Func, (LPVOID)&Hooked##Func));					\
	}while(false)

bool DoGdiHook()
{
	LONG lResult = 0;
	std::vector<std::pair<LPVOID*, LPVOID>> vHookedFunc;
	HMODULE hGdi32Dll;

	hGdi32Dll = LoadLibraryW(L"gdi32.dll");
	if (hGdi32Dll == NULL) {
		return false;
	}

	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	GetFunctionAddressAndHook(hGdi32Dll, CreateFontA, vHookedFunc);
	GetFunctionAddressAndHook(hGdi32Dll, CreateFontW, vHookedFunc);
	GetFunctionAddressAndHook(hGdi32Dll, CreateFontIndirectA, vHookedFunc);
	GetFunctionAddressAndHook(hGdi32Dll, CreateFontIndirectW, vHookedFunc);
	GetFunctionAddressAndHook(hGdi32Dll, CreateFontIndirectExA, vHookedFunc);
	GetFunctionAddressAndHook(hGdi32Dll, CreateFontIndirectExW, vHookedFunc);
	lResult = DetourTransactionCommit();

	if (lResult == NOERROR) {
		g_stInfo.vHookedFunc.insert(g_stInfo.vHookedFunc.end(), vHookedFunc.begin(), vHookedFunc.end());
		return true;
	}
	else {
		return false;
	}
}
