

#include "common.h"
#include "message.h"

#include <unordered_set>

DllInfo g_stInfo;

void LoadFontFile(const std::wstring& strPath)
{
	static std::unordered_set<std::wstring> cache;
	if (cache.find(strPath) == cache.end()) {
		cache.insert(strPath);
		AddFontResourceExW(strPath.c_str(), FR_PRIVATE, NULL);
	}
}

void TryLoadFont(const std::wstring& strName)
{
	auto strPath = GetFontFilePath(g_stInfo.pRpcInfo.get(), strName);
	if (!strPath.empty()) {
		LoadFontFile(strPath);
	}
}

void TryLoadFont(const std::string& strName) {
	auto wstrName = MultiByteToWideACP(strName.data(), strName.size());
	if (wstrName.empty())return;
	TryLoadFont(wstrName);
}

std::wstring MultiByteToWideACP(const char* pszStr, size_t nSize)
{
	DWORD dwLen = MultiByteToWideChar(CP_ACP, 0, pszStr, nSize, nullptr, 0);
	if (dwLen == 0)return std::wstring();
	std::unique_ptr<wchar_t[]> pwcBuf(new wchar_t[dwLen]);
	dwLen = MultiByteToWideChar(CP_ACP, 0, pszStr, nSize, pwcBuf.get(), dwLen);
	if (dwLen == 0)return std::wstring();
	return std::wstring(pwcBuf.get(), dwLen);
}

DllInfo::DllInfo():pRpcInfo(new RPCInfo)
{

}
