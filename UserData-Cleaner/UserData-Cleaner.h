#pragma once
#include "utils.h"
#include "includes.h"
namespace FileMgr {
	bool IsRelevantDir(std::wstring dirname) {
		if (dirname.find(L"Steam\\userdata") != std::wstring::npos)
			return true;
		else if (dirname.find(L"Steam\\config") != std::wstring::npos)
			return true;
		else if (dirname.find(L"Steam\\dumps") != std::wstring::npos)
			return true;
		else if (dirname.find(L"Steam\\logs") != std::wstring::npos)
			return true;
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

	namespace DelList {
		std::vector<std::wstring> targetList;
		std::uintmax_t stuffToDelete = 0;
		void PopulateDeletionList(std::wstring currentDir, std::wstring steamPath) {
			if (targetList.empty()) {
				targetList.push_back(L"\\dumps");
				targetList.push_back(L"\\logs");
				targetList.push_back(L"\\config\\config.vdf");
				targetList.push_back(L"\\config\\loginusers.vdf");
				targetList.push_back(L"\\config\\coplay_");
			}

			for (auto& curIteration : fs::directory_iterator(currentDir)) {
				if (!IsRelevantDir(curIteration.path()))
					continue;
				if (fs::is_directory(curIteration)) {
					std::wstring dirname = curIteration.path();
					if (dirname.find(L"Steam\\userdata\\") != std::wstring::npos) {
						std::wstring subFolder = dirname.substr(steamPath.length());
						targetList.push_back(subFolder);
						continue;
					}
					PopulateDeletionList(dirname, steamPath);
				}
			}
		}
	}
	bool ListedForDeletion(std::wstring path) {
		if (!IsRelevantDir(path))
			return false;

		for (auto& entry : DelList::targetList) {
			if (path.find(entry) != std::wstring::npos)
				return true;
		}
		return false;
	}

	void PopulateDeletionAmount(std::wstring steamInstallPath) {
		for (auto& curIteration : fs::directory_iterator(steamInstallPath)) {
			std::wstring fullPath = curIteration.path();
			std::wstring subFolder = fullPath.substr(steamInstallPath.length());
			if (!IsRelevantDir(fullPath))
				continue;
			if (fs::is_directory(curIteration)) {
				if (ListedForDeletion(fullPath)) {
					DelList::stuffToDelete++;
					PopulateDeletionAmount(fullPath);
				}
			}
			else if (fs::is_regular_file(curIteration)) {
				if (ListedForDeletion(fullPath)) {
					DelList::stuffToDelete++;
				}
			}
		}
	}

	bool HasStuffToDelete(std::wstring steamInstallPath) {
		PopulateDeletionAmount(steamInstallPath);
		return DelList::stuffToDelete > 0;
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
}
