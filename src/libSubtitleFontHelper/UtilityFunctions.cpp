#include "pch.h"

using namespace System;

#include <Windows.h>
#include <vcclr.h>
#include <cstring>
#include <cwchar>

namespace libSubtitleFontHelper
{
	int CALLBACK EnumFontProc(
		const LOGFONTW* lpelfe,
		const TEXTMETRICW* lpntme,
		DWORD				dwFontType,
		LPARAM				lParam)
	{
		*reinterpret_cast<bool*>(lParam) = true;
		return 0;
	}

	public ref class UtilityFunctions
	{
	public:

		static bool IsFontInSystem(const String^ faceName)
		{
			pin_ptr<const wchar_t> wszFaceName = PtrToStringChars(faceName);
			LOGFONTW lf;
			HDC hDC = NULL;
			bool ret = false;

			memset(&lf, 0, sizeof(lf));
			lf.lfCharSet = DEFAULT_CHARSET;
			lf.lfPitchAndFamily = 0;
			std::wcsncpy(lf.lfFaceName, wszFaceName, LF_FACESIZE - 1);

			hDC = CreateCompatibleDC(NULL);
			if (hDC == NULL)
				goto clean;

			EnumFontFamiliesExW(hDC, &lf, EnumFontProc, reinterpret_cast<LPARAM>(&ret), 0);

		clean:
			if (hDC)
				DeleteDC(hDC);
			return ret;
		}
	};
}