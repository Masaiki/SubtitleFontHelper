#pragma once

#include "common.h"
#include <string>

bool AttachToDaemon(RPCInfo* pInfo);
void DetachFromDaemon(RPCInfo* pInfo);

std::vector<std::wstring> GetFontFilePath(RPCInfo* pInfo, const std::wstring& strFaceName);
std::vector<std::wstring> GetFontFilePath(RPCInfo* pInfo, const std::wstring& strFaceName, int weight, int italic);