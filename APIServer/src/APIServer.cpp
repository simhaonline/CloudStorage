#include "pch.h"

#include "APIServer.h"
#include "Constants.h"
#include "IOSocketStream.h"
#include "HTTPParser.h"
#include "HTTPBuilder.h"
#include "UtilityFunction.h"

#include "HTTPNetwork.h"
#include "FilesNetwork.h"

#pragma comment (lib, "BaseTCPServer.lib")
#pragma comment (lib, "SocketStreams.lib")
#pragma comment (lib, "Log.lib")
#pragma comment (lib, "HTTP.lib")

using namespace std;

bool checkHTTP(const string& request);

void showAllFilesInDirectory(streams::IOSocketStream<char>& clientStream, streams::IOSocketStream<char>& filesStream, const string& login, const string& directory);

void uploadFile(streams::IOSocketStream<char>& clientStream, streams::IOSocketStream<char>& filesStream, const string& data, const map<string, string>& headers);

namespace web
{
	void APIServer::clientConnection(SOCKET clientSocket, sockaddr addr)
	{
		streams::IOSocketStream<char> clientStream(new buffers::IOSocketBuffer<char>(new HTTPNetwork(clientSocket)));
		streams::IOSocketStream<char> filesStream(new buffers::IOSocketBuffer<char>(new web::FilesNetwork()));
		HTTPNetwork network(clientSocket);
		string request;
		string response;

		while (true)
		{
			try
			{
				clientStream >> request;

				if (checkHTTP(request))
				{
					HTTPParser parser(request);
					const map<string, string>& headers = parser.getHeaders();
					auto it = headers.find(requestType::accountType);

					if (it != end(headers))
					{

					}
					else
					{
						it = headers.find(requestType::filesType);

						if (it != end(headers))
						{
							const string& request = it->second;

							if (request == filesRequests::showAllFilesInDirectory)
							{
								showAllFilesInDirectory(clientStream, filesStream, headers.at("Login"), headers.at("Directory"));
							}
							else if (request == filesRequests::uploadFile)
							{
								uploadFile(clientStream, filesStream, parser.getBody(), headers);
							}
							if (request == filesRequests::downloadFiles)
							{

							}
							else
							{

							}
						}
						else
						{
							continue;
						}
					}
				}
				else
				{
					continue;
				}
			}
			catch (const WebException& e)
			{

			}
		}
	}

	APIServer::APIServer() : BaseTCPServer(APIServerPort, false)
	{

	}
}

bool checkHTTP(const string& request)
{
	return request.find("HTTP") != string::npos ? true : false;
}

void showAllFilesInDirectory(streams::IOSocketStream<char>& clientStream, streams::IOSocketStream<char>& filesStream, const string& login, const string& directory)
{
	string serverResponse;
	bool error;
	string response;

	try
	{
		filesStream << filesRequests::showAllFilesInDirectory;
		filesStream << login;
		filesStream << directory;

		filesStream >> serverResponse;

		if (serverResponse == responses::okResponse)
		{
			filesStream >> serverResponse;
			error = false;
		}
		else
		{
			filesStream >> serverResponse;
			error = true;
		}

		response = web::HTTPBuilder().responseCode(web::ResponseCodes::ok).headers
		(
			"Content-Length", serverResponse.size(),
			"Content-Type", "text/plain; charset=utf-8",
			"Error", error
		).build(&serverResponse);

		utility::insertSizeHeaderToHTTPMessage(response);

		clientStream << response;
	}
	catch (const web::WebException& e)
	{
		cout << e.what() << endl;
	}
}

void uploadFile(streams::IOSocketStream<char>& clientStream, streams::IOSocketStream<char>& filesStream, const string& data, const map<string, string>& headers)
{
	const string& fileName = headers.at("File-Name");
	const string& login = headers.at("Login");
	const string& directory = headers.at("Directory");
	uintmax_t offset = stoull(headers.at("Range"));
	auto it = headers.find("Total-File-Size");
	bool needResponse = it != end(headers);

	filesStream << filesRequests::uploadFile;
	filesStream << login;
	filesStream << directory;

	filesStream << fileName;
	filesStream << offset;
	filesStream << data;
	filesStream << needResponse;

	if (needResponse)
	{
		uintmax_t uploadSize = stoull(it->second);
		string responseMessage;
		string isSuccess;

		filesStream << uploadSize;

		filesStream >> isSuccess;
		filesStream >> responseMessage;

		string response = web::HTTPBuilder().responseCode(web::ResponseCodes::ok).headers
		(
			"Error", isSuccess == responses::okResponse ? false : true,
			"Content-Length", responseMessage.size(),
			"Content-Type", "text/plain; charset=utf-8"
		).build(&responseMessage);

		utility::insertSizeHeaderToHTTPMessage(response);

		clientStream << response;
	}
}