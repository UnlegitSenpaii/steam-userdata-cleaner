#pragma once
#include "includes.h"
namespace Utils {
	bool Print(const wchar_t* fmt, ...) {
		HANDLE conOutHandle = GetStdHandle(STD_OUTPUT_HANDLE);
		if (!conOutHandle)
			return false;
		wchar_t buf[4096];
		va_list va;

		va_start(va, fmt);
		_vsnwprintf_s(buf, 4096, fmt, va);
		va_end(va);

		return !!WriteConsole(conOutHandle, buf, static_cast<DWORD>(wcslen(buf)), nullptr, nullptr);
	}

	bool Trace(const wchar_t* fmt, ...) {
#ifdef TRACE_LOGS
		WORD m_currentConsoleAttr{};
		CONSOLE_SCREEN_BUFFER_INFO csbi;
		HANDLE conOutHandle = GetStdHandle(STD_OUTPUT_HANDLE);

		if (!conOutHandle)
			return false;

		if (GetConsoleScreenBufferInfo(conOutHandle, &csbi))
			m_currentConsoleAttr = csbi.wAttributes;

		SetConsoleTextAttribute(conOutHandle, 8);//LIGHT GREY
		Print(L"[TRACE] ");

		wchar_t buf[4096];
		va_list va;

		va_start(va, fmt);
		_vsnwprintf_s(buf, 4096, fmt, va);
		va_end(va);
		bool returnvar = !!WriteConsole(conOutHandle, buf, static_cast<DWORD>(wcslen(buf)), nullptr, nullptr);
		SetConsoleTextAttribute(conOutHandle, m_currentConsoleAttr);
		return returnvar;
#else
		return true;
#endif // TRACE
	}

	bool PrintError(const wchar_t* fmt, ...) {
		WORD m_currentConsoleAttr{};
		CONSOLE_SCREEN_BUFFER_INFO csbi;
		HANDLE conOutHandle = GetStdHandle(STD_OUTPUT_HANDLE);

		if (!conOutHandle)
			return false;

		if (GetConsoleScreenBufferInfo(conOutHandle, &csbi))
			m_currentConsoleAttr = csbi.wAttributes;

		SetConsoleTextAttribute(conOutHandle, 12);//INTENSE RED
		Print(L"[ERROR] ");

		SetConsoleTextAttribute(conOutHandle, m_currentConsoleAttr);
		wchar_t buf[4096];
		va_list va;

		va_start(va, fmt);
		_vsnwprintf_s(buf, 4096, fmt, va);
		va_end(va);

		return !!WriteConsole(conOutHandle, buf, static_cast<DWORD>(wcslen(buf)), nullptr, nullptr);
	}

	bool PrintWarning(const wchar_t* fmt, ...) {
		WORD m_currentConsoleAttr{};
		CONSOLE_SCREEN_BUFFER_INFO csbi;
		HANDLE conOutHandle = GetStdHandle(STD_OUTPUT_HANDLE);

		if (!conOutHandle)
			return false;

		if (GetConsoleScreenBufferInfo(conOutHandle, &csbi))
			m_currentConsoleAttr = csbi.wAttributes;

		SetConsoleTextAttribute(conOutHandle, 14);//INTENSE YELLOW
		Print(L"[WARNING] ");

		SetConsoleTextAttribute(conOutHandle, m_currentConsoleAttr);
		wchar_t buf[4096];
		va_list va;

		va_start(va, fmt);
		_vsnwprintf_s(buf, 4096, fmt, va);
		va_end(va);

		return !!WriteConsole(conOutHandle, buf, static_cast<DWORD>(wcslen(buf)), nullptr, nullptr);
	}

	void FatalError(const wchar_t* fmt, ...) {
		WORD m_currentConsoleAttr{};
		CONSOLE_SCREEN_BUFFER_INFO csbi;
		HANDLE conOutHandle = GetStdHandle(STD_OUTPUT_HANDLE);

		if (!conOutHandle)
			return;

		if (GetConsoleScreenBufferInfo(conOutHandle, &csbi))
			m_currentConsoleAttr = csbi.wAttributes;

		SetConsoleTextAttribute(conOutHandle, 12);//INTENSE RED
		Print(L"[FATALERROR] ");
		SetConsoleTextAttribute(conOutHandle, m_currentConsoleAttr);
		wchar_t buf[4096];
		va_list va;

		va_start(va, fmt);
		_vsnwprintf_s(buf, 4096, fmt, va);
		va_end(va);
		WriteConsole(conOutHandle, buf, static_cast<DWORD>(wcslen(buf)), nullptr, nullptr);
		system("pause");
		exit(1);
	}

	std::wstring GetEnviromentVariable(std::wstring var) {
		//rip getenv :(
		//https://docs.microsoft.com/de-de/cpp/c-runtime-library/reference/dupenv-s-wdupenv-s
		wchar_t* pValue;
		size_t len;
		errno_t err = _wdupenv_s(&pValue, &len, var.c_str());
		if (err)
			return L"Not Found";
		return pValue;
	}

	std::wstring GetStringValueFromHKLM(const std::wstring& regSubKey, const std::wstring& regValue)
	{
		//modified version of https://stackoverflow.com/a/50821858
		size_t bufferSize = 4096;
		std::wstring valueBuf;
		valueBuf.resize(bufferSize);
		auto cbData = static_cast<DWORD>(bufferSize * sizeof(wchar_t));
		auto rc = RegGetValue(HKEY_LOCAL_MACHINE, regSubKey.c_str(), regValue.c_str(), RRF_RT_REG_SZ, nullptr, static_cast<void*>(const_cast<wchar_t*>(valueBuf.data())), &cbData);
		while (rc == ERROR_MORE_DATA)
		{
			cbData /= sizeof(wchar_t);
			if (cbData > static_cast<DWORD>(bufferSize))
			{
				bufferSize = static_cast<size_t>(cbData);
			}
			else
			{
				bufferSize *= 2;
				cbData = static_cast<DWORD>(bufferSize * sizeof(wchar_t));
			}
			valueBuf.resize(bufferSize);
			rc = RegGetValue(HKEY_LOCAL_MACHINE, regSubKey.c_str(), regValue.c_str(), RRF_RT_REG_SZ, nullptr, static_cast<void*>(const_cast<wchar_t*>(valueBuf.data())), &cbData);
		}

		if (rc != NO_ERROR)
			throw std::runtime_error("Failed to read from registry, error code: " + std::to_string(rc));

		cbData /= sizeof(wchar_t);
		valueBuf.resize(static_cast<size_t>(cbData - 1));
		return valueBuf;
	}
	bool DeleteRegistryKey(HKEY hKeyRoot, LPCWSTR lpSubKey)
	{
		//https://docs.microsoft.com/en-us/windows/win32/sysinfo/deleting-a-key-with-subkeys
		LPCWSTR lpEnd;
		LONG lResult;
		DWORD dwSize;
		TCHAR szName[MAX_PATH];
		HKEY hKey;
		FILETIME ftWrite;

		lResult = RegDeleteKey(hKeyRoot, lpSubKey);

		if (lResult == ERROR_SUCCESS)
			return true;

		lResult = RegOpenKeyEx(hKeyRoot, lpSubKey, 0, KEY_READ, &hKey);

		if (lResult != ERROR_SUCCESS)
		{
			if (lResult == ERROR_FILE_NOT_FOUND)
				return true;
			else {
				Utils::PrintError(L"Error opening key.\n");
				return false;
			}
		}

		lpEnd = lpSubKey + lstrlen(lpSubKey);

		if (*(lpEnd - 1) != TEXT('\\'))
		{
			lpEnd = TEXT("\\");
			lpEnd++;
			lpEnd = TEXT("\0");
		}

		dwSize = MAX_PATH;
		lResult = RegEnumKeyEx(hKey, 0, szName, &dwSize, NULL,
			NULL, NULL, &ftWrite);

		if (lResult == ERROR_SUCCESS)
		{
			do {

				lpEnd = TEXT("\0");

				if (!DeleteRegistryKey(hKeyRoot, lpSubKey)) {
					break;
				}

				dwSize = MAX_PATH;

				lResult = RegEnumKeyEx(hKey, 0, szName, &dwSize, NULL,
					NULL, NULL, &ftWrite);

			} while (lResult == ERROR_SUCCESS);
		}

		lpEnd--;
		lpEnd = TEXT("\0");

		RegCloseKey(hKey);

		lResult = RegDeleteKey(hKeyRoot, lpSubKey);

		return (lResult == ERROR_SUCCESS);
	}
	std::wstring GetSteamInstallPath() {
		std::wstring regSubKey = L"SOFTWARE\\WOW6432Node\\Valve\\Steam\\";
		std::wstring regValue(L"InstallPath");
		std::wstring valueFromRegistry = L"Not Found";
		try
		{
			valueFromRegistry = GetStringValueFromHKLM(regSubKey, regValue);
		}
		catch (std::exception& e)
		{
			std::cerr << e.what() << std::endl;
		}
		return valueFromRegistry;
	}

	void ClearRegistryVariable(HKEY hKey, std::wstring path, std::wstring variable) {
		HKEY regKey = nullptr;
		auto keyResult = RegOpenKeyEx(hKey, path.c_str(), 0, KEY_WRITE, &regKey);

		if (keyResult != ERROR_SUCCESS)
			RegCloseKey(regKey);

		auto entry = L"";
		auto setkeyResult = RegSetValueEx(regKey, variable.c_str(), 0, REG_SZ, (LPCBYTE)entry, (lstrlen(entry) + 1) * sizeof(WCHAR));
		if (setkeyResult != ERROR_SUCCESS)
			RegCloseKey(regKey);

	}

	bool RemoveRegistryFolder(HKEY hKey, std::wstring path) {
		auto setkeyResult = RegDeleteKey(hKey, path.c_str());
		return setkeyResult == ERROR_SUCCESS;
	}

	void KillProcess(std::string procName) {
		std::string command = "taskkill /f /im " + procName;
		system(command.c_str());
	}
}