// SPDX-License-Identifier: LGPL-3.0-or-later
// PvZ AI Wisdom Tree fork of PvZ-Portable

#include "AIClient.h"
#include "AIUtil.h"

#include <windows.h>
#include <winhttp.h>

#include <atomic>
#include <cstdio>
#include <fstream>
#include <mutex>
#include <thread>
#include <vector>

#pragma comment(lib, "winhttp.lib")

namespace PvZAI
{
	namespace
	{
		struct Config
		{
			std::string apiKey;
			std::string baseUrl = "https://api.deepseek.com";
			std::string model = "deepseek-chat";
		};

		struct State
		{
			std::mutex configMutex;
			Config config;
			bool loaded = false;
			std::wstring configPath;

			std::mutex resultMutex;
			std::atomic<bool> pending{ false };
			bool hasResult = false;
			Result result;
		};

		State& S()
		{
			static State* aState = new State();
			return *aState;
		}

		std::wstring GetExecutableDirectory()
		{
			wchar_t aBuffer[MAX_PATH * 4];
			DWORD aLength = GetModuleFileNameW(nullptr, aBuffer, (DWORD)(sizeof(aBuffer) / sizeof(aBuffer[0])));
			if (aLength == 0)
				return L".";

			std::wstring aPath(aBuffer, aLength);
			size_t aSlash = aPath.find_last_of(L"\\/");
			if (aSlash != std::wstring::npos)
				aPath.resize(aSlash);
			return aPath;
		}

		std::wstring GetRoamingConfigPath()
		{
			wchar_t aBuffer[MAX_PATH * 2];
			DWORD aLength = GetEnvironmentVariableW(L"APPDATA", aBuffer, (DWORD)(sizeof(aBuffer) / sizeof(aBuffer[0])));
			if (aLength == 0 || aLength >= sizeof(aBuffer) / sizeof(aBuffer[0]))
				return L"";

			std::wstring aDirectory(aBuffer, aLength);
			aDirectory += L"\\PvZAI";
			CreateDirectoryW(aDirectory.c_str(), nullptr);
			return aDirectory + L"\\PvZAI.ini";
		}

		std::wstring GetDefaultConfigPath()
		{
			return GetExecutableDirectory() + L"\\PvZAI.ini";
		}

		bool FileExists(const std::wstring& thePath)
		{
			DWORD aAttributes = GetFileAttributesW(thePath.c_str());
			return aAttributes != INVALID_FILE_ATTRIBUTES && !(aAttributes & FILE_ATTRIBUTE_DIRECTORY);
		}

		bool ReadConfigFile(const std::wstring& thePath, Config& theConfig)
		{
			std::ifstream aFile(thePath.c_str(), std::ios::binary);
			if (!aFile.is_open())
				return false;

			std::string aLine;
			bool aFirstLine = true;
			while (std::getline(aFile, aLine))
			{
				if (aFirstLine)
				{
					aFirstLine = false;
					if (aLine.size() >= 3 && (unsigned char)aLine[0] == 0xEF && (unsigned char)aLine[1] == 0xBB && (unsigned char)aLine[2] == 0xBF)
						aLine.erase(0, 3);
				}

				aLine = TrimAscii(aLine);
				if (aLine.empty() || aLine[0] == '#' || aLine[0] == ';')
					continue;

				size_t aEquals = aLine.find('=');
				if (aEquals == std::string::npos)
					continue;

				std::string aKey = TrimAscii(aLine.substr(0, aEquals));
				std::string aValue = TrimAscii(aLine.substr(aEquals + 1));

				if (aKey == "api_key")
					theConfig.apiKey = aValue;
				else if (aKey == "base_url" && !aValue.empty())
					theConfig.baseUrl = aValue;
				else if (aKey == "model" && !aValue.empty())
					theConfig.model = aValue;
			}

			return true;
		}

		bool WriteConfigFile(const std::wstring& thePath, const Config& theConfig)
		{
			std::ofstream aFile(thePath.c_str(), std::ios::binary | std::ios::trunc);
			if (!aFile.is_open())
				return false;

			aFile << "# PvZ AI mod settings\n";
			aFile << "api_key=" << theConfig.apiKey << "\n";
			aFile << "base_url=" << theConfig.baseUrl << "\n";
			aFile << "model=" << theConfig.model << "\n";
			return aFile.good();
		}

		void LoadConfigLocked()
		{
			State& aState = S();
			if (aState.loaded)
				return;
			aState.loaded = true;

			std::wstring aDefaultPath = GetDefaultConfigPath();
			if (FileExists(aDefaultPath))
			{
				aState.configPath = aDefaultPath;
				ReadConfigFile(aDefaultPath, aState.config);
				return;
			}

			std::wstring aRoamingPath = GetRoamingConfigPath();
			if (!aRoamingPath.empty() && FileExists(aRoamingPath))
			{
				aState.configPath = aRoamingPath;
				ReadConfigFile(aRoamingPath, aState.config);
				return;
			}

			aState.configPath = aDefaultPath;
		}

		Config SnapshotConfig()
		{
			State& aState = S();
			std::lock_guard<std::mutex> aLock(aState.configMutex);
			LoadConfigLocked();
			return aState.config;
		}

		std::string GetWin32Error(const char* thePrefix)
		{
			DWORD anError = GetLastError();
			std::string aResult = std::string(thePrefix) + " (code " + std::to_string((unsigned long)anError) + ")";

			wchar_t* aBuffer = nullptr;
			DWORD aLength = FormatMessageW(
				FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
				nullptr, anError, 0, (LPWSTR)&aBuffer, 0, nullptr);
			if (aLength > 0 && aBuffer != nullptr)
			{
				std::wstring aMessage(aBuffer, aLength);
				while (!aMessage.empty() && (aMessage.back() == L'\r' || aMessage.back() == L'\n'))
					aMessage.pop_back();
				aResult += ": " + WideToUtf8(aMessage);
				LocalFree(aBuffer);
			}
			return aResult;
		}

		bool ReadJsonString(const std::string& theJson, size_t& thePos, std::string& theOut)
		{
			if (thePos >= theJson.size() || theJson[thePos] != '"')
				return false;

			thePos++;
			theOut.clear();

			while (thePos < theJson.size())
			{
				char aChar = theJson[thePos++];
				if (aChar == '"')
					return true;

				if (aChar != '\\')
				{
					theOut += aChar;
					continue;
				}

				if (thePos >= theJson.size())
					return false;

				char anEscape = theJson[thePos++];
				switch (anEscape)
				{
				case '"': theOut += '"'; break;
				case '\\': theOut += '\\'; break;
				case '/': theOut += '/'; break;
				case 'b': theOut += '\b'; break;
				case 'f': theOut += '\f'; break;
				case 'n': theOut += '\n'; break;
				case 'r': theOut += '\r'; break;
				case 't': theOut += '\t'; break;
				case 'u':
				{
					if (thePos + 4 > theJson.size())
						return false;

					auto ReadHex = [&](size_t theOffset) -> unsigned int
					{
						unsigned int aValue = 0;
						for (size_t i = 0; i < 4; i++)
						{
							char aHex = theJson[theOffset + i];
							aValue <<= 4;
							if (aHex >= '0' && aHex <= '9') aValue |= (unsigned int)(aHex - '0');
							else if (aHex >= 'a' && aHex <= 'f') aValue |= (unsigned int)(aHex - 'a' + 10);
							else if (aHex >= 'A' && aHex <= 'F') aValue |= (unsigned int)(aHex - 'A' + 10);
							else return 0xFFFFFFFF;
						}
						return aValue;
					};

					unsigned int aCode = ReadHex(thePos);
					thePos += 4;
					if (aCode == 0xFFFFFFFF)
						return false;

					if (aCode >= 0xD800 && aCode <= 0xDBFF)
					{
						if (thePos + 6 <= theJson.size() && theJson[thePos] == '\\' && theJson[thePos + 1] == 'u')
						{
							unsigned int aLow = ReadHex(thePos + 2);
							if (aLow >= 0xDC00 && aLow <= 0xDFFF)
							{
								thePos += 6;
								aCode = 0x10000 + ((aCode - 0xD800) << 10) + (aLow - 0xDC00);
							}
						}
					}

					AppendUtf8(theOut, aCode);
					break;
				}
				default:
					theOut += anEscape;
					break;
				}
			}

			return false;
		}

		bool FindJsonString(const std::string& theJson, const std::string& theKey, std::string& theOut, size_t theStart = 0)
		{
			std::string aNeedle = "\"" + theKey + "\"";
			size_t aPos = theJson.find(aNeedle, theStart);
			if (aPos == std::string::npos)
				return false;

			aPos += aNeedle.size();
			while (aPos < theJson.size() && ((unsigned char)theJson[aPos] <= 0x20))
				aPos++;

			if (aPos >= theJson.size() || theJson[aPos] != ':')
				return false;
			aPos++;

			while (aPos < theJson.size() && ((unsigned char)theJson[aPos] <= 0x20))
				aPos++;

			if (aPos >= theJson.size() || theJson[aPos] != '"')
				return false;

			return ReadJsonString(theJson, aPos, theOut);
		}

		std::string ExtractApiError(const std::string& theJson)
		{
			size_t anErrorPos = theJson.find("\"error\"");
			if (anErrorPos == std::string::npos)
				return std::string();

			std::string aMessage;
			if (FindJsonString(theJson, "message", aMessage, anErrorPos))
				return TrimAscii(aMessage);
			return std::string();
		}

		std::string HttpPost(const std::string& theUrl, const std::string& theApiKey, const std::string& theBody, int& theStatus)
		{
			theStatus = 0;

			std::wstring aUrl = Utf8ToWide(theUrl);
			URL_COMPONENTS aComponents;
			ZeroMemory(&aComponents, sizeof(aComponents));
			aComponents.dwStructSize = sizeof(aComponents);
			aComponents.dwSchemeLength = (DWORD)-1;
			aComponents.dwHostNameLength = (DWORD)-1;
			aComponents.dwUrlPathLength = (DWORD)-1;
			aComponents.dwExtraInfoLength = (DWORD)-1;

			if (!WinHttpCrackUrl(aUrl.c_str(), (DWORD)aUrl.size(), 0, &aComponents))
				return GetWin32Error("WinHttpCrackUrl failed");

			std::wstring aHost(aComponents.lpszHostName, aComponents.dwHostNameLength);
			std::wstring aPath(aComponents.lpszUrlPath, aComponents.dwUrlPathLength);
			if (aComponents.dwExtraInfoLength > 0)
				aPath.append(aComponents.lpszExtraInfo, aComponents.dwExtraInfoLength);

			bool aSecure = aComponents.nScheme == INTERNET_SCHEME_HTTPS;
			bool aLocal = _wcsicmp(aHost.c_str(), L"127.0.0.1") == 0 || _wcsicmp(aHost.c_str(), L"localhost") == 0;
			DWORD anAccessType = aLocal ? WINHTTP_ACCESS_TYPE_NO_PROXY : WINHTTP_ACCESS_TYPE_DEFAULT_PROXY;

			HINTERNET aSession = WinHttpOpen(L"PvZ-AI/1.0", anAccessType, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
			if (aSession == nullptr)
				return GetWin32Error("WinHttpOpen failed");

			WinHttpSetTimeouts(aSession, 10000, 10000, 30000, 90000);

			if (aSecure)
			{
				DWORD aProtocols = WINHTTP_FLAG_SECURE_PROTOCOL_TLS1_2;
				WinHttpSetOption(aSession, WINHTTP_OPTION_SECURE_PROTOCOLS, &aProtocols, sizeof(aProtocols));
			}

			HINTERNET aConnection = WinHttpConnect(aSession, aHost.c_str(), aComponents.nPort, 0);
			if (aConnection == nullptr)
			{
				std::string anError = GetWin32Error("WinHttpConnect failed");
				WinHttpCloseHandle(aSession);
				return anError;
			}

			DWORD aFlags = aSecure ? WINHTTP_FLAG_SECURE : 0;
			HINTERNET aRequest = WinHttpOpenRequest(
				aConnection, L"POST", aPath.c_str(), nullptr,
				WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, aFlags);
			if (aRequest == nullptr)
			{
				std::string anError = GetWin32Error("WinHttpOpenRequest failed");
				WinHttpCloseHandle(aConnection);
				WinHttpCloseHandle(aSession);
				return anError;
			}

			std::wstring aHeaders = L"Content-Type: application/json; charset=utf-8\r\n";
			aHeaders += L"Authorization: Bearer ";
			aHeaders += Utf8ToWide(theApiKey);
			aHeaders += L"\r\n";
			aHeaders += L"Accept: application/json\r\n";

			BOOL aSent = WinHttpSendRequest(
				aRequest, aHeaders.c_str(), (DWORD)-1L,
				(LPVOID)theBody.data(), (DWORD)theBody.size(), (DWORD)theBody.size(), 0);
			if (!aSent)
			{
				std::string anError = GetWin32Error("WinHttpSendRequest failed");
				WinHttpCloseHandle(aRequest);
				WinHttpCloseHandle(aConnection);
				WinHttpCloseHandle(aSession);
				return anError;
			}

			if (!WinHttpReceiveResponse(aRequest, nullptr))
			{
				std::string anError = GetWin32Error("WinHttpReceiveResponse failed");
				WinHttpCloseHandle(aRequest);
				WinHttpCloseHandle(aConnection);
				WinHttpCloseHandle(aSession);
				return anError;
			}

			DWORD aStatusCode = 0;
			DWORD aStatusCodeSize = sizeof(aStatusCode);
			WinHttpQueryHeaders(
				aRequest,
				WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
				WINHTTP_HEADER_NAME_BY_INDEX,
				&aStatusCode, &aStatusCodeSize, WINHTTP_NO_HEADER_INDEX);
			theStatus = (int)aStatusCode;

			std::string aResponse;
			while (true)
			{
				DWORD aAvailable = 0;
				if (!WinHttpQueryDataAvailable(aRequest, &aAvailable) || aAvailable == 0)
					break;

				std::vector<char> aChunk(aAvailable);
				DWORD aRead = 0;
				if (!WinHttpReadData(aRequest, aChunk.data(), aAvailable, &aRead))
					break;

				aResponse.append(aChunk.data(), aRead);
			}

			WinHttpCloseHandle(aRequest);
			WinHttpCloseHandle(aConnection);
			WinHttpCloseHandle(aSession);

			return aResponse;
		}

		Result DoRequest(const std::string& thePrompt, int theTreeSize, const Config& theConfig)
		{
			Result aResult;

			std::string aKey = TrimAscii(theConfig.apiKey);
			if (aKey.empty())
			{
				aResult.error = "还没有设置 DeepSeek API Key。请先在游戏设置中填入有效的 API Key。";
				return aResult;
			}

			std::string aPrompt = TrimAscii(thePrompt);
			if (aPrompt.empty())
			{
				aResult.error = "问题不能为空。";
				return aResult;
			}

			if (aPrompt.size() > 1200)
				aPrompt.resize(1200);

			const char* aSystem =
				"你是《植物大战僵尸》禅境花园里那棵古老的智慧树。刚刚有人给你施了肥料，现在你要回答来者的问题。"
				"请用神秘、幽默、简短的中文说话，像在给出人生忠告；可以玩植物大战僵尸的梗，但不要提到自己是 AI 或语言模型。"
				"只输出你要说的话，不要加引号，不要解释，不要 Markdown，最多 80 个汉字。";

			std::string aUser = "来者问：";
			aUser += aPrompt;
			aUser += "\n（你现在的身高是 ";
			aUser += std::to_string(theTreeSize);
			aUser += " 英尺。）";

			std::string aBody;
			aBody.reserve(2048);
			aBody += "{\"model\":\"";
			aBody += JsonEscape(theConfig.model.empty() ? "deepseek-chat" : theConfig.model);
			aBody += "\",\"messages\":[{\"role\":\"system\",\"content\":\"";
			aBody += JsonEscape(aSystem);
			aBody += "\"},{\"role\":\"user\",\"content\":\"";
			aBody += JsonEscape(aUser);
			aBody += "\"}],\"temperature\":1.05,\"max_tokens\":320,\"stream\":false}";

			std::string aBase = theConfig.baseUrl;
			while (!aBase.empty() && (aBase.back() == '/' || aBase.back() == ' '))
				aBase.pop_back();
			if (aBase.empty())
				aBase = "https://api.deepseek.com";
			std::string aUrl = aBase + "/chat/completions";

			int aStatus = 0;
			std::string aResponse = HttpPost(aUrl, aKey, aBody, aStatus);
			aResult.statusCode = aStatus;

			if (aStatus == 0)
			{
				aResult.error = "连接 DeepSeek 失败：" + aResponse + "。请检查网络后重试。";
				return aResult;
			}

			if (aStatus != 200)
			{
				std::string anApiError = ExtractApiError(aResponse);
				if (aStatus == 401 || aStatus == 403)
				{
					aResult.error = "DeepSeek API Key 无效或已过期。请打开游戏设置检查 API Key，然后再试一次。";
				}
				else if (aStatus == 402)
				{
					aResult.error = "DeepSeek 账户余额不足。请充值后再来问智慧树吧。";
				}
				else if (aStatus == 429)
				{
					aResult.error = "请求太频繁了，智慧树需要喘口气。请稍后再试。";
				}
				else
				{
					aResult.error = "DeepSeek 返回错误（HTTP " + std::to_string(aStatus) + "）。";
				}

				if (!anApiError.empty())
					aResult.error += " " + anApiError;
				return aResult;
			}

			std::string aContent;
			if (!FindJsonString(aResponse, "content", aContent))
			{
				std::string aReasoning;
				if (FindJsonString(aResponse, "reasoning_content", aReasoning))
					aContent = aReasoning;
			}

			aContent = TrimAscii(aContent);
			if (aContent.size() >= 2 && aContent.front() == '"' && aContent.back() == '"')
				aContent = aContent.substr(1, aContent.size() - 2);

			if (aContent.empty())
			{
				aResult.error = "DeepSeek 没有返回内容。请稍后再试。";
				return aResult;
			}

			aResult.success = true;
			aResult.content = aContent;
			FindJsonString(aResponse, "reasoning_content", aResult.reasoning);
			return aResult;
		}
	}

	void LoadConfig()
	{
		State& aState = S();
		std::lock_guard<std::mutex> aLock(aState.configMutex);
		LoadConfigLocked();
	}

	std::string GetApiKey()
	{
		return SnapshotConfig().apiKey;
	}

	void SetApiKey(const std::string& theKey)
	{
		State& aState = S();
		std::lock_guard<std::mutex> aLock(aState.configMutex);
		LoadConfigLocked();
		aState.config.apiKey = TrimAscii(theKey);

		if (aState.configPath.empty())
			aState.configPath = GetDefaultConfigPath();

		if (!WriteConfigFile(aState.configPath, aState.config))
		{
			std::wstring aRoamingPath = GetRoamingConfigPath();
			if (!aRoamingPath.empty())
			{
				aState.configPath = aRoamingPath;
				WriteConfigFile(aState.configPath, aState.config);
			}
		}
	}

	std::string GetBaseUrl()
	{
		return SnapshotConfig().baseUrl;
	}

	std::string GetModel()
	{
		return SnapshotConfig().model;
	}

	bool HasApiKey()
	{
		std::string aKey = TrimAscii(GetApiKey());
		return aKey.size() >= 8;
	}

	std::string GetConfigPath()
	{
		State& aState = S();
		std::lock_guard<std::mutex> aLock(aState.configMutex);
		LoadConfigLocked();
		return WideToUtf8(aState.configPath);
	}

	const char* MissingKeyMessage()
	{
		return "还没有设置 DeepSeek API Key。\n请打开游戏设置，在“DeepSeek API Key”一栏填入有效的 Key，再来找智慧树吧。";
	}

	bool StartRequest(const std::string& thePrompt, int theTreeSize)
	{
		State& aState = S();

		{
			std::lock_guard<std::mutex> aLock(aState.resultMutex);
			if (aState.pending)
				return false;
			aState.pending = true;
			aState.hasResult = false;
		}

		Config aConfig = SnapshotConfig();
		std::string aPrompt = thePrompt;

		std::thread aThread([aPrompt, theTreeSize, aConfig]()
		{
			Result aResult = DoRequest(aPrompt, theTreeSize, aConfig);
			State& aState = S();
			std::lock_guard<std::mutex> aLock(aState.resultMutex);
			aState.result = aResult;
			aState.hasResult = true;
			aState.pending = false;
		});
		aThread.detach();

		return true;
	}

	bool IsPending()
	{
		return S().pending.load();
	}

	bool Poll(Result& theResult)
	{
		State& aState = S();
		std::lock_guard<std::mutex> aLock(aState.resultMutex);
		if (!aState.hasResult)
			return false;

		theResult = aState.result;
		aState.hasResult = false;
		return true;
	}

	void Reset()
	{
		State& aState = S();
		std::lock_guard<std::mutex> aLock(aState.resultMutex);
		aState.hasResult = false;
	}

	std::string UserFacingError(const Result& theResult)
	{
		if (!theResult.error.empty())
			return theResult.error;
		return "智慧树暂时说不出话。请稍后再试。";
	}
}

