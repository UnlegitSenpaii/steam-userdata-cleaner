#pragma once
#include "includes.h"

namespace Config
{
	inline bool doTraceLogs = false;
	inline bool doBackups = false;

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
		}
		return returnVar;
	}

	inline void InitVariables() {
		//populate config variables, boom? :)
		doBackups = std::stoi(GetConfig(L"General", L"Backup-Files", L"1"));
		doTraceLogs = std::stoi(GetConfig(L"General", L"Trace-Logs", L"0"));
	}
}
