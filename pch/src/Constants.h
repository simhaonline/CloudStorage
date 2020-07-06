#pragma once

#include <string>

inline constexpr std::string_view APIServerIp = "31.207.173.47";
inline constexpr std::string_view APIServerPort = "8500";

inline constexpr std::string_view cloudStorageServerIp = "31.207.173.47";
inline constexpr std::string_view cloudStorageServerPort = "12500";

inline constexpr std::string_view usersDirectory = "Data";

inline constexpr short HTTPPacketSize = 4096;
inline constexpr int filePacketSize = 1 * 1024 * 1024;	//1 MB

inline constexpr std::string_view customHTTPHeaderSize = "Total-HTTP-Message-Size: ";

namespace requestType
{
	inline const std::string accountType = "Account request";
	inline const std::string filesType = "Files request";
}

namespace accountRequest
{
	inline constexpr std::string_view authorization = "Authorization";
	inline constexpr std::string_view registration = "Registration";
}

namespace filesRequests
{
	inline constexpr std::string_view uploadFile = "Upload file";
	inline constexpr std::string_view downloadFiles = "Download file";
	inline constexpr std::string_view showAllFilesInDirectory = "Show all files in directory";
}

namespace responses
{
	inline constexpr std::string_view okResponse = "OK";
	inline constexpr std::string_view failResponse = "FAIL";
	inline constexpr std::string_view unknownRequest = "Unknown request";
}

namespace filesResponses
{
	inline constexpr std::string_view emptyDirectory = "Directory is empty";
	inline constexpr std::string_view successUploadFile = "File successfully uploaded";
	inline constexpr std::string_view failUploadFile = "Fail while file upload";
}