
#pragma once

#include "framework.h"

#include <vector>
#include <utility>
#include <unordered_map>
#include <mutex>
#include <memory>

struct RPCInfo {
	std::mutex mutMutex;
	std::wstring strPipeName = LR"_(\\.\pipe\SubtitleFontHelperPipe)_";
	HANDLE hPipe = INVALID_HANDLE_VALUE;
	size_t nMaxRetry = 3;
};

struct DllInfo {
	// first LPVOID points to function being hooked
	// second LPVOID points to function used to replace hooked function
	std::vector<std::pair<LPVOID*, LPVOID>> vHookedFunc;

	std::unique_ptr<RPCInfo> pRpcInfo;

	std::mutex mutCache;
	std::unordered_map<std::wstring, std::wstring> mapCache;

	DllInfo();
};

extern DllInfo g_stInfo;


void LoadFontFile(const std::wstring& strPath);
void TryLoadFont(const std::wstring& strName);
void TryLoadFont(const std::string& strName);

std::wstring MultiByteToWideACP(const char* pszStr, size_t nSize);