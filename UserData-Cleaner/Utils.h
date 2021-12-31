#pragma once
#include "configuration.h"
#include "includes.h"

namespace Utils
{
	inline bool Print(const wchar_t* fmt, ...) {
		HANDLE conOutHandle = GetStdHandle(STD_OUTPUT_HANDLE);
		if (!conOutHandle)
			return false;
		wchar_t buf[4096];
		va_list va;

		va_start(va, fmt);
		_vsnwprintf_s(buf, 4096, fmt, va);
		va_end(va);

		return !!WriteConsole(conOutHandle, buf, wcslen(buf), nullptr, nullptr);
	}

	inline bool Trace(const wchar_t* fmt, ...) {
		if (!Config::doTraceLogs)
			return true;

		WORD m_currentConsoleAttr{};
		CONSOLE_SCREEN_BUFFER_INFO csbi;
		HANDLE conOutHandle = GetStdHandle(STD_OUTPUT_HANDLE);

		if (!conOutHandle)
			return false;

		if (GetConsoleScreenBufferInfo(conOutHandle, &csbi))
			m_currentConsoleAttr = csbi.wAttributes;

		SetConsoleTextAttribute(conOutHandle, 8); //LIGHT GREY
		Print(L"[TRACE] ");

		wchar_t buf[4096];
		va_list va;

		va_start(va, fmt);
		_vsnwprintf_s(buf, 4096, fmt, va);
		va_end(va);
		bool returnvar = !!WriteConsole(conOutHandle, buf, wcslen(buf), nullptr, nullptr);
		SetConsoleTextAttribute(conOutHandle, m_currentConsoleAttr);
		return returnvar;
	}

	inline bool PrintError(const wchar_t* fmt, ...) {
		WORD m_currentConsoleAttr{};
		CONSOLE_SCREEN_BUFFER_INFO csbi;
		HANDLE conOutHandle = GetStdHandle(STD_OUTPUT_HANDLE);

		if (!conOutHandle)
			return false;

		if (GetConsoleScreenBufferInfo(conOutHandle, &csbi))
			m_currentConsoleAttr = csbi.wAttributes;

		SetConsoleTextAttribute(conOutHandle, 12); //INTENSE RED
		Print(L"[ERROR] ");

		SetConsoleTextAttribute(conOutHandle, m_currentConsoleAttr);
		wchar_t buf[4096];
		va_list va;

		va_start(va, fmt);
		_vsnwprintf_s(buf, 4096, fmt, va);
		va_end(va);

		return !!WriteConsole(conOutHandle, buf, wcslen(buf), nullptr, nullptr);
	}

	inline bool PrintWarning(const wchar_t* fmt, ...) {
		WORD m_currentConsoleAttr{};
		CONSOLE_SCREEN_BUFFER_INFO csbi;
		HANDLE conOutHandle = GetStdHandle(STD_OUTPUT_HANDLE);

		if (!conOutHandle)
			return false;

		if (GetConsoleScreenBufferInfo(conOutHandle, &csbi))
			m_currentConsoleAttr = csbi.wAttributes;

		SetConsoleTextAttribute(conOutHandle, 14); //INTENSE YELLOW
		Print(L"[WARNING] ");

		SetConsoleTextAttribute(conOutHandle, m_currentConsoleAttr);
		wchar_t buf[4096];
		va_list va;

		va_start(va, fmt);
		_vsnwprintf_s(buf, 4096, fmt, va);
		va_end(va);

		return !!WriteConsole(conOutHandle, buf, wcslen(buf), nullptr, nullptr);
	}

	inline void FatalError(const wchar_t* fmt, ...) {
		WORD m_currentConsoleAttr{};
		CONSOLE_SCREEN_BUFFER_INFO csbi;
		HANDLE conOutHandle = GetStdHandle(STD_OUTPUT_HANDLE);

		if (!conOutHandle)
			return;

		if (GetConsoleScreenBufferInfo(conOutHandle, &csbi))
			m_currentConsoleAttr = csbi.wAttributes;

		SetConsoleTextAttribute(conOutHandle, 12); //INTENSE RED
		Print(L"[FATAL-ERROR] ");
		SetConsoleTextAttribute(conOutHandle, m_currentConsoleAttr);
		wchar_t buf[4096];
		va_list va;

		va_start(va, fmt);
		_vsnwprintf_s(buf, 4096, fmt, va);
		va_end(va);
		WriteConsole(conOutHandle, buf, wcslen(buf), nullptr, nullptr);
		system("pause"); //function is no thread safe :vibe:
		exit(1); //function is no thread safe :vibe:
	}

	inline std::wstring GetEnviromentVariable(const std::wstring& var) {
		//rip getenv :(
		//https://docs.microsoft.com/de-de/cpp/c-runtime-library/reference/dupenv-s-wdupenv-s
		wchar_t* pValue;
		size_t len;

		if (_wdupenv_s(&pValue, &len, var.c_str()))
			return L"Not Found";
		return pValue;
	}

	inline std::wstring GetStringValueFromHKLM(const std::wstring& regSubKey, const std::wstring& regValue) {
		//modified version of https://stackoverflow.com/a/50821858
		size_t bufferSize = 4096;
		std::wstring valueBuf;
		valueBuf.resize(bufferSize);
		auto cbData = static_cast<DWORD>(bufferSize) * sizeof(wchar_t);
		auto rc = RegGetValue(
			HKEY_LOCAL_MACHINE, regSubKey.c_str(), regValue.c_str(), RRF_RT_REG_SZ, nullptr, valueBuf.data(), &cbData
		);
		while (rc == ERROR_MORE_DATA) {
			cbData /= sizeof(wchar_t);
			if (cbData > static_cast<DWORD>(bufferSize)) {
				bufferSize = static_cast<size_t>(cbData);
			}
			else {
				bufferSize *= 2;
				cbData = static_cast<DWORD>(bufferSize) * sizeof(wchar_t);
			}
			valueBuf.resize(bufferSize);
			rc = RegGetValue(
				HKEY_LOCAL_MACHINE, regSubKey.c_str(), regValue.c_str(), RRF_RT_REG_SZ, nullptr, valueBuf.data(),
				&cbData
			);
		}

		if (rc != NO_ERROR)
			throw std::runtime_error("Failed to read from registry, error code: " + std::to_string(rc));

		cbData /= sizeof(wchar_t);
		valueBuf.resize(static_cast<size_t>(cbData - 1));
		return valueBuf;
	}

	inline BOOL RegDelnodeRecurse(HKEY hKeyRoot, LPTSTR lpSubKey) {
		//https://docs.microsoft.com/en-us/windows/win32/sysinfo/deleting-a-key-with-subkeys
		LPTSTR lpEnd;
		LONG lResult;
		DWORD dwSize;
		TCHAR szName[MAX_PATH];
		HKEY hKey;
		FILETIME ftWrite;

		// First, see if we can delete the key without having
		// to recurse.

		lResult = RegDeleteKey(hKeyRoot, lpSubKey);

		if (lResult == ERROR_SUCCESS)
			return TRUE;

		lResult = RegOpenKeyEx(hKeyRoot, lpSubKey, 0, KEY_READ, &hKey);

		if (lResult != ERROR_SUCCESS) {
			if (lResult == ERROR_FILE_NOT_FOUND)
				return true;
			PrintError(L"Error opening key.\n");
			return false;
		}

		// Check for an ending slash and add one if it is missing.

		lpEnd = lpSubKey + lstrlen(lpSubKey);

		if (*(lpEnd - 1) != TEXT('\\')) {
			*lpEnd = TEXT('\\');
			lpEnd++;
			*lpEnd = TEXT('\0');
		}

		// Enumerate the keys

		dwSize = MAX_PATH;
		lResult = RegEnumKeyEx(hKey, 0, szName, &dwSize, nullptr, nullptr, nullptr, &ftWrite);

		if (lResult == ERROR_SUCCESS) {
			do {
				*lpEnd = TEXT('\0');
				StringCchCat(lpSubKey, MAX_PATH * 2, szName);

				if (!RegDelnodeRecurse(hKeyRoot, lpSubKey)) {
					break;
				}

				dwSize = MAX_PATH;

				lResult = RegEnumKeyEx(hKey, 0, szName, &dwSize, nullptr, nullptr, nullptr, &ftWrite);
			}
			while (lResult == ERROR_SUCCESS);
		}

		lpEnd--;
		*lpEnd = TEXT('\0');

		RegCloseKey(hKey);

		// Try again to delete the key.

		lResult = RegDeleteKey(hKeyRoot, lpSubKey);

		if (lResult == ERROR_SUCCESS)
			return TRUE;

		return FALSE;
	}

	inline bool DeleteRegistryKey(HKEY hKeyRoot, LPCTSTR lpSubKey) {
		TCHAR szDelKey[MAX_PATH * 2];

		StringCchCopy(szDelKey, MAX_PATH * 2, lpSubKey);
		return RegDelnodeRecurse(hKeyRoot, szDelKey);
	}

	inline std::wstring GetSteamInstallPath() {
		const std::wstring regSubKey = L"SOFTWARE\\WOW6432Node\\Valve\\Steam\\";
		const std::wstring regValue(L"InstallPath");
		std::wstring valueFromRegistry = L"Not Found";
		try {
			valueFromRegistry = GetStringValueFromHKLM(regSubKey, regValue);
		}
		catch (std::exception& e) {
			std::cerr << e.what() << std::endl;
		}
		return valueFromRegistry;
	}

	inline void ClearRegistryVariable(HKEY hKey, const std::wstring& path, const std::wstring& variable) {
		HKEY regKey = nullptr;
		const auto keyResult = RegOpenKeyEx(hKey, path.c_str(), 0, KEY_WRITE, &regKey);

		if (keyResult != ERROR_SUCCESS)
			RegCloseKey(regKey);

		const auto entry = L"";
		const auto setKeyResult = RegSetValueEx(
			regKey, variable.c_str(), 0, REG_SZ, (LPCBYTE)entry, (lstrlen(entry) + 1) * sizeof(WCHAR)
		);
		if (setKeyResult != ERROR_SUCCESS)
			RegCloseKey(regKey);
	}

	inline bool RemoveRegistryFolder(HKEY hKey, const std::wstring& path) {
		return RegDeleteKey(hKey, path.c_str()) == ERROR_SUCCESS;
	}

	inline void KillProcess(const std::string& procName) {
		const std::string command = "taskkill /f /im " + procName;
		system(command.c_str()); //function is no thread safe :vibe:
	}
}
