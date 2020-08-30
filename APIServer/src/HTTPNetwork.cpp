#include "pch.h"

#include "HTTPNetwork.h"
#include "Log.h"
#include "HTTPParser.h"
#include "Constants.h"

using namespace std;

namespace web
{
	void HTTPNetwork::log(string&& message, any&& data) noexcept
	{
		Log::error(move(message));
	}

	int HTTPNetwork::sendData(const dataContainer& data)
	{
		try
		{
			return parent::sendBytes(data.data(), data.size());
		}
		catch (const WebException& e)
		{
			this->log(e.what());
			return -1;
		}
	}

	int HTTPNetwork::receiveData(dataContainer& data)
	{
		try
		{
			data.resize(HTTPPacketSize);

			int size = 0;
			int lastPacket = 0;
			int totalReceive = 0;

			do
			{
				lastPacket = recv(parent::clientSocket, data.data() + totalReceive, data.size() - totalReceive, NULL);

				if (lastPacket < 0)
				{
					throw WebException();
				}
				else if (lastPacket == 0)
				{
					return totalReceive;
				}

				totalReceive += lastPacket;

				if (totalReceive > 25 && !size)
				{
					HTTPParser parser(data);
					const unordered_map<string, string>& headers = parser.getHeaders();

					auto it = headers.find("Total-HTTP-Message-Size");

					if (it == end(headers))
					{
						return -1;
					}

					size = stoi(it->second);

					if (data.size() < size)
					{
						data.resize(size);
					}
				}
				else if (!size)
				{
					continue;
				}

			} while (totalReceive < size);

			return totalReceive;
		}
		catch (const WebException& e)
		{
			this->log(e.what());
			return -1;
		}
	}

	HTTPNetwork::HTTPNetwork(SOCKET clientSocket) :
		parent(clientSocket, parent::ReceiveMode::prohibitResize)
	{

	}

	int HTTPNetwork::sendData(const string_view& data)
	{
		try
		{
			return parent::sendBytes(data.data(), data.size());
		}
		catch (const WebException& e)
		{
			this->log(e.what());
			return -1;
		}
	}

	int HTTPNetwork::receiveData(string& data)
	{
		try
		{
			data.resize(HTTPPacketSize);

			int size = 0;
			int lastPacket = 0;
			int totalReceive = 0;

			do
			{
				lastPacket = recv(parent::clientSocket, data.data() + totalReceive, data.size() - totalReceive, NULL);

				if (lastPacket < 0)
				{
					throw WebException();
				}
				else if (lastPacket == 0)
				{
					return totalReceive;
				}

				totalReceive += lastPacket;

				if (totalReceive > 25 && !size)
				{
					HTTPParser parser(data);
					const unordered_map<string, string>& headers = parser.getHeaders();

					auto it = headers.find("Total-HTTP-Message-Size");

					if (it == end(headers))
					{
						return -1;
					}

					size = stoi(it->second);

					if (data.size() < size)
					{
						data.resize(size);
					}
				}
				else if (!size)
				{
					continue;
				}

			} while (totalReceive < size);

			return totalReceive;
		}
		catch (const WebException& e)
		{
			this->log(e.what());
			return -1;
		}
	}
}