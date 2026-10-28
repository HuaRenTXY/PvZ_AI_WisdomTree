// SPDX-License-Identifier: LGPL-3.0-or-later
// PvZ AI Wisdom Tree fork of PvZ-Portable

#ifndef __PVZ_AI_UTIL_H__
#define __PVZ_AI_UTIL_H__

#include <string>

namespace PvZAI
{
	std::wstring Utf8ToWide(const std::string& theString);
	std::string WideToUtf8(const std::wstring& theString);
	std::string TrimAscii(const std::string& theString);
	std::string JsonEscape(const std::string& theString);
	void AppendUtf8(std::string& theOut, unsigned int theCodePoint);
	void Log(const std::string& theMessage);
}

#endif

