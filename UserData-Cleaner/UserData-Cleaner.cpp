#include "UserData-Cleaner.h"



int main()
{
	SetConsoleTitle(L"UserData-Cleaner by UnlegitSenpaii");


	std::wstring rootFolder = L"UserData-Cleaner";
	std::wstring backupDir = L"\\SteamData_OLD_" + std::to_wstring(std::chrono::system_clock::now().time_since_epoch().count());
	std::wstring currentBackup = rootFolder + backupDir;
	std::wstring cbProfileDir = rootFolder + backupDir + L"\\AppData";
	std::wstring cbInstallDir = rootFolder + backupDir + L"\\InstallDir";

	Utils::Print(L"Welcome to UnlegitSenpaii's Steam User Data Remover\n\tfound on github.com/UnlegitSenpaii/steam-userdata-cleaner\n");
	Utils::Print(L"Read before usage:\n\tBefore deletion, the files are copied to %s,\n\tso if errors occur a revert of the changes can be made.\n", currentBackup.c_str());
	Utils::Print(L"The following directories are scanned:\n\t-Steam Installation Directory\n\t-Steam Registry Path\n\t-USERPROFILE\\appdata\\local\\steam\n");
	Sleep(1000);
	system("pause");

	Utils::Print(L"Closing steam applications..\n");

	Utils::KillProcess("Steam.exe");
	Utils::KillProcess("steamwebhelper.exe");
	Utils::KillProcess("gameoverlayui.exe");
	//needs administrator
	Utils::KillProcess("SteamService.exe");

	Sleep(250);


	Utils::Print(L"Cleaning Registry..\n");
	Utils::ClearRegistryVariable(HKEY_CURRENT_USER, L"SOFTWARE\\Valve\\Steam", L"LastGameNameUsed");
	Utils::ClearRegistryVariable(HKEY_CURRENT_USER, L"SOFTWARE\\Valve\\Steam", L"AutoLoginUser");
	if (!Utils::DeleteRegistryKey(HKEY_CURRENT_USER, L"SOFTWARE\\Valve\\Steam\\Users"))
		Utils::PrintError(L"Failed to delete userlist in regirstry!\n");

	std::wstring steamInstallPath = Utils::GetSteamInstallPath();

	if (steamInstallPath == L"Not Found" || !fs::exists(steamInstallPath)) {
		Utils::FatalError(L"Failed to find steam install directory\n");
		return 1;
	}

	Utils::Print(L"Found Steam Directory: %s \n", steamInstallPath.c_str());

	std::wstring userProfilePath = Utils::GetEnviromentVariable(L"userprofile");

	if (userProfilePath.empty() || userProfilePath == L"Not Found") {
		Utils::FatalError(L"Failed to find userprofile directory\n");
		return 1;
	}

	Utils::Print(L"Found User Profile Directory: %s \n", userProfilePath.c_str());

	userProfilePath += L"\\AppData\\Local\\Steam";

	Utils::Print(L"Populating deletion list\n");

	FileMgr::steamDeletionList.PopulateDeletionList(steamInstallPath, steamInstallPath);
	FileMgr::profileDeletionList.PopulateDeletionList(userProfilePath, userProfilePath);


	Utils::Trace(L"SteamDeletionList:\n");
	for(auto& file : FileMgr::steamDeletionList.targetList)
		Utils::Trace(L"\t%s\n", file.c_str());

	Utils::Trace(L"ProfileDeletionList:\n");
	for (auto& file : FileMgr::profileDeletionList.targetList)
		Utils::Trace(L"\t%s\n", file.c_str());

	int filesToDelete = FileMgr::StuffToDelete(steamInstallPath, userProfilePath);

	if (filesToDelete <= 0)
		Utils::FatalError(L"There are no files that need to be deleted.\n");

	Utils::Print(L"Found %d items to delete.\n", filesToDelete);

	fs::create_directory(rootFolder);
	fs::create_directory(currentBackup);
	fs::create_directory(cbProfileDir);
	fs::create_directory(cbInstallDir);

	FileMgr::BackupCurrentSteamData(steamInstallPath, cbInstallDir);
	FileMgr::BackupCurrentProfileData(userProfilePath, cbProfileDir);

	FileMgr::DelteCurrentData(steamInstallPath, userProfilePath);

	Utils::Print(L"Successfully removed %d files / folders!\n", FileMgr::timesDeleted);

	system("pause");
	return 0;
}