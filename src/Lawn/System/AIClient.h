// SPDX-License-Identifier: LGPL-3.0-or-later
// PvZ AI Wisdom Tree fork of PvZ-Portable

#ifndef __PVZ_AI_CLIENT_H__
#define __PVZ_AI_CLIENT_H__

#include <string>

namespace PvZAI
{
	struct Result
	{
		bool success = false;
		int statusCode = 0;
		std::string content;
		std::string reasoning;
		std::string error;
	};

	void LoadConfig();
	std::string GetApiKey();
	void SetApiKey(const std::string& theKey);
	std::string GetBaseUrl();
	std::string GetModel();
	bool HasApiKey();
	std::string GetConfigPath();
	const char* MissingKeyMessage();

	bool StartRequest(const std::string& thePrompt, int theTreeSize);
	bool IsPending();
	bool Poll(Result& theResult);
	void Reset();

	// Returns the exact UTF-8 bytes for UI text.
	std::string UserFacingError(const Result& theResult);
}

#endif

