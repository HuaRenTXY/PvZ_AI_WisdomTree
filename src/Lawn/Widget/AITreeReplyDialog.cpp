// SPDX-License-Identifier: LGPL-3.0-or-later
// PvZ AI Wisdom Tree fork of PvZ-Portable

#include "AITreeReplyDialog.h"
#include "../../LawnApp.h"
#include "../System/AIFont.h"
#include "../System/AIUtil.h"
#include <vector>

AITreeReplyDialog::AITreeReplyDialog(LawnApp* theApp, const std::string& theText)
	: LawnDialog(theApp, Dialogs::DIALOG_AI_REPLY, true,
		"Wisdom Tree", "", "OK", Dialog::BUTTONS_FOOTER)
{
	mApp = theApp;
	mText = theText;
	CalcSize(560, 260);
	Resize(80, 100, 640, 400);
}

AITreeReplyDialog::~AITreeReplyDialog() = default;

void AITreeReplyDialog::Draw(Graphics* g)
{
	LawnDialog::Draw(g);

	std::wstring aText = PvZAI::Utf8ToWide(mText);
	PvZ2Font* aFont = PvZ2Font::Get(17);
	std::vector<std::wstring> aLines;
	std::wstring aLine;
	for (wchar_t aChar : aText)
	{
		if (aChar == L'\r')
			continue;
		if (aChar == L'\n')
		{
			aLines.push_back(aLine);
			aLine.clear();
			continue;
		}
		aLine += aChar;
		if (aLine.size() >= 22)
		{
			aLines.push_back(aLine);
			aLine.clear();
		}
	}
	if (!aLine.empty())
		aLines.push_back(aLine);

	int aY = 150;
	int aLineSpacing = aFont->GetHeight() + 2;
	for (size_t i = 0; i < aLines.size() && aY < mHeight - 90; i++)
	{
		int aWidth = aFont->StringWidthW(aLines[i]);
		int aX = (mWidth - aWidth) / 2;
		aFont->DrawStringW(g, aX, aY, aLines[i], Color(255, 255, 255), Rect(0, 0, mWidth, mHeight));
		aY += aLineSpacing;
	}
}

