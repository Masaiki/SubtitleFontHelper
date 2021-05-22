
#include "common.h"
#include "hook.h"

#if WINVER >= 0x0A00
// enable for win10 or later
#define DWRITE3_SUPPORT
#endif

#ifdef DWRITE3_SUPPORT
#include <dwrite_3.h>
#else
#include <dwrite.h>
#endif
#include <detours.h>

static HRESULT(STDMETHODCALLTYPE* pfnRealIDWriteGdiInterop_CreateFontFromLOGFONT)(
	LPVOID _this,
	LOGFONTW const* logFont,
	IDWriteFont** font
	) = nullptr;

static HRESULT STDMETHODCALLTYPE HookedIDWriteGdiInterop_CreateFontFromLOGFONT(
	LPVOID _this,
	LOGFONTW const* logFont,
	IDWriteFont** font
) {
	TryLoadFont(logFont->lfFaceName);
	return pfnRealIDWriteGdiInterop_CreateFontFromLOGFONT(_this, logFont, font);
}

#ifdef DWRITE3_SUPPORT
static HRESULT(STDMETHODCALLTYPE* pfnRealIDWriteGdiInterop1_CreateFontFromLOGFONT)(
	LPVOID _this,
	LOGFONTW const* logFont,
	IDWriteFontCollection* fontCollection,
	IDWriteFont** font
	) = nullptr;

static HRESULT STDMETHODCALLTYPE HookedIDWriteGdiInterop1_CreateFontFromLOGFONT(
	LPVOID _this,
	LOGFONTW const* logFont,
	IDWriteFontCollection* fontCollection,
	IDWriteFont** font
) {
	TryLoadFont(logFont->lfFaceName);
	return pfnRealIDWriteGdiInterop1_CreateFontFromLOGFONT(_this, logFont, fontCollection, font);
}
#endif

typedef HRESULT(WINAPI* PFNDWRITECREATEFACTORY)(
	_In_ DWRITE_FACTORY_TYPE factoryType,
	_In_ REFIID iid,
	_COM_Outptr_ IUnknown** factory
	);

struct IDummyInterface {
	struct DummyVtbl {
		LPVOID* lpFunc[1];
	}*lpVtbl;
};

constexpr size_t IUnknownVtblSize = 3;
constexpr size_t IDWriteGdiInteropVtblSize = 5;

constexpr size_t IDWriteGdiInterop_CreateFontFromLOGFONT_Index = 0;
constexpr size_t IDWriteGdiInterop1_CreateFontFromLOGFONT_Index = 0;

static bool GetDirectWriteFunctionAddress() {
	HMODULE hDWriteDll;
	HRESULT hResult;
	hDWriteDll = LoadLibraryW(L"DWrite.dll");
	if (hDWriteDll == NULL) {
		return false;
	}

	PFNDWRITECREATEFACTORY pfnDWriteCreateFactory;
	pfnDWriteCreateFactory = reinterpret_cast<PFNDWRITECREATEFACTORY>(GetProcAddress(hDWriteDll, "DWriteCreateFactory"));
	if (pfnDWriteCreateFactory == nullptr) {
		return false;
	}

	// Get Factory
	IDWriteFactory* pFactory;
	hResult = pfnDWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), reinterpret_cast<IUnknown**>(&pFactory));
	if (FAILED(hResult)) {
		return false;
	}


	IDWriteGdiInterop* pGdiInterop;
	hResult = pFactory->GetGdiInterop(&pGdiInterop);
	if (FAILED(hResult)) {
		pFactory->Release();
		return false;
	}

	IDummyInterface* pDummy = reinterpret_cast<IDummyInterface*>(pGdiInterop);
	pfnRealIDWriteGdiInterop_CreateFontFromLOGFONT =
		reinterpret_cast<decltype(pfnRealIDWriteGdiInterop_CreateFontFromLOGFONT)>(
			pDummy->lpVtbl->lpFunc[IUnknownVtblSize + IDWriteGdiInterop_CreateFontFromLOGFONT_Index]);

#ifdef DWRITE3_SUPPORT
	IDWriteGdiInterop1* pGdiInterop1;
	hResult = pGdiInterop->QueryInterface(&pGdiInterop1);
	if (SUCCEEDED(hResult)) {
		pDummy = reinterpret_cast<IDummyInterface*>(pGdiInterop1);
		pfnRealIDWriteGdiInterop1_CreateFontFromLOGFONT =
			reinterpret_cast<decltype(pfnRealIDWriteGdiInterop1_CreateFontFromLOGFONT)>(
				pDummy->lpVtbl->lpFunc[IUnknownVtblSize + IDWriteGdiInteropVtblSize + IDWriteGdiInterop1_CreateFontFromLOGFONT_Index]);
		pGdiInterop1->Release();
	}
#endif

	pGdiInterop->Release();
	pFactory->Release();
	return true;

}

#define CheckFunctionPointerAndHook(Func, vHookedFunc) do { \
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

bool DoDirectWriteHook()
{
	LONG lResult = 0;
	std::vector<std::pair<LPVOID*, LPVOID>> vHookedFunc;
	if (!GetDirectWriteFunctionAddress()) {
		return false;
	}


	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	CheckFunctionPointerAndHook(IDWriteGdiInterop_CreateFontFromLOGFONT, vHookedFunc);
#ifdef DWRITE3_SUPPORT
	CheckFunctionPointerAndHook(IDWriteGdiInterop1_CreateFontFromLOGFONT, vHookedFunc);
#endif
	lResult = DetourTransactionCommit();

	if (lResult == NOERROR) {
		g_stInfo.vHookedFunc.insert(g_stInfo.vHookedFunc.end(), vHookedFunc.begin(), vHookedFunc.end());
		return true;
	}
	else {
		return false;
	}
}


