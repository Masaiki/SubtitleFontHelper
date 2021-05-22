#include "pch.h"

#include "FreeTypeFontMetadata.h"

#include <vcclr.h>

#include <cstring>
#include <memory>
#include <algorithm>

#define generic __identifier(generic)

#include <ft2build.h>

#include FT_TRUETYPE_IDS_H
#include FT_SFNT_NAMES_H
#include FT_ERRORS_H
#include FT_FONT_FORMATS_H

#undef generic

using namespace System;
using namespace System::ComponentModel;
using namespace System::Text;
using namespace System::Diagnostics;

namespace libSubtitleFontHelper {

	FreeTypeFontMetadata::FreeTypeFontMetadata()
	{
		InitializeNative();
	}

	FreeTypeFontMetadata::~FreeTypeFontMetadata()
	{
		// release unmanaged resources
		this->!FreeTypeFontMetadata();
	}

	FreeTypeFontMetadata::!FreeTypeFontMetadata()
	{
		if (_face_parser)delete _face_parser;
		if (_ft_lib)FT_Done_FreeType(_ft_lib);
		ReleaseFileMap(_file_map);
	}

	void FreeTypeFontMetadata::OpenFontFile(const String^ file_path)
	{
		FT_Face font_face = nullptr;
		FT_Error err = FT_Err_Ok;
		FileMapInfo file_map;
		FT_Long font_face_count = 0;

		file_map = CreateFileMap(file_path);
		if ((err = FT_New_Memory_Face(_ft_lib, (const FT_Byte*)file_map.map_mem,
			file_map.map_size, -1, &font_face)) != FT_Err_Ok) {

			throw gcnew System::Exception(String::Format(gcnew String("FT_New_Memory_Face failed. {0}"), gcnew String(FT_Error_String(err))));
		}

		_fontface_count = font_face->num_faces;
		FT_Done_Face(font_face);

		_file_map.Release();
		_file_map = file_map;
	}

	long FreeTypeFontMetadata::GetFontFaceCount()
	{
		return _fontface_count;
	}

	IFreeTypeFontFaceMetadata^ FreeTypeFontMetadata::GetFontFace(long index)
	{
		FT_Error err = FT_Err_Ok;
		FT_Face face = nullptr;
		FreeTypeFontFaceMetaData^ meta;

		if ((err = FT_New_Memory_Face(_ft_lib,
			(const FT_Byte*)_file_map.map_mem, _file_map.map_size, index, &face)) != FT_Err_Ok) {
			throw gcnew System::Exception(String::Format(gcnew String("FT_New_Memory_Face failed. {0}"), gcnew String(FT_Error_String(err))));
		}

		try {
			meta = gcnew FreeTypeFontFaceMetaData(face, _face_parser);
		}
		finally {
			FT_Done_Face(face);
		}

		return meta;
	}

	void FreeTypeFontMetadata::InitializeNative()
	{
		pin_ptr<FT_Library> ft_pinned_lib = &_ft_lib;
		FT_Error err = FT_Init_FreeType(ft_pinned_lib);
		if (err != FT_Err_Ok) {
			throw gcnew System::Exception(String::Format(gcnew String("FT_Init_FreeType failed. {0}"), gcnew String(FT_Error_String(err))));
		}

		_face_parser = new FreeTypeFontFaceMetadataNative();
	}

	FreeTypeFontMetadata::FileMapInfo FreeTypeFontMetadata::CreateFileMap(const String^ file_path)
	{
		pin_ptr<const wchar_t> file_path_str = PtrToStringChars(file_path);
		HANDLE file_handle = CreateFileW((LPCWSTR)file_path_str, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, NULL, NULL);
		if (file_handle == INVALID_HANDLE_VALUE) {
			DWORD err = GetLastError();
			throw gcnew Win32Exception(err);
		}

		HANDLE filemap_handle = CreateFileMappingW(file_handle, NULL, PAGE_READONLY, 0, 0, nullptr);
		if (filemap_handle == NULL) {
			DWORD err = GetLastError();
			CloseHandle(file_handle);
			throw gcnew Win32Exception(err);
		}

		void* file_mappedmem = MapViewOfFile(filemap_handle, FILE_MAP_READ, 0, 0, 0);
		if (file_mappedmem == NULL) {
			DWORD err = GetLastError();
			CloseHandle(filemap_handle);
			CloseHandle(file_handle);
			throw gcnew Win32Exception(err);
		}

		size_t file_size = 0;
		LARGE_INTEGER large_int;
		if (GetFileSizeEx(file_handle, &large_int) != 0) {
			file_size = (size_t)large_int.QuadPart;
		}
		else {
			MEMORY_BASIC_INFORMATION mbi;
			if (VirtualQuery(file_mappedmem, &mbi, sizeof(mbi)) == 0) {
				UnmapViewOfFile(file_mappedmem);
				CloseHandle(filemap_handle);
				CloseHandle(file_handle);
				throw gcnew System::Exception("Unable to get file size");
			}
			file_size = mbi.RegionSize;
		}

		FileMapInfo info;

		info.map_mem = file_mappedmem;
		info.map_size = file_size;
		info.filemap_handle = filemap_handle;
		info.file_handle = file_handle;

		return info;
	}

	void FreeTypeFontMetadata::ReleaseFileMap(FileMapInfo% file_map)
	{
		file_map.Release();
	}

	void FreeTypeFontMetadata::FileMapInfo::Release()
	{
		if (map_mem)UnmapViewOfFile(map_mem);
		if (filemap_handle)CloseHandle(filemap_handle);
		if (file_handle)CloseHandle(file_handle);
	}


	IReadOnlyList<String^>^ FreeTypeFontFaceMetaData::GetPostScriptNames()
	{
		return _ps_names->AsReadOnly();
	}

	IReadOnlyList<String^>^ FreeTypeFontFaceMetaData::GetFullNames()
	{
		return _full_names->AsReadOnly();
	}

	IReadOnlyList<String^>^ FreeTypeFontFaceMetaData::GetFamilyNames()
	{
		return _family_names->AsReadOnly();
	}

	FreeTypeFontFaceMetaData::FreeTypeFontFaceMetaData(FT_Face ft_face, FreeTypeFontFaceMetadataNative* face_parser)
	{
		_face_parser = face_parser;
		CheckFontFaceFormat(ft_face);
		ReadFontFaceMetadata(ft_face);
	}

	void FreeTypeFontFaceMetaData::CheckFontFaceFormat(FT_Face ft_face)
	{
		const char* fmt_str = FT_Get_Font_Format(ft_face);
		if (!fmt_str)throw gcnew System::Exception("unknown font format");
		if (std::strcmp(fmt_str, "TrueType") == 0) {
			// OK
		}
		else if (std::strcmp(fmt_str, "CFF") == 0) {
			// OK
		}
		else {
			throw gcnew System::Exception("unsupported font format");
		}

		if (!(ft_face->face_flags & FT_FACE_FLAG_SFNT)) {
			throw gcnew System::Exception("unsupported font format: no SFNT info");
		}
	}

	void FreeTypeFontFaceMetaData::ReadFontFaceMetadata(FT_Face ft_face)
	{
		_face_parser->ReadFontFace(ft_face);
		_ps_names = STLVectorToCLIList(_face_parser->GetPSNames());
		_full_names = STLVectorToCLIList(_face_parser->GetFullnames());
		_family_names = STLVectorToCLIList(_face_parser->GetFamilyNames());
	}

	List<String^>^ FreeTypeFontFaceMetaData::STLVectorToCLIList(const std::vector<std::wstring>& vec)
	{
		List<String^>^ ret = gcnew List<String^>(vec.size());
		for (auto& str : vec) {
			ret->Add(gcnew String(str.c_str()));
		}
		return ret;
	}

#pragma managed(push,off)

	FreeTypeFontFaceMetadataNative::FreeTypeFontFaceMetadataNative()
	{
		_conv_buf.resize(128);
	}
	bool FreeTypeFontFaceMetadataNative::ReadFontFace(FT_Face face)
	{
		// clear vectors first
		_ps_names.clear();
		_full_names.clear();
		_family_names.clear();

		FT_SfntName sfnt_name;
		FT_UInt sfnt_name_count = FT_Get_Sfnt_Name_Count(face);

		if (sfnt_name_count == 0)
			return false;

		for (size_t i = 0; i < sfnt_name_count; ++i) {
			std::vector<std::wstring>* store_target;
			FT_Error err = FT_Get_Sfnt_Name(face, i, &sfnt_name);
			if (err != FT_Err_Ok)
				continue;
			
			if (sfnt_name.platform_id != TT_PLATFORM_MICROSOFT)
				continue;

			switch (sfnt_name.name_id) {
			case TT_NAME_ID_PS_NAME:
				store_target = &_ps_names; break;
			case TT_NAME_ID_FULL_NAME:
				store_target = &_full_names; break;
			case TT_NAME_ID_FONT_FAMILY:
				store_target = &_family_names; break;
			default:
				continue;
			}

			std::wstring name = GetString(sfnt_name);
			if (name.empty())
				continue;

			store_target->push_back(name);
		}

		DeduplicateVector(_ps_names);
		DeduplicateVector(_full_names);
		DeduplicateVector(_family_names);

		return true;
	}
	std::wstring FreeTypeFontFaceMetadataNative::GetString(FT_SfntName& raw_name)
	{
		UINT codepage = GetCodePageByEncodingId(raw_name.encoding_id);
		if (codepage == -1)return std::wstring();  // bad encoding

		std::wstring primary_str = DecodeBytes(codepage, (const char*)raw_name.string, raw_name.string_len);
		if (primary_str.empty())return GetStringFallback(raw_name); // we need some workarounds

		size_t p_null_pos = 0;
		size_t f_null_pos = 0;
		if ((p_null_pos = primary_str.find(L'\0')) != std::wstring::npos) {
			// string contains NULL
			std::wstring fallback_str = GetStringFallback(raw_name);
			// select string which is longer in range[0...NULL]
			f_null_pos = fallback_str.find(L'\0');
			if (p_null_pos > f_null_pos) {
				primary_str.resize(p_null_pos);
				return primary_str;
			}
			else {
				if (f_null_pos != std::wstring::npos) {
					fallback_str.resize(f_null_pos);
				}
				return fallback_str;
			}
		}
		return primary_str;
	}
	std::wstring FreeTypeFontFaceMetadataNative::GetStringFallback(FT_SfntName& raw_name)
	{
		if (raw_name.encoding_id == TT_MS_ID_BIG_5 || raw_name.encoding_id == TT_MS_ID_PRC) {
			// some fonts may store GBK/BIG5 bytes in big-endian uint16_t 
			if (raw_name.string_len != 0 && raw_name.string_len % 2 == 0) {
				size_t new_len = raw_name.string_len / 2;
				std::unique_ptr<char[]> bytes(new char[new_len]);
				for (size_t i = 0; i < new_len; ++i) {
					if (raw_name.string[i * 2] != 0) {
						// not uint16 BE
						return std::wstring();
					}
					bytes[i] = raw_name.string[i * 2 + 1];
				}
				UINT codepage = GetCodePageByEncodingId(raw_name.encoding_id);
				return DecodeBytes(codepage, bytes.get(), new_len);
			}
		}
		else if (raw_name.encoding_id == TT_MS_ID_SJIS) {
			// some fonts may store utf16be bytes but telling SJIS encoding
			return DecodeUTF16BE((const char*)raw_name.string, raw_name.string_len);
		}
		return std::wstring();
	}
	std::wstring FreeTypeFontFaceMetadataNative::DecodeBytes(UINT codepage, const char* bytes, size_t length)
	{
		if (codepage == 1201) {
			return DecodeUTF16BE(bytes, length);
		}

		int hr = MultiByteToWideChar(codepage, MB_ERR_INVALID_CHARS, bytes, length, _conv_buf.data(), _conv_buf.size());
		if (hr == 0) {
			DWORD err = GetLastError();
			if (err == ERROR_INSUFFICIENT_BUFFER) {
				_conv_buf.resize(static_cast<size_t>(_conv_buf.size() * 1.5));
				return DecodeBytes(codepage, bytes, length);
			}
			else {
				return std::wstring();
			}
		}
		return std::wstring(_conv_buf.data(), hr);
	}
	std::wstring FreeTypeFontFaceMetadataNative::DecodeUTF16BE(const char* bytes, size_t length)
	{
		std::wstring_convert<std::codecvt_utf16<wchar_t>> utf16be_conv;
		try {
			return utf16be_conv.from_bytes(bytes, bytes + length);
		}
		catch (std::range_error) {
			return std::wstring();
		}
	}
	inline UINT FreeTypeFontFaceMetadataNative::GetCodePageByEncodingId(FT_UInt encoding_id)
	{
		switch (encoding_id) {
		case TT_MS_ID_SYMBOL_CS:
			// Symbol Fonts, using UTF-16 BE
		case TT_MS_ID_UCS_4:
			// though it's name is UCS-4, it actually uses utf16be in names
		case TT_MS_ID_UNICODE_CS:
			// UTF-16 BE
			return 1201;
		case TT_MS_ID_PRC:
			// GB18030
			return 54936;
		case TT_MS_ID_SJIS:
			// Shift-JIS
			return 932;
		case TT_MS_ID_BIG_5:
			// BIG-5
			return 950;
		case TT_MS_ID_WANSUNG:
			// Wansung
			return 20949;
		case TT_MS_ID_JOHAB:
			// Johab
			return 1361;
		default:
			return -1;
		}
	}
	inline void FreeTypeFontFaceMetadataNative::DeduplicateVector(std::vector<std::wstring>& vec)
	{
		std::sort(vec.begin(), vec.end());
		auto new_end = std::unique(vec.begin(), vec.end());
		vec.erase(new_end, vec.end());
	}

#pragma managed(pop)
}
