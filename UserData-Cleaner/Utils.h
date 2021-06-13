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

	std::wstring GetStringValueFromHKLM(const std::wstring& regSubKey, const std::wstring& regValue)
	{
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

	void KillProcess(std::string procName) {
		std::string command = "taskkill /f /im " + procName;
		system(command.c_str());
	}
}