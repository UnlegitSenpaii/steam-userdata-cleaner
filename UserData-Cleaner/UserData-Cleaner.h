#pragma once
#include "includes.h"
#include "utils.h"

struct DeletionList_t
{
	std::vector<std::wstring> targetList;
	std::vector<std::wstring> relevantDirs;
	std::uintmax_t stuffToDelete = 0;

	DeletionList_t(const std::vector<std::wstring>& initialList, const std::vector<std::wstring>& relevantDirectories) {
		targetList = initialList;
		relevantDirs = relevantDirectories;
	}

	//omg this is fucking black magic
	[[nodiscard]] bool Relevant(const std::wstring& dirname) const {
		return std::any_of(
			relevantDirs.begin(), relevantDirs.end(), [dirname](const std::wstring& str)
			{
				return dirname.find(str) != std::wstring::npos;
			}
		);
	}

	void PopulateDeletionList(const std::wstring& currentDir, const std::wstring& rootPath) {
		for (auto& curIteration : fs::directory_iterator(currentDir)) {
			if (!Relevant(curIteration.path()))
				continue;
			if (is_directory(curIteration)) {
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

namespace FileMgr
{
	inline std::vector<std::wstring> relevantSteamInstallFolders = {
		L"Steam\\userdata",
		L"Steam\\config",
		L"Steam\\dumps",
		L"Steam\\logs",
		L"Steam\\appcache"
	};

	inline std::vector<std::wstring> relevantProfileFolders = { L"Steam\\htmlcache" };

	inline DeletionList_t steamDeletionList = DeletionList_t(
		{
			L"\\dumps",
			L"\\logs",
			L"\\appcache",
			L"\\config\\config.vdf",
			L"\\config\\loginusers.vdf",
			L"\\config\\coplay_"
		}, relevantSteamInstallFolders
	);

	inline DeletionList_t profileDeletionList = DeletionList_t({ L"\\htmlcache" }, relevantProfileFolders);

	inline std::uintmax_t timesDeleted = 0;

	inline bool IsRelevantDir(const std::wstring& dirname) {
		const bool resultSteamInstall = std::any_of(
			relevantSteamInstallFolders.begin(), relevantSteamInstallFolders.end(), [dirname](const std::wstring& str)
			{
				return dirname.find(str) != std::wstring::npos;
			}
		);

		const bool resultSteamProfile = std::any_of(
			relevantProfileFolders.begin(), relevantProfileFolders.end(), [dirname](const std::wstring& str)
			{
				return dirname.find(str) != std::wstring::npos;
			}
		);

		return resultSteamInstall || resultSteamProfile;
	}

	inline void BackupCurrentSteamData(const std::wstring& steamInstallPath, const std::wstring& backupDir) {
		for (auto& curIteration : fs::directory_iterator(steamInstallPath)) {
			if (!IsRelevantDir(curIteration.path()))
				continue;
			if (is_directory(curIteration)) {
				std::wstring fullPath = curIteration.path();
				std::wstring subFolder = fullPath.substr(steamInstallPath.length());
				std::wstring destFolder = backupDir + subFolder;
				try {
					fs::copy(curIteration, destFolder, fs::copy_options::overwrite_existing);
				}
				catch (const std::exception& e) {
					printf("%s \n", e.what());
					Utils::FatalError(L"Failed File Copy\n");
				}
				Utils::Trace(L"Copied folder %s to %s\n", subFolder.c_str(), destFolder.c_str());
				BackupCurrentSteamData(fullPath, destFolder);
			}
		}
	}

	inline void BackupCurrentProfileData(const std::wstring& userProfilePath, const std::wstring& backupDir) {
		for (auto& curIteration : fs::directory_iterator(userProfilePath)) {
			if (!IsRelevantDir(curIteration.path()))
				continue;
			if (is_directory(curIteration)) {
				std::wstring fullPath = curIteration.path();
				std::wstring subFolder = fullPath.substr(userProfilePath.length());
				std::wstring destFolder = backupDir + subFolder;
				try {
					fs::copy(curIteration, destFolder, fs::copy_options::overwrite_existing);
				}
				catch (const std::exception& e) {
					printf("%s \n", e.what());
					Utils::FatalError(L"Failed File Copy\n");
				}
				Utils::Trace(L"Copied folder %s to %s\n", subFolder.c_str(), destFolder.c_str());
				BackupCurrentProfileData(fullPath, destFolder);
			}
		}
	}

	inline bool ListedForDeletion(const std::wstring& path) {
		if (!IsRelevantDir(path))
			return false;

		const auto& steamList = steamDeletionList.targetList;
		const auto& profileList = profileDeletionList.targetList;

		const bool resultSteamList = std::any_of(
			steamList.begin(), steamList.end(), [path](const std::wstring& str)
			{
				return path.find(str) != std::wstring::npos;
			}
		);

		const bool resultProfileList = std::any_of(
			profileList.begin(), profileList.end(), [path](const std::wstring& str)
			{
				return path.find(str) != std::wstring::npos;
			}
		);

		const bool isIgnoredAccount = std::any_of(
			Config::ignoredAccounts.begin(), Config::ignoredAccounts.end(), [path](const std::wstring& str)
			{
				return path.find(str) != std::wstring::npos;
			}
		);

		return (resultSteamList || resultProfileList) && !isIgnoredAccount;
	}

	inline void PopulateSteamDeletionAmount(const std::wstring& steamInstallPath) {
		for (auto& curIteration : fs::directory_iterator(steamInstallPath)) {
			std::wstring fullPath = curIteration.path();
			std::wstring subFolder = fullPath.substr(steamInstallPath.length());
			if (!IsRelevantDir(fullPath))
				continue;

			if (is_directory(curIteration)) {
				if (ListedForDeletion(fullPath)) {
					steamDeletionList.stuffToDelete++;
					PopulateSteamDeletionAmount(fullPath);
				}
			}
			else {
				if (ListedForDeletion(fullPath)) {
					steamDeletionList.stuffToDelete++;
				}
			}
		}
	}

	inline void PopulateProfileDeletionAmount(const std::wstring& profilePath) {
		for (auto& curIteration : fs::directory_iterator(profilePath)) {
			std::wstring fullPath = curIteration.path();
			std::wstring subFolder = fullPath.substr(profilePath.length());
			if (!IsRelevantDir(fullPath))
				continue;

			if (is_directory(curIteration)) {
				if (ListedForDeletion(fullPath)) {
					profileDeletionList.stuffToDelete++;
					PopulateProfileDeletionAmount(fullPath);
				}
			}
			else {
				if (ListedForDeletion(fullPath)) {
					profileDeletionList.stuffToDelete++;
				}
			}
		}
	}

	inline void PopulateDeletionAmount(const std::wstring& steamInstallPath, const std::wstring& profilePath) {
		PopulateSteamDeletionAmount(steamInstallPath);
		PopulateProfileDeletionAmount(profilePath);
	}

	inline std::uintmax_t StuffToDelete(const std::wstring& steamInstallPath, const std::wstring& profilePath) {
		PopulateDeletionAmount(steamInstallPath, profilePath);
		return steamDeletionList.stuffToDelete + profileDeletionList.stuffToDelete;
	}

	inline void DelteCurrentSteamData(const std::wstring& steamInstallPath) {
		for (auto& curIteration : fs::directory_iterator(steamInstallPath)) {
			std::wstring fullPath = curIteration.path();
			std::wstring subFolder = fullPath.substr(steamInstallPath.length());
			if (!IsRelevantDir(fullPath))
				continue;
			if (is_directory(curIteration)) {
				if (ListedForDeletion(fullPath)) {
					const std::uintmax_t n = fs::remove_all(fullPath);
					Utils::Trace(L"Deleted folder %s with %d sub files/folders\n", subFolder.c_str(), n);
					timesDeleted += n;
				}
				else {
					DelteCurrentSteamData(fullPath);
				}
			}
			else {
				if (ListedForDeletion(fullPath)) {
					const std::uintmax_t n = fs::remove_all(fullPath);
					Utils::Trace(L"Deleted file %s\n", subFolder.c_str());
					timesDeleted += n;
				}
			}
		}
	}

	inline void DelteCurrentProfileData(const std::wstring& profilePath) {
		for (auto& curIteration : fs::directory_iterator(profilePath)) {
			std::wstring fullPath = curIteration.path();
			std::wstring subFolder = fullPath.substr(profilePath.length());
			if (!IsRelevantDir(fullPath))
				continue;
			if (is_directory(curIteration)) {
				if (ListedForDeletion(fullPath)) {
					const std::uintmax_t n = fs::remove_all(fullPath);
					Utils::Trace(L"Deleted folder %s with %d sub files/folders\n", subFolder.c_str(), n);
					timesDeleted += n;
				}
				else {
					DelteCurrentSteamData(fullPath);
				}
			}
			else {
				if (ListedForDeletion(fullPath)) {
					const std::uintmax_t n = fs::remove(fullPath);
					Utils::Trace(L"Deleted file %s\n", subFolder.c_str());
					timesDeleted += n;
				}
			}
		}
	}

	inline void DelteCurrentData(const std::wstring& steamInstallPath, const std::wstring& profilePath) {
		DelteCurrentSteamData(steamInstallPath);
		DelteCurrentProfileData(profilePath);
	}
}
