#include "message.h"

#include <cstdint>
#include <mutex>
#include <functional>
#include <cassert>
#include <memory>
#include <codecvt>

#include "proto/message.pb.h"

static bool WriteBytesToFile(HANDLE hFile, const char* pBuffer, size_t nBytes);
static bool WriteBytesToFilePrefixed(HANDLE hFile, const char* pBuffer, size_t nBytes);
static bool ReadBytesFromFile(HANDLE hFile, char* pBuffer, size_t nBytes);
static bool SendAttachMessage(HANDLE hPipe);
static std::wstring GetFontFilePathRpc(RPCInfo* pInfo, const std::wstring& strFullName);
static void CleanUpRpcInfo(RPCInfo* pInfo);

bool AttachToDaemon(RPCInfo* pInfo)
{
	std::lock_guard<std::mutex> lg(pInfo->mutMutex);
	size_t nRetry = 0;
	while (true) {
		pInfo->hPipe = CreateFileW(
			pInfo->strPipeName.c_str(),
			GENERIC_READ | GENERIC_WRITE,
			NULL,
			NULL,
			OPEN_EXISTING,
			NULL,
			NULL);
		DWORD hr = GetLastError();
		if (pInfo->hPipe != INVALID_HANDLE_VALUE) {
			break;
		}

		DWORD dwError = GetLastError();

		if (dwError == ERROR_PIPE_BUSY) {
			if (WaitNamedPipeW(pInfo->strPipeName.c_str(), NMPWAIT_USE_DEFAULT_WAIT) == 0) {
				++nRetry;
				if (nRetry > pInfo->nMaxRetry) {
					break;
				}
				else {
					continue;
				}
			}
		}
		break;
	}

	if (pInfo->hPipe == INVALID_HANDLE_VALUE) {
		return false;
	}

	if (!SendAttachMessage(pInfo->hPipe)) {
		CleanUpRpcInfo(pInfo);
		return false;
	}

	return true;
}

void DetachFromDaemon(RPCInfo* pInfo)
{
	std::lock_guard<std::mutex> lg(pInfo->mutMutex);
	// just closes handle
	CleanUpRpcInfo(pInfo);
}

std::wstring GetFontFilePath(RPCInfo* pInfo, const std::wstring& strFullName)
{	
	std::lock_guard<std::mutex> lg(g_stInfo.mutCache);
	auto iterCache = g_stInfo.mapCache.find(strFullName);
	if (iterCache == g_stInfo.mapCache.end()) {
		// not exist
		auto strPath = GetFontFilePathRpc(pInfo, strFullName);
		if (!strPath.empty()){
			g_stInfo.mapCache.insert(std::make_pair(strFullName, strPath));
		}
		return strPath;
	}
	else {
		return iterCache->second;
	}
}

static bool WriteBytesToFile(HANDLE hFile, const char* pBuffer, size_t nBytes) {
	size_t nPos = 0;
	DWORD dwWritten;
	while (nPos < nBytes) {
		if (WriteFile(hFile, pBuffer + nPos, nBytes - nPos, &dwWritten, NULL) == FALSE) {
			// FAILURE, BROKEN PIPE/DATA
			return false;
		}
		nPos += dwWritten;
	}
	return true;
}

bool WriteBytesToFilePrefixed(HANDLE hFile, const char* pBuffer, size_t nBytes)
{
	uint32_t nLength = nBytes;
	if (!WriteBytesToFile(hFile, reinterpret_cast<const char*>(&nBytes), sizeof(uint32_t))) {
		return false;
	}
	return WriteBytesToFile(hFile, pBuffer, nBytes);
}

static bool ReadBytesFromFile(HANDLE hFile, char* pBuffer, size_t nBytes) {
	size_t nPos = 0;
	DWORD dwRead = 0;
	while (nPos < nBytes) {
		if (ReadFile(hFile, pBuffer + nPos, nBytes - nPos, &dwRead, NULL) == FALSE) {
			return false;
		}
		nPos += dwRead;
	}
	return true;
}

static bool SendAttachMessage(HANDLE hPipe) {
	using namespace SubtitleFontHelper;
	Message msgHeader;
	AttachIndication msgAI;
	bool ret = false;

	msgAI.set_processid(GetProcessId(GetCurrentProcess()));
	msgHeader.set_type(MessageType::Indication_Attach);
	msgHeader.set_allocated_attachindication(&msgAI);
	
	std::string bufSerialized = msgHeader.SerializeAsString();

	if (!bufSerialized.empty()) {
		ret = WriteBytesToFilePrefixed(hPipe, bufSerialized.data(), bufSerialized.size());
	}

	msgHeader.release_attachindication();

	return ret;
}



static bool SendQueryMessage(HANDLE hPipe,const std::wstring& strQuery) {
	using namespace SubtitleFontHelper;
	Message msgHeader;
	FontQueryRequest msgQuery;
	std::wstring_convert<std::codecvt_utf8<wchar_t>> utf8_conv;
	bool ret = false;

	msgQuery.set_fullname(utf8_conv.to_bytes(strQuery));
	msgHeader.set_type(MessageType::Request_FontQuery);
	msgHeader.set_allocated_fontqueryrequest(&msgQuery);

	std::string bufSerialized = msgHeader.SerializeAsString();

	if (!bufSerialized.empty()) {
		ret = WriteBytesToFilePrefixed(hPipe, bufSerialized.data(), bufSerialized.size());
	}

	msgHeader.release_fontqueryrequest();

	return ret;
}

static bool ReceiveQueryResponse(HANDLE hPipe, std::wstring& strResult) {
	using namespace SubtitleFontHelper;
	Message msgHeader;
	uint32_t nLength;
	std::wstring_convert<std::codecvt_utf8<wchar_t>> utf8_conv;
	bool ret = false;

	if (!ReadBytesFromFile(hPipe, reinterpret_cast<char*>(&nLength), sizeof(uint32_t))) {
		return false;
	}

	std::vector<char> bufSerialized(nLength);
	if (!ReadBytesFromFile(hPipe, bufSerialized.data(), nLength)) {
		return false;
	}

	msgHeader.ParseFromArray(bufSerialized.data(), bufSerialized.size());

	if (msgHeader.type() != MessageType::Response_FontQuery) {
		return false;
	}

	if (msgHeader.has_fontqueryresponse() == false) {
		return false;
	}

	try {
		strResult = utf8_conv.from_bytes(msgHeader.fontqueryresponse().fullpath());
	}
	catch (...) {
		return false;
	}

	return true;
}

static std::wstring GetFontFilePathRpc(RPCInfo* pInfo, const std::wstring& strFullName) {
	std::lock_guard<std::mutex> lg(pInfo->mutMutex);
	if (pInfo->hPipe == INVALID_HANDLE_VALUE)return std::wstring();
	std::wstring strResult;
	bool bStat = false;
	bStat = SendQueryMessage(pInfo->hPipe, strFullName);
	if (bStat == false)goto error;
	bStat = ReceiveQueryResponse(pInfo->hPipe, strResult);
	if (bStat == false)goto error;
	goto success;
error:
	CleanUpRpcInfo(pInfo);
	strResult.clear();
success:
	return strResult;
}

void CleanUpRpcInfo(RPCInfo* pInfo)
{
	CloseHandle(pInfo->hPipe);
	pInfo->hPipe = INVALID_HANDLE_VALUE;
}
