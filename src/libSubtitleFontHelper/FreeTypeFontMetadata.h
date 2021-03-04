
#pragma once

#define generic __identifier(generic)

#include <ft2build.h>

#include FT_FREETYPE_H
#include FT_SFNT_NAMES_H

#undef generic

#include <vector>
#include <string>
#include <codecvt>

using namespace System;
using namespace System::Text;
using namespace System::Collections::Generic;

namespace libSubtitleFontHelper {

	class FreeTypeFontFaceMetadataNative;

	public interface class IFreeTypeFontFaceMetadata{
	public:
		virtual IReadOnlyList<String^>^ GetPostScriptNames()abstract;
		virtual IReadOnlyList<String^>^ GetFullNames()abstract;
		virtual IReadOnlyList<String^>^ GetFamilyNames()abstract;
	};

	private ref class FreeTypeFontFaceMetaData :public IFreeTypeFontFaceMetadata {
	public:
		/// <summary>
		/// implement GetPostScriptNames
		/// </summary>
		/// <returns>the list of names</returns>
		virtual IReadOnlyList<String^>^ GetPostScriptNames();

		/// <summary>
		/// implement GetFullNames
		/// </summary>
		/// <returns>the list of names</returns>
		virtual IReadOnlyList<String^>^ GetFullNames();

		/// <summary>
		/// implement GetFamilyNames
		/// </summary>
		/// <returns>the list of names</returns>
		virtual IReadOnlyList<String^>^ GetFamilyNames();

		/// <summary>
		/// Initialize metadata from FT_Face
		/// </summary>
		/// <param name="ft_face">the FT_Face to the font face</param>
		FreeTypeFontFaceMetaData(FT_Face ft_face, FreeTypeFontFaceMetadataNative* face_parser);

	private:

		void CheckFontFaceFormat(FT_Face ft_face);
		void ReadFontFaceMetadata(FT_Face ft_face);
		static List<String^>^ STLVectorToCLIList(const std::vector<std::wstring>& vec);
	private:
		FreeTypeFontFaceMetadataNative* _face_parser;

		List<String^>^ _ps_names;
		List<String^>^ _full_names;
		List<String^>^ _family_names;
	};

	public ref class FreeTypeFontMetadata
	{
	private:
		value struct FileMapInfo {
			HANDLE filemap_handle;
			HANDLE file_handle;

			void* map_mem;
			size_t map_size;

			/// <summary>
			/// Release all resources
			/// </summary>
			void Release();
		};

	public:
		/// <summary>
		/// Create a object to get font metadata using FreeType
		/// </summary>
		FreeTypeFontMetadata();

		// Release all resources
		~FreeTypeFontMetadata();
		// Release all unmanaged resources
		!FreeTypeFontMetadata();

		/// <summary>
		/// Open a new font to read
		/// </summary>
		/// <param name="file_path">path to file</param>
		void OpenFontFile(const String^ file_path);

		/// <summary>
		/// Get font face count in opened font file
		/// </summary>
		/// <returns>font face count</returns>
		long GetFontFaceCount();

		/// <summary>
		/// Get font face object by index
		/// </summary>
		/// <param name="index">font face index(starts from 0)</param>
		/// <returns>font face object</returns>
		IFreeTypeFontFaceMetadata^ GetFontFace(long index);

	private:
		void InitializeNative();
		FileMapInfo CreateFileMap(const String^ file_path);
		void ReleaseFileMap(FileMapInfo% file_map);
	private:
		FT_Library _ft_lib;
		
		long _fontface_count = 0;

		FileMapInfo _file_map;

		FreeTypeFontFaceMetadataNative* _face_parser;
	};

	class FreeTypeFontFaceMetadataNative {
	public:
		FreeTypeFontFaceMetadataNative();

		const std::vector<std::wstring>& GetPSNames()const { return _ps_names; }
		const std::vector<std::wstring>& GetFullnames()const { return _full_names; }
		const std::vector<std::wstring>& GetFamilyNames()const { return _family_names; }

		bool ReadFontFace(FT_Face face);

	private:

		std::wstring GetString(FT_SfntName& raw_name);
		std::wstring GetStringFallback(FT_SfntName& raw_name);

		std::wstring DecodeBytes(UINT codepage, const char* bytes, size_t length);
		std::wstring DecodeUTF16BE(const char* bytes, size_t length);

		inline static UINT GetCodePageByEncodingId(FT_UInt encoding_id);

		inline static void DeduplicateVector(std::vector<std::wstring>& vec);
	private:

		std::vector<std::wstring> _ps_names;
		std::vector<std::wstring> _full_names;
		std::vector<std::wstring> _family_names;

		std::vector<wchar_t> _conv_buf;
	};

}
