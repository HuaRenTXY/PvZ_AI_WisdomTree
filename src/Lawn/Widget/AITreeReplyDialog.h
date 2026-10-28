// SPDX-License-Identifier: LGPL-3.0-or-later
// PvZ AI Wisdom Tree fork of PvZ-Portable

#pragma once

#include "LawnDialog.h"
#include <string>

class LawnApp;

class AITreeReplyDialog : public LawnDialog
{
public:
	AITreeReplyDialog(LawnApp* theApp, const std::string& theText);
	~AITreeReplyDialog() override;
	void Draw(Sexy::Graphics* g) override;

private:
	LawnApp* mApp;
	std::string mText;
};

