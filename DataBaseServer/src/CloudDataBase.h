#pragma once

#include <vector>

#include "DataBaseOperations.h"
#include "fileData.h"

namespace db
{
	class CloudDataBase
	{
	private:
		DataBaseOperations db;

	private:
		int getId(const std::string& login) const;

		std::string getLogin(int id) const;

		intmax_t getPassword(const std::string& login) const;

		intmax_t getPassword(int id) const;

	private:
		static intmax_t customHash(const std::string& source);

	public:
		CloudDataBase();

		bool uploadFileData(const std::string& login, std::string&& fileName, std::string&& filePath, std::string&& fileExtension, intmax_t fileSize) const;

		bool removeFileData(const std::string& login, std::string&& fileName, std::string&& filePath) const;

		void addFolder(const std::string& login, std::string&& folderName, std::string&& filePath, std::string&& fileExtension) const;

		std::vector<fileData> getFiles(const std::string& login, const std::string& path) const;

		std::string registration(const std::string& login, const std::string& password) const;

		bool authorization(const std::string& login, const std::string& password) const;

		~CloudDataBase() = default;
	};
}