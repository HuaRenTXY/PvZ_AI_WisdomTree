// SPDX-License-Identifier: LGPL-3.0-or-later
// PvZ AI Wisdom Tree fork of PvZ-Portable

#include "AIUtil.h"

#include <windows.h>
#include <fstream>

namespace PvZAI
{
	std::wstring Utf8ToWide(const std::string& theString)
	{
		if (theString.empty())
			return std::wstring();

		int aLength = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, theString.c_str(), (int)theString.size(), nullptr, 0);
		if (aLength <= 0)
			aLength = MultiByteToWideChar(CP_ACP, 0, theString.c_str(), (int)theString.size(), nullptr, 0);
		if (aLength <= 0)
			return std::wstring();

		std::wstring aResult(aLength, L'\0');
		int aConverted = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, theString.c_str(), (int)theString.size(), aResult.data(), aLength);
		if (aConverted <= 0)
			aConverted = MultiByteToWideChar(CP_ACP, 0, theString.c_str(), (int)theString.size(), aResult.data(), aLength);
		if (aConverted <= 0)
			return std::wstring();

		aResult.resize(aConverted);
		return aResult;
	}

	std::string WideToUtf8(const std::wstring& theString)
	{
		if (theString.empty())
			return std::string();

		int aLength = WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, theString.c_str(), (int)theString.size(), nullptr, 0, nullptr, nullptr);
		if (aLength <= 0)
			aLength = WideCharToMultiByte(CP_ACP, 0, theString.c_str(), (int)theString.size(), nullptr, 0, nullptr, nullptr);
		if (aLength <= 0)
			return std::string();

		std::string aResult(aLength, '\0');
		int aConverted = WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, theString.c_str(), (int)theString.size(), aResult.data(), aLength, nullptr, nullptr);
		if (aConverted <= 0)
			aConverted = WideCharToMultiByte(CP_ACP, 0, theString.c_str(), (int)theString.size(), aResult.data(), aLength, nullptr, nullptr);
		if (aConverted <= 0)
			return std::string();

		aResult.resize(aConverted);
		return aResult;
	}

	std::string TrimAscii(const std::string& theString)
	{
		size_t aStart = 0;
		while (aStart < theString.size() && (unsigned char)theString[aStart] <= 0x20)
			aStart++;

		size_t anEnd = theString.size();
		while (anEnd > aStart && (unsigned char)theString[anEnd - 1] <= 0x20)
			anEnd--;

		return theString.substr(aStart, anEnd - aStart);
	}

	std::string JsonEscape(const std::string& theString)
	{
		std::string aResult;
		aResult.reserve(theString.size() + 32);

		for (size_t i = 0; i < theString.size(); i++)
		{
			unsigned char aChar = (unsigned char)theString[i];
			switch (aChar)
			{
			case '"':  aResult += "\\\""; break;
			case '\\': aResult += "\\\\"; break;
			case '\b': aResult += "\\b"; break;
			case '\f': aResult += "\\f"; break;
			case '\n': aResult += "\\n"; break;
			case '\r': aResult += "\\r"; break;
			case '\t': aResult += "\\t"; break;
			default:
				if (aChar < 0x20)
				{
					char aBuffer[8];
					snprintf(aBuffer, sizeof(aBuffer), "\\u%04x", aChar);
					aResult += aBuffer;
				}
				else
				{
					aResult += (char)aChar;
				}
				break;
			}
		}

		return aResult;
	}

	void AppendUtf8(std::string& theOut, unsigned int theCodePoint)
	{
		if (theCodePoint <= 0x7F)
		{
			theOut += (char)theCodePoint;
		}
		else if (theCodePoint <= 0x7FF)
		{
			theOut += (char)(0xC0 | (theCodePoint >> 6));
			theOut += (char)(0x80 | (theCodePoint & 0x3F));
		}
		else if (theCodePoint <= 0xFFFF)
		{
			theOut += (char)(0xE0 | (theCodePoint >> 12));
			theOut += (char)(0x80 | ((theCodePoint >> 6) & 0x3F));
			theOut += (char)(0x80 | (theCodePoint & 0x3F));
		}
		else
		{
			theOut += (char)(0xF0 | (theCodePoint >> 18));
			theOut += (char)(0x80 | ((theCodePoint >> 12) & 0x3F));
			theOut += (char)(0x80 | ((theCodePoint >> 6) & 0x3F));
			theOut += (char)(0x80 | (theCodePoint & 0x3F));
		}
	}

	void Log(const std::string& theMessage)
	{
		std::ofstream aFile("PvZAI_debug.log", std::ios::out | std::ios::app);
		if (aFile.is_open())
		{
			aFile << theMessage << "\n";
			aFile.flush();
		}
	}
}

