#pragma once

#include "common.h"
#include <string>

bool AttachToDaemon(RPCInfo* pInfo);
void DetachFromDaemon(RPCInfo* pInfo);

std::wstring GetFontFilePath(RPCInfo* pInfo, const std::wstring& strFullName);