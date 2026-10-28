// SPDX-License-Identifier: LGPL-3.0-or-later
// PvZ AI Wisdom Tree fork of PvZ-Portable

#ifndef __PVZ_AI_FONT_H__
#define __PVZ_AI_FONT_H__

#include "graphics/Font.h"
#include "graphics/Graphics.h"
#include "misc/Rect.h"

#include <string>
#include <string_view>
#include <vector>

class PvZ2Font : public Sexy::_Font
{
public:
	explicit PvZ2Font(int thePixelSize);
	~PvZ2Font() override;

	int StringWidth(std::string_view theString) override;
	int CharWidth(char32_t theChar) override;
	int CharWidthKern(char32_t theChar, char32_t thePrevChar) override;
	void DrawString(Sexy::Graphics* g, int theX, int theY, std::string_view theString, const Sexy::Color& theColor, const Sexy::Rect& theClipRect) override;
	Sexy::_Font* Duplicate() override;

	int StringWidthW(const std::wstring& theString);
	void DrawStringW(Sexy::Graphics* g, int theX, int theY, const std::wstring& theString, const Sexy::Color& theColor, const Sexy::Rect& theClipRect);

	static PvZ2Font* Get(int thePixelSize);

private:
	HDC mDC;
	HFONT mFont;
	int mPixelSize;
};

namespace PvZAI
{
	struct TextLine
	{
		std::wstring text;
		int width = 0;
	};

	std::vector<TextLine> WrapText(PvZ2Font& theFont, const std::wstring& theText, int theMaxWidth);
	int MeasureWrappedHeight(const std::vector<TextLine>& theLines, int theLineSpacing);
	void DrawWrappedText(Sexy::Graphics* g, const std::string& theText, const Sexy::Rect& theRect, int theFontSize, const Sexy::Color& theColor, bool theCenterHorizontally = true, bool theCenterVertically = true);
	void DrawCenteredText(Sexy::Graphics* g, const std::string& theText, int theCenterX, int theBaselineY, int theFontSize, const Sexy::Color& theColor);
	int DrawSingleLine(Sexy::Graphics* g, const std::string& theText, int theX, int theBaselineY, int theMaxWidth, int theFontSize, const Sexy::Color& theColor, bool theKeepTail = true);
	const char* ThinkingText();
}

#endif

