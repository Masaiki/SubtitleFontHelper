#include "pch.h"

#include "FreeTypeFontMetadata.h"

#include <vcclr.h>

using namespace System::ComponentModel;

namespace libSubtitleFontHelper {

    FreeTypeFontMetadata::FreeTypeFontMetadata()
    {
        InitializeFreeTypeLib();
    }

    FreeTypeFontMetadata::FreeTypeFontMetadata(const String^ file_path)
    {
        InitializeFreeTypeLib();
        OpenFontFile(file_path);
    }

    FreeTypeFontMetadata::~FreeTypeFontMetadata()
    {
        _ft_ref_lib->DestroyInstance();
        // release unmanaged resources
        this->!FreeTypeFontMetadata();
    }

    FreeTypeFontMetadata::!FreeTypeFontMetadata()
    {
        ReleaseFileMap(_file_map);
    }

    void FreeTypeFontMetadata::OpenFontFile(const String^ file_path)
    {
        FT_Face font_face = nullptr;
        FT_Error err = FT_Err_Ok;
        FileMapInfo file_map;
        FT_Long font_face_count = 0;
        
        file_map = CreateFileMap(file_path);
        if ((err = FT_New_Memory_Face(_ft_ref_lib->GetLibrary(), (const FT_Byte*)file_map.map_mem,
            file_map.map_size, -1, &font_face)) != FT_Err_Ok) {
            
            throw gcnew System::Exception(gcnew String("FT_New_Memory_Face failed. ") + gcnew String(FT_Error_String(err)));
        }

        _fontface_count = font_face->num_faces;
        FT_Done_Face(font_face);

        _file_map.Release();
    }

    long FreeTypeFontMetadata::GetFontFaceCount()
    {
        return _fontface_count;
    }

    IFreeTypeFontFaceMetadata^ FreeTypeFontMetadata::GetFontFace(long index)
    {
        throw gcnew System::NotImplementedException();
    }

    void FreeTypeFontMetadata::InitializeFreeTypeLib()
    {
        _ft_ref_lib = FreeTypeLibrary::GetInstance();
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

    FreeTypeLibrary::FreeTypeLibrary()
    {
        pin_ptr<FT_Library> ft_lib = &_ft_lib;
        FT_Error err = FT_Init_FreeType(ft_lib);
        if (err != FT_Err_Ok) {
            throw gcnew System::Exception(gcnew String("Unable to initialize FreeType. ") + gcnew String(FT_Error_String(err)));
        }
    }

    void FreeTypeLibrary::AddRef()
    {
        ++_ref_count;
    }

    void FreeTypeLibrary::Release()
    {
        --_ref_count;
        if (_ref_count == 0) {
            _instance = nullptr;
            delete this;
        }
    }

    FreeTypeLibrary::~FreeTypeLibrary()
    {
        this->!FreeTypeLibrary();
    }

    FreeTypeLibrary::!FreeTypeLibrary()
    {
        FT_Done_FreeType(_ft_lib);
    }

    FreeTypeLibrary^ FreeTypeLibrary::GetInstance()
    {
        if (_instance == nullptr) {
            _instance = gcnew FreeTypeLibrary();
        }
        else {
            _instance->AddRef();
        }
        return _instance;
    }

    void FreeTypeLibrary::DestroyInstance()
    {
        _instance->Release();
    }

    FT_Library FreeTypeLibrary::GetLibrary()
    {
        return _ft_lib;
    }


    IReadOnlyList<String^>^ FreeTypeFontFaceMetaData::GetPostScriptNames()
    {
        throw gcnew System::NotImplementedException();
        // TODO: 在此处插入 return 语句
    }

    IReadOnlyList<String^>^ FreeTypeFontFaceMetaData::GetFullNames()
    {
        throw gcnew System::NotImplementedException();
        // TODO: 在此处插入 return 语句
    }

    IReadOnlyList<String^>^ FreeTypeFontFaceMetaData::GetFamilyNames()
    {
        throw gcnew System::NotImplementedException();
        // TODO: 在此处插入 return 语句
    }

};