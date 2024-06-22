#include "UserData-Cleaner.h"

int main() {
	SetConsoleTitle(L"UserData-Cleaner by UnlegitSenpaii");

	const std::wstring rootFolder = L"UserData-Cleaner";
	const std::wstring backupDir = L"\\SteamData_OLD_" + std::to_wstring(
		std::chrono::system_clock::now().time_since_epoch().count()
	);
	const std::wstring currentBackup = rootFolder + backupDir;
	const std::wstring profilePath = rootFolder + backupDir + L"\\AppData";
	const std::wstring installPath = rootFolder + backupDir + L"\\InstallDir";
	const std::wstring configFile = rootFolder + L"\\configuration.ini";

	//get full path for config file
	const fs::path fullConfigPath = absolute(fs::path(configFile));

	//check if our folder exists.
	if (!fs::exists(rootFolder)) {
		fs::create_directory(rootFolder);
	}

	Utils::Print(
		L"Welcome to UnlegitSenpaii's Steam User Data Remover\n\tfound on github.com/UnlegitSenpaii/steam-userdata-cleaner\n"
	);
	Utils::Print(
		L"Read before usage:\n\tBefore deletion, the files are copied to %s,\n\tso if errors occur a revert of the changes can be made.\n",
		currentBackup.c_str()
	);
	Utils::Print(
		L"The following directories are scanned:\n\t-Steam Installation Directory\n\t-Steam Registry Path\n\t-USERPROFILE\\appdata\\local\\steam\n"
	);
	Utils::Print(
		L"You can change the behaviour of the application by editing its config file in %s\n", configFile.c_str()
	);

	//admin is needed??
	if (!Config::FirstTimeSetup(fullConfigPath))
		Utils::PrintError(L"Failed to setup config file! \n");

	Sleep(1000);
	system("pause");//function is no thread safe :vibe:

	//admin is needed??
	Config::InitVariables();

	Utils::Trace(L"TraceLogs: %d - Backups: %d\n", Config::doTraceLogs, Config::doBackups);

	Utils::Trace(L"Ignored Accounts: \n");
	for (auto& ignoredAcc : Config::ignoredAccounts) {
		Utils::Trace(L"\t -> %s \n", ignoredAcc);
	}

	Utils::Print(L"Closing steam applications..\n");

	Utils::KillProcess("Steam.exe");
	Utils::KillProcess("steamwebhelper.exe");
	Utils::KillProcess("gameoverlayui.exe");
	Utils::KillProcess("SteamService.exe");

	Sleep(500);

	Utils::Print(L"Cleaning Registry..\n");
	Utils::ClearRegistryVariable(HKEY_CURRENT_USER, L"SOFTWARE\\Valve\\Steam", L"LastGameNameUsed");
	Utils::ClearRegistryVariable(HKEY_CURRENT_USER, L"SOFTWARE\\Valve\\Steam", L"AutoLoginUser");

	if (!Utils::DeleteRegistryKey(HKEY_CURRENT_USER, L"SOFTWARE\\Valve\\Steam\\Users"))
		Utils::PrintError(L"Failed to delete user list in registry!\n");

	if (!Utils::DeleteRegistryKey(HKEY_CURRENT_USER, L"SOFTWARE\\Valve\\Steam\\ActiveProcess"))
		Utils::PrintError(L"Failed to delete user list in registry!\n");

	const std::wstring steamInstallPath = Utils::GetSteamInstallPath();

	if (steamInstallPath == L"Not Found" || !fs::exists(steamInstallPath)) {
		Utils::FatalError(L"Failed to find steam install directory\n");
		return 1;
	}

	Utils::Print(L"Found Steam Directory: %s \n", steamInstallPath.c_str());

	std::wstring userProfilePath = Utils::GetEnviromentVariable(L"userprofile");

	if (userProfilePath.empty() || userProfilePath == L"Not Found") {
		Utils::FatalError(L"Failed to find user profile directory\n");
		return 1;
	}

	Utils::Print(L"Found User Profile Directory: %s \n", userProfilePath.c_str());

	userProfilePath += L"\\AppData\\Local\\Steam";

	Utils::Print(L"Populating deletion list\n");

	FileMgr::steamDeletionList.PopulateDeletionList(steamInstallPath, steamInstallPath);

	if(Config::deleteAppManifest)
		FileMgr::steamDeletionList.targetList.push_back(L"\\steamapps\\appmanifest_");

	FileMgr::profileDeletionList.PopulateDeletionList(userProfilePath, userProfilePath);

	Utils::Trace(L"SteamDeletionList:\n");
	for (auto& file : FileMgr::steamDeletionList.targetList)
		Utils::Trace(L"\t%s\n", file.c_str());

	Utils::Trace(L"ProfileDeletionList:\n");
	for (auto& file : FileMgr::profileDeletionList.targetList)
		Utils::Trace(L"\t%s\n", file.c_str());

	const auto filesToDelete = FileMgr::StuffToDelete(steamInstallPath, userProfilePath);

	if (filesToDelete <= 0)
		Utils::FatalError(L"There are no files that need to be deleted.\n");

	Utils::Print(L"Found %d items to delete.\n", filesToDelete);
	//todo: multi thread this and add a md5 check
	if (Config::doBackups) {
		Utils::Print(L"Backing up.. Do not close!\n");
		fs::create_directory(rootFolder);
		fs::create_directory(currentBackup);
		fs::create_directory(profilePath);
		fs::create_directory(installPath);

		FileMgr::BackupCurrentSteamData(steamInstallPath, installPath);
		FileMgr::BackupCurrentProfileData(userProfilePath, profilePath);
	}
	Utils::Print(L"Deleting.. Do not close!\n");
	FileMgr::DelteCurrentData(steamInstallPath, userProfilePath);

	Utils::Print(L"Successfully removed %d files / folders!\n", FileMgr::timesDeleted);

	system("pause");//function is no thread safe :vibe:
	return 0;
}