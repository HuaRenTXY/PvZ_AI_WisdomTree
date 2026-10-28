// SPDX-License-Identifier: LGPL-3.0-or-later
// PvZ AI Wisdom Tree fork of PvZ-Portable

#pragma once

#include "LawnDialog.h"
#include "widget/EditListener.h"
#include <memory>

class LawnApp;
class LawnEditWidget;

class AITreePromptDialog : public LawnDialog, public EditListener
{
public:
	AITreePromptDialog(LawnApp* theApp, bool theKeyMode = false);
	~AITreePromptDialog() override;

	void AddedToManager(Sexy::WidgetManager* theWidgetManager) override;
	void RemovedFromManager(Sexy::WidgetManager* theWidgetManager) override;
	int GetPreferredHeight(int theWidth) override;
	void Resize(int theX, int theY, int theWidth, int theHeight) override;
	void Draw(Sexy::Graphics* g) override;
	void ButtonDepress(int theId) override;
	void KeyDown(Sexy::KeyCode theKey) override;
	void EditWidgetText(int theId, const std::string& theString) override;

private:
	LawnApp* mApp;
	bool mKeyMode;
	std::unique_ptr<LawnEditWidget> mEditWidget;
};

