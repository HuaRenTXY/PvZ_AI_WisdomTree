// SPDX-License-Identifier: LGPL-3.0-or-later
// PvZ AI Wisdom Tree fork of PvZ-Portable

#include "AIFont.h"
#include "AIUtil.h"

#include "graphics/MemoryImage.h"

#include <windows.h>

#include <algorithm>
#include <map>

namespace
{
	bool EnsureFontResource()
	{
		static bool sChecked = false;
		static bool sAvailable = false;
		if (sChecked)
			return sAvailable;
		sChecked = true;

		wchar_t aLocalAppData[MAX_PATH * 2];
		DWORD aLength = GetEnvironmentVariableW(L"LOCALAPPDATA", aLocalAppData, (DWORD)(sizeof(aLocalAppData) / sizeof(aLocalAppData[0])));
		if (aLength > 0 && aLength < sizeof(aLocalAppData) / sizeof(aLocalAppData[0]))
		{
			std::wstring aPath(aLocalAppData, aLength);
			aPath += L"\\Microsoft\\Windows\\Fonts\\PVZ2 REGULAR.TTF";
			if (GetFileAttributesW(aPath.c_str()) != INVALID_FILE_ATTRIBUTES)
				sAvailable = AddFontResourceExW(aPath.c_str(), FR_PRIVATE, nullptr) > 0;
		}

		// The font may already be installed system-wide; the caller still verifies the face.
		return sAvailable;
	}

	bool IsPvZ2Face(HDC theDC)
	{
		wchar_t aFace[LF_FACESIZE] = { 0 };
		if (GetTextFaceW(theDC, LF_FACESIZE, aFace) <= 0)
			return false;

		std::wstring aName(aFace);
		return aName.find(L"PvZ2") != std::wstring::npos || aName.find(L"PVZ2") != std::wstring::npos;
	}
}

PvZ2Font::PvZ2Font(int thePixelSize) :
	mDC(nullptr),
	mFont(nullptr),
	mPixelSize(thePixelSize)
{
	EnsureFontResource();

	mDC = CreateCompatibleDC(nullptr);

	auto MakeFont = [this](const wchar_t* theFace)
	{
		if (mFont != nullptr)
		{
			DeleteObject(mFont);
			mFont = nullptr;
		}

		mFont = CreateFontW(
			-mPixelSize, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
			DEFAULT_CHARSET, OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS,
			ANTIALIASED_QUALITY, DEFAULT_PITCH | FF_DONTCARE, theFace);
	};

	MakeFont(L"PvZ2 Regular");

	if (mDC != nullptr && mFont != nullptr)
	{
		HGDIOBJ anOldFont = SelectObject(mDC, mFont);
		if (!IsPvZ2Face(mDC))
		{
			SelectObject(mDC, anOldFont);
			MakeFont(L"Microsoft YaHei");
		}
		else
		{
			SelectObject(mDC, anOldFont);
		}
	}

	mAscent = thePixelSize;
	mHeight = thePixelSize + 2;
	mAscentPadding = 0;
	mLineSpacingOffset = 0;

	if (mDC != nullptr && mFont != nullptr)
	{
		HGDIOBJ anOldFont = SelectObject(mDC, mFont);
		TEXTMETRICW aMetrics;
		if (GetTextMetricsW(mDC, &aMetrics))
		{
			mHeight = aMetrics.tmHeight;
			mAscent = aMetrics.tmAscent;
		}
		SelectObject(mDC, anOldFont);
	}
}

PvZ2Font::~PvZ2Font()
{
	if (mFont != nullptr)
		DeleteObject(mFont);
	if (mDC != nullptr)
		DeleteDC(mDC);
}

int PvZ2Font::StringWidthW(const std::wstring& theString)
{
	if (theString.empty() || mDC == nullptr || mFont == nullptr)
		return 0;

	HGDIOBJ anOldFont = SelectObject(mDC, mFont);
	SIZE aSize = { 0, 0 };
	GetTextExtentPoint32W(mDC, theString.c_str(), (int)theString.size(), &aSize);
	SelectObject(mDC, anOldFont);
	return aSize.cx;
}

int PvZ2Font::StringWidth(std::string_view theString)
{
	return StringWidthW(PvZAI::Utf8ToWide(std::string(theString)));
}

int PvZ2Font::CharWidth(char32_t theChar)
{
	if (theChar == 0)
		return 0;
	return StringWidthW(std::wstring(1, (wchar_t)theChar));
}

int PvZ2Font::CharWidthKern(char32_t theChar, char32_t thePrevChar)
{
	(void)thePrevChar;
	return CharWidth(theChar);
}

void PvZ2Font::DrawStringW(Sexy::Graphics* g, int theX, int theY, const std::wstring& theString, const Sexy::Color& theColor, const Sexy::Rect& theClipRect)
{
	(void)theClipRect;
	if (g == nullptr || theString.empty() || mDC == nullptr || mFont == nullptr)
		return;

	SIZE aSize = { 0, 0 };
	HGDIOBJ anOldFont = SelectObject(mDC, mFont);
	GetTextExtentPoint32W(mDC, theString.c_str(), (int)theString.size(), &aSize);
	SelectObject(mDC, anOldFont);

	int aWidth = aSize.cx + 3;
	int aHeight = mHeight + 3;
	if (aWidth <= 0 || aHeight <= 0)
		return;

	BITMAPINFO aInfo;
	ZeroMemory(&aInfo, sizeof(aInfo));
	aInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	aInfo.bmiHeader.biWidth = aWidth;
	aInfo.bmiHeader.biHeight = -aHeight;
	aInfo.bmiHeader.biPlanes = 1;
	aInfo.bmiHeader.biBitCount = 32;
	aInfo.bmiHeader.biCompression = BI_RGB;

	void* aBits = nullptr;
	HBITMAP aBitmap = CreateDIBSection(mDC, &aInfo, DIB_RGB_COLORS, &aBits, nullptr, 0);
	if (aBitmap == nullptr || aBits == nullptr)
	{
		if (aBitmap != nullptr)
			DeleteObject(aBitmap);
		return;
	}

	HDC aDC = CreateCompatibleDC(mDC);
	HGDIOBJ anOldBitmap = SelectObject(aDC, aBitmap);
	HGDIOBJ anOldTextFont = SelectObject(aDC, mFont);

	RECT aRect = { 0, 0, aWidth, aHeight };
	FillRect(aDC, &aRect, (HBRUSH)GetStockObject(BLACK_BRUSH));
	SetBkMode(aDC, TRANSPARENT);
	SetTextColor(aDC, RGB(255, 255, 255));
	TextOutW(aDC, 1, 1, theString.c_str(), (int)theString.size());
	GdiFlush();

	SelectObject(aDC, anOldTextFont);
	SelectObject(aDC, anOldBitmap);
	DeleteDC(aDC);

	Sexy::MemoryImage aImage;
	aImage.Create(aWidth, aHeight);
	uint32_t* aDest = aImage.GetBits();
	const unsigned char* aSrc = (const unsigned char*)aBits;

	int aRed = std::max(0, std::min(255, theColor.GetRed()));
	int aGreen = std::max(0, std::min(255, theColor.GetGreen()));
	int aBlue = std::max(0, std::min(255, theColor.GetBlue()));
	int aColorAlpha = std::max(0, std::min(255, theColor.GetAlpha()));

	for (int i = 0; i < aWidth * aHeight; i++)
	{
		int aBlueValue = aSrc[i * 4 + 0];
		int aGreenValue = aSrc[i * 4 + 1];
		int aRedValue = aSrc[i * 4 + 2];
		int aAlpha = std::max(aBlueValue, std::max(aGreenValue, aRedValue));
		aAlpha = aAlpha * aColorAlpha / 255;

		if (aAlpha <= 0)
		{
			aDest[i] = 0;
		}
		else
		{
			aDest[i] = ((uint32_t)aAlpha << 24) | ((uint32_t)aRed << 16) | ((uint32_t)aGreen << 8) | (uint32_t)aBlue;
		}
	}

	aImage.BitsChanged();
	g->DrawImage(&aImage, theX, theY - mAscent + 1);
	DeleteObject(aBitmap);
}

void PvZ2Font::DrawString(Sexy::Graphics* g, int theX, int theY, std::string_view theString, const Sexy::Color& theColor, const Sexy::Rect& theClipRect)
{
	DrawStringW(g, theX, theY, PvZAI::Utf8ToWide(std::string(theString)), theColor, theClipRect);
}

Sexy::_Font* PvZ2Font::Duplicate()
{
	return new PvZ2Font(mPixelSize);
}

PvZ2Font* PvZ2Font::Get(int thePixelSize)
{
	static std::map<int, PvZ2Font*>* sFonts = new std::map<int, PvZ2Font*>();
	auto anItr = sFonts->find(thePixelSize);
	if (anItr != sFonts->end())
		return anItr->second;

	PvZ2Font* aFont = new PvZ2Font(thePixelSize);
	(*sFonts)[thePixelSize] = aFont;
	return aFont;
}

namespace PvZAI
{
	std::vector<TextLine> WrapText(PvZ2Font& theFont, const std::wstring& theText, int theMaxWidth)
	{
		std::vector<TextLine> aLines;
		std::wstring aLine;

		auto PushLine = [&]()
		{
			while (!aLine.empty() && aLine.back() == L' ')
				aLine.pop_back();

			if (!aLine.empty())
			{
				TextLine aTextLine;
				aTextLine.text = aLine;
				aTextLine.width = theFont.StringWidthW(aLine);
				aLines.push_back(aTextLine);
			}
			aLine.clear();
		};

		for (wchar_t aChar : theText)
		{
			if (aChar == L'\r')
				continue;

			if (aChar == L'\n')
			{
				PushLine();
				continue;
			}

			std::wstring aCandidate = aLine;
			aCandidate += aChar;
			int aWidth = theFont.StringWidthW(aCandidate);

			if (aWidth > theMaxWidth && !aLine.empty())
			{
				size_t aSpace = aLine.find_last_of(L' ');
				if (aSpace != std::wstring::npos && aSpace > 0 && aSpace + 1 < aLine.size())
				{
					std::wstring aBefore = aLine.substr(0, aSpace);
					while (!aBefore.empty() && aBefore.back() == L' ')
						aBefore.pop_back();

					if (!aBefore.empty())
					{
						TextLine aTextLine;
						aTextLine.text = aBefore;
						aTextLine.width = theFont.StringWidthW(aBefore);
						aLines.push_back(aTextLine);
					}

					aLine = aLine.substr(aSpace + 1);
					while (!aLine.empty() && aLine.front() == L' ')
						aLine.erase(aLine.begin());
					aLine += aChar;
				}
				else
				{
					PushLine();
					aLine += aChar;
				}
			}
			else
			{
				aLine = aCandidate;
			}
		}

		PushLine();
		if (aLines.empty())
		{
			TextLine aTextLine;
			aLines.push_back(aTextLine);
		}
		return aLines;
	}

	int MeasureWrappedHeight(const std::vector<TextLine>& theLines, int theLineSpacing)
	{
		return (int)theLines.size() * theLineSpacing;
	}

	void DrawWrappedText(Sexy::Graphics* g, const std::string& theText, const Sexy::Rect& theRect, int theFontSize, const Sexy::Color& theColor, bool theCenterHorizontally, bool theCenterVertically)
	{
		if (g == nullptr || theRect.mWidth <= 0 || theRect.mHeight <= 0)
			return;

		std::wstring aText = Utf8ToWide(theText);
		if (aText.empty())
			return;

		int aFontSize = std::max(12, theFontSize);
		PvZ2Font* aFont = PvZ2Font::Get(aFontSize);
		std::vector<TextLine> aLines = WrapText(*aFont, aText, theRect.mWidth);
		int aLineSpacing = aFont->GetHeight() + 2;

		while (aLines.size() * aLineSpacing > theRect.mHeight && aFontSize > 12)
		{
			aFontSize--;
			aFont = PvZ2Font::Get(aFontSize);
			aLines = WrapText(*aFont, aText, theRect.mWidth);
			aLineSpacing = aFont->GetHeight() + 2;
		}

		int aTotalHeight = (int)aLines.size() * aLineSpacing;
		int aStartY = theCenterVertically ? theRect.mY + std::max(0, (theRect.mHeight - aTotalHeight) / 2) : theRect.mY;

		for (size_t i = 0; i < aLines.size(); i++)
		{
			int aLineY = aStartY + (int)i * aLineSpacing;
			if (aLineY > theRect.mY + theRect.mHeight)
				break;

			int aX = theRect.mX;
			if (theCenterHorizontally)
				aX += std::max(0, (theRect.mWidth - aLines[i].width) / 2);

			aFont->DrawStringW(g, aX, aLineY + aFont->GetAscent(), aLines[i].text, theColor, theRect);
		}
	}

	void DrawCenteredText(Sexy::Graphics* g, const std::string& theText, int theCenterX, int theBaselineY, int theFontSize, const Sexy::Color& theColor)
	{
		std::wstring aText = Utf8ToWide(theText);
		if (g == nullptr || aText.empty())
			return;

		PvZ2Font* aFont = PvZ2Font::Get(theFontSize);
		int aWidth = aFont->StringWidthW(aText);
		Sexy::Rect aClip(0, 0, 10000, 10000);
		aFont->DrawStringW(g, theCenterX - aWidth / 2, theBaselineY, aText, theColor, aClip);
	}

	int DrawSingleLine(Sexy::Graphics* g, const std::string& theText, int theX, int theBaselineY, int theMaxWidth, int theFontSize, const Sexy::Color& theColor, bool theKeepTail)
	{		std::wstring aText = Utf8ToWide(theText);
		if (g == nullptr || aText.empty())
			return 0;

		PvZ2Font* aFont = PvZ2Font::Get(theFontSize);
		if (theKeepTail)
		{
			while (!aText.empty() && aFont->StringWidthW(aText) > theMaxWidth)
			{
				aText.erase(aText.begin());
				if (!aText.empty() && aText.front() >= 0xDC00 && aText.front() <= 0xDFFF)
					aText.erase(aText.begin());
			}
		}
		else
		{
			while (!aText.empty() && aFont->StringWidthW(aText) > theMaxWidth)
				aText.pop_back();
		}

		int aWidth = aFont->StringWidthW(aText);
		Sexy::Rect aClip(0, 0, 10000, 10000);
		aFont->DrawStringW(g, theX, theBaselineY, aText, theColor, aClip);
		return aWidth;
	}

	const char* ThinkingText()
	{
		return "智慧树正在思考……";
	}
}

