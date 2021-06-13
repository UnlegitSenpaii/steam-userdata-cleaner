#include "UserData-Cleaner.h"

int main()
{
	SetConsoleTitle(L"UserData-Cleaner by UnlegitSenpaii");
	std::wstring backupDir = L"SteamData_OLD_" + std::to_wstring(std::chrono::system_clock::now().time_since_epoch().count());
	Utils::Print(L"Welcome to UnlegitSenpaii's Steam User Data Remover\n\tfound on github.com/UnlegitSenpaii/steam-userdata-cleaner\n");
	Utils::Print(L"Read before usage:\n\tThis program deletes files in the steam folder.\n\tBefore deletion, the files are copied to %s,\n\tso if errors occur a revert of the changes can be made.\n", backupDir.c_str());
	Sleep(1000);
	system("pause");

	Utils::Print(L"Closing steam applications..\n");

	Utils::KillProcess("Steam.exe");
	Utils::KillProcess("steamwebhelper.exe");
	Utils::KillProcess("gameoverlayui.exe");
	//needs administrator
	Utils::KillProcess("SteamService.exe");

	Sleep(250);

	std::wstring steamInstallPath = Utils::GetSteamInstallPath();

	if (steamInstallPath == L"Not Found" || !fs::exists(steamInstallPath)) {
		Utils::FatalError(L"Failed to find steam install directory\n");
		return 1;
	}

	Utils::Print(L"Found Steam Directory: %s \n", steamInstallPath.c_str());

	Utils::Print(L"Populating deletion list\n");
	FileMgr::DelList::PopulateDeletionList(steamInstallPath, steamInstallPath);

	if (!FileMgr::HasStuffToDelete(steamInstallPath))
		Utils::FatalError(L"There are no files that need to be deleted.\n");

	fs::create_directory(backupDir);

	FileMgr::BackupCurrentSteamData(steamInstallPath, backupDir);

	FileMgr::DelteCurrentSteamData(steamInstallPath);

	Utils::Print(L"Successfully removed %d files / folders!\n", FileMgr::timesDeleted);
	system("pause");
	return 0;
}