
#include "pch.h"

#define generic __identifier(generic)

#include <ft2build.h>
#include <string>

#include FT_TRUETYPE_TAGS_H
#include FT_TRUETYPE_TABLES_H
#include FT_TRUETYPE_IDS_H
#include FT_SFNT_NAMES_H
#include FT_ERRORS_H
#include FT_FONT_FORMATS_H

#undef generic

using namespace System;
using namespace System::Text;
using namespace System::Collections::Generic;

#include <vector>
#include <string>
#include <codecvt>
#include <algorithm>

#include <cliext/vector>
#include <cliext/algorithm>

#include <Windows.h>
#include <vcclr.h>

namespace libSubtitleFontHelper
{
	public ref class FTFontFaceInfo
	{
	public:

		enum class ReaderStatus :int
		{
			OK = 0x0,
			FreeTypeError = 0x1,
			NotSupportedFormat = 0x2,
			NonstandardNameDetected = 0x4
		};

		property ReaderStatus Status;
		property List<String^>^ FullName;
		property List<String^>^ Win32FamilyName;
		property List<String^>^ TypographicFamilyName;
		property String^ PostScriptName;

		FTFontFaceInfo()
		{
			Status = ReaderStatus::OK;
		}
	};

	/// <summary>
	/// Wrapped FT_Library.
	/// A FTContext should only be used in only one thread.
	/// </summary>
	public ref class FTContext
	{
	public:
		FTContext()
		{
			pin_ptr<FT_Library> _lib = &_pLib;
			FT_Error err = FT_Init_FreeType(_lib);
			if (err != FT_Err_Ok)
			{
				throw gcnew Exception("Freetype initialization failed");
			}
		}

		~FTContext()
		{
			FT_Done_FreeType(_pLib);
		}

		FT_Library GetFTLibrary()
		{
			return _pLib;
		}
	private:
		FT_Library _pLib;
	};

	public ref class FTFontFileMap
	{
	public:
		/// <summary>
		/// Replace current file with new file.
		/// Return true for success, false for failure
		/// </summary>
		/// <param name="path">path to file</param>
		bool OpenFile(const String^ path)
		{
			CloseFile();

			pin_ptr<const wchar_t> wszPath = PtrToStringChars(path);
			LARGE_INTEGER li;
			MEMORY_BASIC_INFORMATION mbi;

			_hFile = CreateFileW(wszPath, GENERIC_READ, FILE_SHARE_READ,
				NULL, OPEN_EXISTING, NULL,
				NULL);
			if (_hFile == INVALID_HANDLE_VALUE)
				goto failure;

			_hMapping = CreateFileMappingW(_hFile, NULL, PAGE_READONLY,
				0, 0, nullptr);
			if (_hMapping == NULL)
				goto failure;

			_pMem = MapViewOfFile(_hMapping, FILE_MAP_READ, 0,
				0, 0);
			if (_pMem == nullptr)
				goto failure;

			if (GetFileSizeEx(_hFile, &li) != 0) {
				_nFileSize = static_cast<size_t>(li.QuadPart);
			}
			else {
				if (VirtualQuery(_hMapping, &mbi, sizeof(mbi)) == 0) {
					goto failure;
				}
				_nFileSize = mbi.RegionSize;
			}

			return true;
		failure:
			CloseFile();
			return false;
		}

		/// <summary>
		/// Close any file mapping if exists
		/// </summary>
		void CloseFile()
		{
			if (_pMem)
				UnmapViewOfFile(_pMem);
			if (_hMapping != NULL)
				CloseHandle(_hMapping);
			if (_hFile != INVALID_HANDLE_VALUE)
				CloseHandle(_hFile);
			_pMem = nullptr;
			_hMapping = NULL;
			_hFile = INVALID_HANDLE_VALUE;
		}

		/// <summary>
		/// Return true for opened file present, or return false
		/// </summary>
		bool IsOpened()
		{
			return _pMem != nullptr;
		}

		size_t GetFileSize()
		{
			return _nFileSize;
		}

		void* GetMappedPtr()
		{
			return _pMem;
		}

		~FTFontFileMap()
		{
			CloseFile();
		}
	private:
		HANDLE _hFile = INVALID_HANDLE_VALUE;
		HANDLE _hMapping = NULL;
		void* _pMem = nullptr;
		size_t _nFileSize = 0;
	};

	public ref class FTFontReader
	{
	public:
		FTFontReader(FTContext^ context, FTFontFileMap^ map)
		{
			_ctx = context;
			_map = map;
			_pbConv = new std::vector<wchar_t>(128);
		}

		int GetFontFaceCount()
		{
			FT_Face face = nullptr;
			FT_Error err;
			int n;
			err = FT_New_Memory_Face(_ctx->GetFTLibrary(), reinterpret_cast<FT_Byte*>(_map->GetMappedPtr()), _map->GetFileSize(),
				-1, &face);
			if (err != FT_Err_Ok)
				return -1;

			n = face->num_faces;

			FT_Done_Face(face);
			return n;
		}

		FTFontFaceInfo^ GetFaceInfo(int n)
		{
			FTFontFaceInfo^ faceInfo = gcnew FTFontFaceInfo();
			bool bBadNames = false;
			FT_Face face;
			FT_Error err;
			FT_SfntName name;
			int nNameCount = 0;
			const char* szPostScriptName = nullptr;
			std::vector<std::wstring> vFullNames;
			std::vector<std::wstring> vWin32FamilyNames;
			std::vector<std::wstring> vTypographicNames;
			std::vector<std::wstring>* pvStore = nullptr;

			err = FT_New_Memory_Face(_ctx->GetFTLibrary(), reinterpret_cast<FT_Byte*>(_map->GetMappedPtr()), _map->GetFileSize(),
				n, &face);
			if (err != FT_Err_Ok)
			{
				faceInfo->Status = faceInfo->Status | FTFontFaceInfo::ReaderStatus::FreeTypeError;
				goto cleanup;
			}

			if (!IsLegalFontFace(face))
			{
				faceInfo->Status = faceInfo->Status | FTFontFaceInfo::ReaderStatus::NotSupportedFormat;
				goto cleanup;
			}

			szPostScriptName = FT_Get_Postscript_Name(face);
			if (szPostScriptName != nullptr)
				faceInfo->PostScriptName = gcnew String(szPostScriptName);

			nNameCount = FT_Get_Sfnt_Name_Count(face);

			for (int i = 0; i < nNameCount; ++i)
			{
				err = FT_Get_Sfnt_Name(face, i, &name);
				if (err != FT_Err_Ok)
					continue;

				if (name.platform_id != TT_PLATFORM_MICROSOFT)
					continue;

				switch (name.name_id)
				{
				case TT_NAME_ID_FULL_NAME:
					pvStore = &vFullNames; break;
				case TT_NAME_ID_FONT_FAMILY:
					pvStore = &vWin32FamilyNames; break;
				case TT_NAME_ID_TYPOGRAPHIC_FAMILY:
					pvStore = &vTypographicNames; break;
				default:
					continue;
				}

				std::wstring wsName = ConvertName(name, bBadNames);
				if (wsName.empty())continue;
				pvStore->push_back(std::move(wsName));
			}

			DistinctList(vFullNames);
			DistinctList(vWin32FamilyNames);
			DistinctList(vTypographicNames);

			if (vTypographicNames.empty())
			{
				vTypographicNames.assign(vWin32FamilyNames.begin(), vWin32FamilyNames.end());
			}

			faceInfo->FullName = ConvertVectorToCliList(vFullNames);
			faceInfo->Win32FamilyName = ConvertVectorToCliList(vWin32FamilyNames);
			faceInfo->TypographicFamilyName = ConvertVectorToCliList(vTypographicNames);

			if (bBadNames)
				faceInfo->Status = faceInfo->Status | FTFontFaceInfo::ReaderStatus::NonstandardNameDetected;

		cleanup:
			if (face)
				FT_Done_Face(face);
			return faceInfo;
		}
	private:
		/// <summary>
		/// Check if a font face is supported
		/// </summary>
		/// <param name="n">face index</param>
		static bool IsLegalFontFace(FT_Face face)
		{
			const char* szFmt = FT_Get_Font_Format(face);
			if (!szFmt)return false;
			if (std::strcmp(szFmt, "TrueType") == 0) {
				// OK
			}
			else if (std::strcmp(szFmt, "CFF") == 0) {
				// OK
			}
			else {
				return false;
			}

			if (!(face->face_flags & FT_FACE_FLAG_SFNT)) {
				return false;
			}
			return true;
		}

		/// <summary>
		/// Convert encoding_id to windows codepages
		/// </summary>
		/// <param name="uEnc"></param>
		/// <returns></returns>
		static UINT GetCodepage(FT_UInt uEnc)
		{
			switch (uEnc) {
			case TT_MS_ID_SYMBOL_CS:
				// Symbol Fonts, using UTF-16 BE
			case TT_MS_ID_UCS_4:
				// though it's name is UCS-4, it actually uses utf16be in names
			case TT_MS_ID_UNICODE_CS:
				// UTF-16 BE
				return 1201;
			case TT_MS_ID_PRC:
				// GBK
				return 936;
			case TT_MS_ID_SJIS:
				// Shift-JIS
				return 932;
			case TT_MS_ID_BIG_5:
				// BIG-5
				return 950;
			case TT_MS_ID_WANSUNG:
				// Wansung
				return 949;
			case TT_MS_ID_JOHAB:
				// Johab
				return 1361;
			default:
				return -1;
			}
		}

		/// <summary>
		/// Decode bytes to wchar
		/// </summary>
		/// <param name="codepage">codepage used</param>
		/// <param name="str">bytes to be decoded</param>
		/// <param name="length">bytes length</param>
		/// <returns>decoded string, empty if fail</returns>
		std::wstring DecodeBytes(UINT codepage, const char* str, size_t length)
		{
			if (codepage == 1201) {
				return DecodeBytes(str, length);
			}

			int result = MultiByteToWideChar(codepage, MB_ERR_INVALID_CHARS, str,
				length, _pbConv->data(), _pbConv->size());
			if (result == 0) {
				DWORD err = GetLastError();
				if (err == ERROR_INSUFFICIENT_BUFFER) {
					_pbConv->resize(_pbConv->size() + _pbConv->size() / 2);
					return DecodeBytes(codepage, str, length);
				}
				else {
					return std::wstring();
				}
			}
			return std::wstring(_pbConv->data(), result);
		}

		/// <summary>
		/// Decode bytes as UTF16BE
		/// </summary>
		/// <param name="str">bytes to be decoded</param>
		/// <param name="length">bytes length</param>
		/// <returns>decoded string, empty if fail</returns>
		static std::wstring DecodeBytes(const char* str, size_t length)
		{
			std::wstring_convert<std::codecvt_utf16<wchar_t>> utf16be_conv;
			try {
				return utf16be_conv.from_bytes(str, str + length);
			}
			catch (std::range_error) {
				return std::wstring();
			}
		}

		std::wstring ConvertName(const FT_SfntName& name, bool& warn)
		{
			std::wstring wsName;
			UINT uCodepage = GetCodepage(name.encoding_id);
			if (uCodepage == -1)return std::wstring();

			bool bUnicodeName = true;
			if (name.encoding_id == TT_MS_ID_BIG_5 ||
				name.encoding_id == TT_MS_ID_PRC ||
				name.encoding_id == TT_MS_ID_WANSUNG)
			{
				const size_t nCvtSize = 128;
				char tmp[nCvtSize];
				const char* sSFNTName = reinterpret_cast<const char*>(name.string);
				size_t nAnsiLength = 0;
				size_t nSFNTLength = name.string_len - (name.string_len % 2);

				for (; nSFNTLength; nSFNTLength -= 2)
				{

					if (sSFNTName[0])
					{
						if (nCvtSize >= (nAnsiLength + 2)) {
							tmp[nAnsiLength++] = sSFNTName[0];
							tmp[nAnsiLength++] = sSFNTName[1];
						}
						else
						{
							break;
						}
					}
					else
					{
						if (nCvtSize >= (nAnsiLength + 1)) {
							tmp[nAnsiLength++] = sSFNTName[1];
						}
						else
						{
							break;
						}
					}
					sSFNTName += 2;
				}
				wsName = DecodeBytes(uCodepage, tmp, nAnsiLength);
				if (!wsName.empty())
					bUnicodeName = false;
			}
			if (bUnicodeName)
				wsName = DecodeBytes(reinterpret_cast<const char*>(name.string), name.string_len & (~1u));
			return wsName;
		}

		static void DistinctList(std::vector<std::wstring>& list)
		{
			std::sort(list.begin(), list.end());
			auto itEnd = std::unique(list.begin(), list.end());
			list.erase(itEnd, list.end());
		}

		static List<String^>^ ConvertVectorToCliList(const std::vector<std::wstring>& vec)
		{
			List<String^>^ ret = gcnew List<String^>(vec.size());
			for (auto& str : vec) {
				ret->Add(gcnew String(str.c_str()));
			}
			return ret;
		}
	private:
		FTContext^ _ctx;

		FTFontFileMap^ _map;

		std::vector<wchar_t>* _pbConv;
	};
}