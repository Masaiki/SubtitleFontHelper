
#pragma once

#define generic __identifier(generic)

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_ERRORS_H

#undef generic

using namespace System;
using namespace System::Collections::Generic;

namespace libSubtitleFontHelper {
	private ref class FreeTypeLibrary {
	private:
		static FreeTypeLibrary^ _instance;

		int _ref_count = 1;

		FT_Library _ft_lib = nullptr;

		FreeTypeLibrary();

		void AddRef();
		void Release();

		~FreeTypeLibrary();
		!FreeTypeLibrary();
	public:
		/// <summary>
		/// Get a reference to freetype library
		/// Create a new instance or increase its refcount
		/// </summary>
		/// <returns>The library</returns>
		static FreeTypeLibrary^ GetInstance();

		/// <summary>
		/// Destroy the reference to library. 
		/// Decrease its refcount or Dispose it
		/// </summary>
		void DestroyInstance();

		/// <summary>
		/// Get the FT_Library
		/// </summary>
		/// <returns>FreeType library</returns>
		FT_Library GetLibrary();
	};

	public interface class IFreeTypeFontFaceMetadata{
	public:
		virtual IReadOnlyList<String^>^ GetPostScriptNames()abstract;
		virtual IReadOnlyList<String^>^ GetFullNames()abstract;
		virtual IReadOnlyList<String^>^ GetFamilyNames()abstract;
	};

	private ref class FreeTypeFontFaceMetaData :public IFreeTypeFontFaceMetadata {
	public:
		virtual IReadOnlyList<String^>^ GetPostScriptNames()override;
		virtual IReadOnlyList<String^>^ GetFullNames()override;
		virtual IReadOnlyList<String^>^ GetFamilyNames()override;
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

		/// <summary>
		/// Create a object to get font metadata using FreeType with given file path
		/// </summary>
		/// <param name="file_path">path to file</param>
		FreeTypeFontMetadata(const String^ file_path);

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
		void InitializeFreeTypeLib();
		FileMapInfo CreateFileMap(const String^ file_path);
		void ReleaseFileMap(FileMapInfo% file_map);
	private:
		FreeTypeLibrary^ _ft_ref_lib;
		
		long _fontface_count = 0;

		FileMapInfo _file_map;
	};



}
