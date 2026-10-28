// SPDX-License-Identifier: LGPL-3.0-or-later
// PvZ AI Wisdom Tree fork of PvZ-Portable

#include "AITreeDialog.h"
#include "../../LawnApp.h"
#include "../LawnCommon.h"
#include "../../Resources.h"
#include "../System/AIClient.h"
#include "../System/AIFont.h"
#include "widget/WidgetManager.h"

AITreePromptDialog::AITreePromptDialog(LawnApp* theApp, bool theKeyMode)
	: LawnDialog(theApp, Dialogs::DIALOG_AI_PROMPT, true,
		theKeyMode ? "DeepSeek API Key" : "Ask the Wisdom Tree",
		theKeyMode ? "Enter your DeepSeek API key:" : "Enter a question for DeepSeek:",
		"[DIALOG_BUTTON_OK]",
		Dialog::BUTTONS_OK_CANCEL)
{
	mApp = theApp;
	mKeyMode = theKeyMode;
	mVerticalCenterText = false;
	mEditWidget = CreateEditWidget(0, this, this);
	mEditWidget->SetFont(PvZ2Font::Get(18));
	mEditWidget->mMaxChars = 200;
	mEditWidget->AddWidthCheckFont(FONT_BRIANNETOD16, 400);
	CalcSize(300, 80);
}

AITreePromptDialog::~AITreePromptDialog() = default;

void AITreePromptDialog::AddedToManager(WidgetManager* theWidgetManager)
{
	LawnDialog::AddedToManager(theWidgetManager);
	AddWidget(mEditWidget.get());
	theWidgetManager->SetFocus(mEditWidget.get());
}

void AITreePromptDialog::RemovedFromManager(WidgetManager* theWidgetManager)
{
	LawnDialog::RemovedFromManager(theWidgetManager);
	RemoveWidget(mEditWidget.get());
}

void AITreePromptDialog::Resize(int theX, int theY, int theWidth, int theHeight)
{
	LawnDialog::Resize(theX, theY, theWidth, theHeight);
	mEditWidget->Resize(mContentInsets.mLeft + 12, mHeight - 150, mWidth - mContentInsets.mLeft - mContentInsets.mRight - 24, 28);
}

int AITreePromptDialog::GetPreferredHeight(int theWidth)
{
	(void)theWidth;
	return 360;
}

void AITreePromptDialog::Draw(Graphics* g)
{
	LawnDialog::Draw(g);
	DrawEditBox(g, mEditWidget.get());
}

void AITreePromptDialog::ButtonDepress(int theId)
{
	if (theId == Dialog::ID_OK)
	{
		if (mKeyMode)
			PvZAI::SetApiKey(mEditWidget->mString);
		else
			mApp->BeginAITreeRequest(mEditWidget->mString);
		mApp->KillDialog(Dialogs::DIALOG_AI_PROMPT);
		return;
	}
	LawnDialog::ButtonDepress(theId);
}

void AITreePromptDialog::KeyDown(KeyCode theKey)
{
	if (theKey == KEYCODE_RETURN)
	{
		if (mKeyMode)
			PvZAI::SetApiKey(mEditWidget->mString);
		else
			mApp->BeginAITreeRequest(mEditWidget->mString);
		mApp->KillDialog(Dialogs::DIALOG_AI_PROMPT);
		return;
	}
	LawnDialog::KeyDown(theKey);
}

void AITreePromptDialog::EditWidgetText(int, const std::string&)
{
}

