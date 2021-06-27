#pragma once
#include "utils.h"
#include "includes.h"

struct DeletionList_t {
	std::vector<std::wstring> targetList;
	std::vector<std::wstring> relevantDirs;
	std::uintmax_t stuffToDelete = 0;

	DeletionList_t(std::vector<std::wstring> initialList, std::vector<std::wstring> relevantDirectories){
		targetList = initialList;
		relevantDirs = relevantDirectories;
	}

	bool Relevant(std::wstring dirname) {
		for (auto& dir : relevantDirs) {
			if (dirname.find(dir) != std::wstring::npos)
				return true;
		}
		return false;
	}

	void PopulateDeletionList(std::wstring currentDir, std::wstring rootPath) {
		for (auto& curIteration : fs::directory_iterator(currentDir)) {
			if (!Relevant(curIteration.path()))
				continue;
			if (fs::is_directory(curIteration)) {
				std::wstring dirname = curIteration.path();
				if (dirname.find(L"Steam\\userdata\\") != std::wstring::npos) {
					std::wstring subFolder = dirname.substr(rootPath.length());
					targetList.push_back(subFolder);
					continue;
				}
				PopulateDeletionList(dirname, rootPath);
			}
		}
	}
};


namespace FileMgr {

	std::vector<std::wstring> relevantSteamInstallFolders = { L"Steam\\userdata", L"Steam\\config", L"Steam\\dumps", L"Steam\\logs", L"Steam\\appcache" };
	std::vector<std::wstring> relevantProfileFolders = { L"Steam\\htmlcache"};


	bool IsRelevantDir(std::wstring dirname) {

		for (auto& dir : relevantSteamInstallFolders) {
			if (dirname.find(dir) != std::wstring::npos)
				return true;
		}

		for (auto& dir : relevantProfileFolders) {
			if (dirname.find(dir) != std::wstring::npos)
				return true;
		}
		return false;
	}

	void BackupCurrentSteamData(std::wstring steamInstallPath, std::wstring backupDir) {
		for (auto& curIteration : fs::directory_iterator(steamInstallPath)) {
			if (!IsRelevantDir(curIteration.path()))
				continue;
			if (fs::is_directory(curIteration)) {
				std::wstring fullPath = curIteration.path();
				std::wstring subFolder = fullPath.substr(steamInstallPath.length());
				std::wstring destFolder = backupDir + subFolder;
				fs::copy(curIteration, destFolder);
				Utils::Trace(L"Copied folder %s to %s\n", subFolder.c_str(), destFolder.c_str());
				BackupCurrentSteamData(fullPath, destFolder);
			}
		}
	}

	void BackupCurrentProfileData(std::wstring userProfilePath, std::wstring backupDir) {
		for (auto& curIteration : fs::directory_iterator(userProfilePath)) {
			if (!IsRelevantDir(curIteration.path()))
				continue;
			if (fs::is_directory(curIteration)) {
				std::wstring fullPath = curIteration.path();
				std::wstring subFolder = fullPath.substr(userProfilePath.length());
				std::wstring destFolder = backupDir + subFolder;
				fs::copy(curIteration, destFolder);
				Utils::Trace(L"Copied folder %s to %s\n", subFolder.c_str(), destFolder.c_str());
				BackupCurrentProfileData(fullPath, destFolder);
			}
		}
	}

	DeletionList_t steamDeletionList = DeletionList_t({ L"\\dumps" , L"\\logs",L"\\appcache", L"\\config\\config.vdf", L"\\config\\loginusers.vdf", L"\\config\\coplay_" }, relevantSteamInstallFolders);
	DeletionList_t profileDeletionList = DeletionList_t({ L"\\htmlcache" }, relevantProfileFolders);

	bool ListedForDeletion(std::wstring path) {
		if (!IsRelevantDir(path))
			return false;

		for (auto& entry : steamDeletionList.targetList) {
			if (path.find(entry) != std::wstring::npos)
				return true;
		}

		for (auto& entry : profileDeletionList.targetList) {
			if (path.find(entry) != std::wstring::npos)
				return true;
		}

		return false;
	}
	void PopulateSteamDeletionAmount(std::wstring steamInstallPath) {
		for (auto& curIteration : fs::directory_iterator(steamInstallPath)) {
			std::wstring fullPath = curIteration.path();
			std::wstring subFolder = fullPath.substr(steamInstallPath.length());
			if (!IsRelevantDir(fullPath))
				continue;

			if (fs::is_directory(curIteration)) {
				if (ListedForDeletion(fullPath)) {
					steamDeletionList.stuffToDelete++;
					PopulateSteamDeletionAmount(fullPath);
				}
			}
			else if (fs::is_regular_file(curIteration)) {
				if (ListedForDeletion(fullPath)) {
					steamDeletionList.stuffToDelete++;
				}
			}
		}
	}

	void PopulateProfileDeletionAmount(std::wstring profilePath) {
		for (auto& curIteration : fs::directory_iterator(profilePath)) {
			std::wstring fullPath = curIteration.path();
			std::wstring subFolder = fullPath.substr(profilePath.length());
			if (!IsRelevantDir(fullPath))
				continue;

			if (fs::is_directory(curIteration)) {
				if (ListedForDeletion(fullPath)) {
					profileDeletionList.stuffToDelete++;
					PopulateProfileDeletionAmount(fullPath);
				}
			}
			else if (fs::is_regular_file(curIteration)) {
				if (ListedForDeletion(fullPath)) {
					profileDeletionList.stuffToDelete++;
				}
			}
		}
	}

	void PopulateDeletionAmount(std::wstring steamInstallPath, std::wstring profilePath) {

		PopulateSteamDeletionAmount(steamInstallPath);
		PopulateProfileDeletionAmount(profilePath);

	}

	int StuffToDelete(std::wstring steamInstallPath, std::wstring profilePath) {
		PopulateDeletionAmount(steamInstallPath, profilePath);
		return steamDeletionList.stuffToDelete + profileDeletionList.stuffToDelete;
	}

	std::uintmax_t timesDeleted = 0;

	void DelteCurrentSteamData(std::wstring steamInstallPath) {
		for (auto& curIteration : fs::directory_iterator(steamInstallPath)) {
			std::wstring fullPath = curIteration.path();
			std::wstring subFolder = fullPath.substr(steamInstallPath.length());
			if (!IsRelevantDir(fullPath))
				continue;
			if (fs::is_directory(curIteration)) {
				if (ListedForDeletion(fullPath)) {
					std::uintmax_t n = fs::remove_all(fullPath);
					Utils::Trace(L"Deleted folder %s with %d subfiles/subfolders\n", subFolder.c_str(), n);
					timesDeleted += n;
				}
				else
					DelteCurrentSteamData(fullPath);
			}
			else if (fs::is_regular_file(curIteration)) {
				if (ListedForDeletion(fullPath)) {
					int n = fs::remove_all(fullPath);
					Utils::Trace(L"Deleted file %s\n", subFolder.c_str());
					timesDeleted += n;
				}
			}
		}
	}

	void DelteCurrentProfileData(std::wstring profilePath) {
		for (auto& curIteration : fs::directory_iterator(profilePath)) {
			std::wstring fullPath = curIteration.path();
			std::wstring subFolder = fullPath.substr(profilePath.length());
			if (!IsRelevantDir(fullPath))
				continue;
			if (fs::is_directory(curIteration)) {
				if (ListedForDeletion(fullPath)) {
					std::uintmax_t n = fs::remove_all(fullPath);
					Utils::Trace(L"Deleted folder %s with %d subfiles/subfolders\n", subFolder.c_str(), n);
					timesDeleted += n;
				}
				else
					DelteCurrentSteamData(fullPath);
			}
			else if (fs::is_regular_file(curIteration)) {
				if (ListedForDeletion(fullPath)) {
					int n = fs::remove_all(fullPath);
					Utils::Trace(L"Deleted file %s\n", subFolder.c_str());
					timesDeleted += n;
				}
			}
		}
	}

	void DelteCurrentData(std::wstring steamInstallPath, std::wstring profilePath) {
		DelteCurrentSteamData(steamInstallPath);
		DelteCurrentProfileData(profilePath);
	}

}
