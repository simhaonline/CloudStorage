#include "pch.h"

#include "DataBaseFunctions.h"
#include "Constants.h"

using namespace std;

void authorization(streams::IOSocketStream<char>& clientStream, const db::CloudDataBase& db, string& login)
{
	string password;

	clientStream >> login;
	clientStream >> password;

	if (db.authorization(login, password))
	{
		clientStream << responses::okResponse;
	}
	else
	{
		clientStream << accountResponses::failAuthorization;
	}
}

void registration(streams::IOSocketStream<char>& clientStream, const db::CloudDataBase& db, string& login)
{
	string password;

	clientStream >> login;
	clientStream >> password;

	if (db.registration(login, password).size())
	{
		clientStream << accountResponses::failRegistration;
	}
	else
	{
		clientStream << accountResponses::successRegistration;
	}
}

void showAllFilesInDirectory(streams::IOSocketStream<char>& clientStream, const db::CloudDataBase& db, const string& login)
{
	string directory;
	string result;

	clientStream >> directory;

	vector<db::fileData> data = db.getFiles(login);

	for (const auto& i : data)
	{
		result += i;
	}

	if (data.empty())
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