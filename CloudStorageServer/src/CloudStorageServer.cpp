#include "pch.h"

#include "CloudStorageServer.h"
#include "IOSocketStream.h"
#include "Log.h"
#include "Constants.h"

#include "FilesNetwork.h"

#pragma comment (lib, "BaseTCPServer.lib")
#pragma comment (lib, "SocketStreams.lib")
#pragma comment (lib, "Log.lib")

using namespace std;

void showAllFilesInDirectory(streams::IOSocketStream<char>& clientStream, const filesystem::path& currentPath);

void uploadFile(streams::IOSocketStream<char>& clientStream, const filesystem::path& currentPath);

void findDirectory(filesystem::path& currentPath, const string& directory);

namespace web
{
	void CloudStorageServer::clientConnection(SOCKET clientSocket, sockaddr addr)
	{
		streams::IOSocketStream<char> clientStream(new buffers::IOSocketBuffer<char>(new FilesNetwork(clientSocket)));

		while (true)
		{
			try
			{
				filesystem::path currentPath(filesystem::current_path());
				string request;
				string login;
				string directory;
				string end;

				clientStream >> request;
				clientStream >> login;
				clientStream >> directory;

				currentPath.append(usersDirectory);
				currentPath.append(login);

				if (!filesystem::exists(currentPath))
				{
					filesystem::create_directories(currentPath);
				}

				if (directory != "Home")
				{
					findDirectory(currentPath, directory);
				}

				if (request == filesRequests::showAllFilesInDirectory)
				{
					showAllFilesInDirectory(clientStream, currentPath);
				}
				else if (request == filesRequests::uploadFile)
				{
					uploadFile(clientStream, currentPath);
				}
				else if (request == filesRequests::downloadFiles)
				{

				}
			}
			catch (const WebException&)
			{
				return;
			}
		}
	}

	CloudStorageServer::CloudStorageServer() : BaseTCPServer(cloudStorageServerPort, false)
	{

	}
}

void showAllFilesInDirectory(streams::IOSocketStream<char>& clientStream, const filesystem::path& currentPath)
{
	string result;
	filesystem::directory_iterator it(currentPath);
	
	for (const auto& i : it)
	{
		result += i.path().filename().string() + ":";
	}

	try
	{
		if (result.empty())
		{
			clientStream << responses::failResponse;
			clientStream << filesResponses::emptyDirectory;
		}
		else
		{
			clientStream << responses::okResponse;
			clientStream << result;
		}
	}
	catch (const web::WebException& e)
	{
		cout << e.what() << endl;
	}

	
}

void uploadFile(streams::IOSocketStream<char>& clientStream, const filesystem::path& currentPath)
{
	string fileName;
	uintmax_t offset;
	string data;
	bool needResponse;

	clientStream >> fileName;
	clientStream >> offset;
	clientStream >> data;
	clientStream >> needResponse;

	ofstream out(filesystem::path(currentPath).append(fileName), ios::binary | ios::app);

	out.seekp(offset);

	out.write(data.data(), data.size());

	out.close();

	if (needResponse)
	{
		uintmax_t uploadSize;
		uintmax_t downloadSize = filesystem::file_size(filesystem::path(currentPath).append(fileName));

		clientStream >> uploadSize;

		if (uploadSize == downloadSize)
		{
			clientStream << responses::okResponse;
			clientStream << filesResponses::successUploadFile;
		}
		else
		{
			clientStream << responses::failResponse;
			clientStream << filesResponses::failUploadFile;
		}
	}
}

void findDirectory(filesystem::path& currentPath, const string& directory)
{
	filesystem::recursive_directory_iterator it(currentPath);

	for (const auto& i : it)
	{
		if (filesystem::is_directory(i) && i.path().filename() == directory)
		{
			currentPath = i;
			return;
		}
	}
}