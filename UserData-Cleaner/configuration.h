#pragma once
#include "includes.h"

namespace Config
{
	inline bool doTraceLogs = false;
	inline bool doBackups = false;
	inline bool deleteAppManifest = false;
	inline std::vector<std::wstring> ignoredAccounts;
	inline fs::path configPath;

	inline std::wstring GetConfig(LPCWSTR section, LPCWSTR variableName, LPCWSTR defaultVal) {
		wchar_t temp[420];
		const int result = GetPrivateProfileStringW(
			section, variableName, defaultVal, temp, std::size(temp), configPath.c_str()
		);
		return std::wstring(temp, result);
	}

	inline bool SetConfig(LPCWSTR section, LPCWSTR variableName, LPCWSTR value) {
		return WritePrivateProfileStringW(section, variableName, value, configPath.c_str());
	}

	inline bool FirstTimeSetup(const std::wstring& curCfgPath) {
		configPath = fs::path(curCfgPath);

		bool returnVar = true;
		//create config file if it doesn't exist
		if (!exists(configPath)) {
			returnVar &= SetConfig(L"General", L"Backup-Files", L"1");
			returnVar &= SetConfig(L"General", L"Trace-Logs", L"0");
			returnVar &= SetConfig(L"General", L"IgnoreUDataFor-0", L"you can expand this however you like with IgnoreUDataFor-1...");
			returnVar &= SetConfig(L"General", L"Delete-AppManifest", L"0");
		}
		return returnVar;
	}

	inline void InitVariables() {
		//populate config variables, boom? :)
		doBackups = std::stoi(GetConfig(L"General", L"Backup-Files", L"1"));
		doTraceLogs = std::stoi(GetConfig(L"General", L"Trace-Logs", L"0"));
		deleteAppManifest = std::stoi(GetConfig(L"General", L"Delete-AppManifest", L"0"));
		//i hate how i did the config saving now..
		while (true) {
			static int index = 0;
			std::wstring currentConfigString = L"IgnoreUDataFor-" + std::to_wstring(index);
			std::wstring configVariable = GetConfig(L"General", currentConfigString.c_str(), L"0");

			index++;

			if (configVariable == L"0")
				break;

			ignoredAccounts.push_back(configVariable);
		}
	}
}
