#include "pch.h"

#include "DataBaseNetwork.h"
#include "Constants.h"
#include "Log.h"

using namespace std;

namespace web
{
	void DataBaseNetwork::log(string&& message, any&& data) noexcept
	{
		Log::error(move(message));
	}

	DataBaseNetwork::DataBaseNetwork() :
		parent(dataBaseServerIp, dataBaseServerPort)
	{

	}

	void DataBaseNetwork::setDataBaseServerIp(string&& ip) noexcept
	{
		dataBaseServerIp = move(ip);
	}

	void DataBaseNetwork::setDataBaseServerPort(string&& port) noexcept
	{
		dataBaseServerPort = move(port);
	}
}