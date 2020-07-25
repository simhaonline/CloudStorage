#include "pch.h"

#include "NetworkFunctions.h"
#include "HTTPBuilder.h"
#include "HTTPParser.h"
#include "UIConstants.h"
#include "Constants.h"
#include "UtilityFunctions.h"
#include "Screens/ScreenFunctions.h"
#include "PopupWindows/PopupWindowFunctions.h"
#include "PopupWindows/UploadFilePopupWindow.h"
#include "PopupWindows/DownloadFilePopupWindow.h"
#include "HTTPNetwork.h"
#include "fileData.h"
#include "ClientTime.h"
#include "ErrorHandling.h"
#include "Log.h"

#include <CommCtrl.h>

using namespace std;

string cancelUploadFile(const string& fileName);

void asyncUploadFile(UI::MainWindow& ref, streams::IOSocketStream<char>& clientStream, const wstring& filePath, bool& isCancel);

void asyncDownloadFile(UI::MainWindow& ref, streams::IOSocketStream<char>& clientStream, const wstring& fileName, bool& isCancel);

string folderControlMessages(streams::IOSocketStream<char>& clientStream, const string_view& controlRequest, string&& data = "");

////////////////////////////////////////////////////////////////

void getFiles(UI::MainWindow& ref, streams::IOSocketStream<char>& clientStream, vector<db::fileDataRepresentation>& fileNames, bool showError)
{
	utility::ClientTime& instance = utility::ClientTime::get();
	string request = web::HTTPBuilder().postRequest().headers
	(
		requestType::filesType, filesRequests::showAllFilesInFolder
	).build();
	string response;
	fileNames.clear();

	utility::web::insertSizeHeaderToHTTPMessage(request);

	try
	{
		clientStream << request;
	}
	catch (const web::WebException&)
	{
		UI::serverRequestError(ref);
		return;
	}

	try
	{
		clientStream >> response;
	}
	catch (const web::WebException&)
	{
		UI::serverResponseError(ref);
		return;
	}

	web::HTTPParser parser(response);
	const string& error = parser.getHeaders().at("Error");
	const string& body = parser.getBody();

	if (error == "0")
	{
		array<string, 6> tem;	//each index equals member position in fileData struct
		vector<db::fileData> data;
		size_t curIndex = 0;

		for (const auto& i : body)
		{
			if (i == dataDelimiter[0])
			{
				fileNames.emplace_back
				(
					utility::conversion::to_wstring(tem[0]),
					utility::conversion::to_wstring(tem[1]),
					utility::conversion::to_wstring(tem[2]),
					instance.convertToLocal(tem[3]),
					instance.convertToLocal(tem[4]),
					stoul(tem[5])
				);

				curIndex = 0;

				for (auto& i : tem)
				{
					i.clear();
				}
			}
			else if (i == dataPartDelimiter[0])
			{
				curIndex++;
			}
			else
			{
				tem[curIndex] += i;
			}
		}
	}

	updateColumns(ref, fileNames);

	if (error == "1" && showError)
	{
		MessageBoxW
		(
			ref.getMainWindow(),
			utility::conversion::to_wstring(body).data(),
			L"������",
			MB_OK
		);
	}
}

void uploadFile(UI::MainWindow& ref, streams::IOSocketStream<char>& clientStream, const wstring& filePath, bool& isCancel)
{
	wstring message = L"�������� ����� " + wstring(begin(filePath) + filePath.rfind('\\') + 1, end(filePath));

	initUploadFilePopupWindow(ref, message);

	thread(asyncUploadFile, std::ref(ref), std::ref(clientStream), std::ref(filePath), std::ref(isCancel)).detach();
}

int downloadFile(UI::MainWindow& ref, streams::IOSocketStream<char>& clientStream, const vector<db::fileDataRepresentation>& fileNames, bool& isCancel, int searchId)
{
	int id = SendMessageW(ref.getList(), LVM_GETNEXTITEM, searchId, LVNI_SELECTED);

	if (id != -1)
	{
		initDownloadFilePopupWindow(ref, wstring(L"���������� ����� ") + fileNames[id].fileName);

		thread(asyncDownloadFile, std::ref(ref), std::ref(clientStream), std::ref(fileNames[id].fileName), std::ref(isCancel)).detach();
	}

	return id;
}

void removeFile(UI::MainWindow& ref, streams::IOSocketStream<char>& clientStream, const vector<db::fileDataRepresentation>& fileNames)
{
	int id = SendMessageW(ref.getList(), LVM_GETNEXTITEM, -1, LVNI_SELECTED);

	while (id != -1)
	{
		if (removeFileDialog(ref, fileNames[id].fileName))
		{
			removeFile(ref, clientStream, fileNames[id].fileName);
		}

		id = SendMessageW(ref.getList(), LVM_GETNEXTITEM, id, LVNI_SELECTED);
	}
}

void removeFile(UI::MainWindow& ref, streams::IOSocketStream<char>& clientStream, const std::wstring& fileName)
{
	string request = web::HTTPBuilder().postRequest().headers
	(
		requestType::filesType, filesRequests::removeFile,
		"File-Name", utility::conversion::to_string(fileName)
	).build();
	string response;

	utility::web::insertSizeHeaderToHTTPMessage(request);

	try
	{
		clientStream << request;
	}
	catch (const web::WebException&)
	{
		UI::serverRequestError(ref);
		return;
	}

	try
	{
		clientStream >> response;
	}
	catch (const web::WebException&)
	{
		UI::serverResponseError(ref);
		return;
	}

	web::HTTPParser parser(response);
	const map<string, string>& headers = parser.getHeaders();

	if (headers.at("Error") == "0")
	{
		//TODO: success message
	}
	else
	{
		MessageBoxW(ref.getMainWindow(), wstring(L"�� ������� ������� " + fileName).data(), L"������", MB_OK);
	}
}

void reconnect(UI::MainWindow& ref, streams::IOSocketStream<char>& clientStream)
{
	exitFromApplication(ref, clientStream);

	clientStream = streams::IOSocketStream<char>(new buffers::IOSocketBuffer<char>(new web::HTTPNetwork()));
}

void nextFolder(streams::IOSocketStream<char>& clientStream, const wstring& folderName)
{
	folderControlMessages(clientStream, controlRequests::nextFolder, utility::conversion::to_string(folderName));
}

void prevFolder(streams::IOSocketStream<char>& clientStream, filesystem::path& currentPath)
{
	if (currentPath != "Home")
	{
		const string path = currentPath.string();

		currentPath = string(begin(path), begin(path) + path.rfind('\\'));
	}

	folderControlMessages(clientStream, controlRequests::prevFolder);
}

void setPath(streams::IOSocketStream<char>& clientStream, string&& path)
{
	folderControlMessages(clientStream, controlRequests::setPath, move(path));
}

void createFolder(UI::MainWindow& ref, streams::IOSocketStream<char>& clientStream)
{
	wstring wFolderName;
	HWND enterFolderNameEdit = ref.getEnterFolderNameEdit();
	string request;
	string body;

	wFolderName.resize(GetWindowTextLengthW(enterFolderNameEdit));

	GetWindowTextW(enterFolderNameEdit, wFolderName.data(), wFolderName.size() + 1);

	body = "folder=" + utility::conversion::to_string(wFolderName);

	request = web::HTTPBuilder().postRequest().headers
	(
		requestType::filesType, filesRequests::createFolder,
		"Content-Length", body.size()
	).build(&body);

	utility::web::insertSizeHeaderToHTTPMessage(request);

	try
	{
		clientStream << request;
	}
	catch (const web::WebException&)
	{

	}

	SendMessageW(ref.getMainWindow(), UI::events::deletePopupWindowE, NULL, NULL);

	SendMessageW(ref.getMainWindow(), UI::events::getFilesE, NULL, NULL);
}

void setLogin(streams::IOSocketStream<char>& clientStream, const wstring& login, const wstring password)
{
	string body = "login=" + utility::conversion::to_string(login) + "&" + "password=" + utility::conversion::to_string(password);

	string request = web::HTTPBuilder().postRequest().headers
	(
		requestType::accountType, accountRequests::setLogin,
		"Content-Length", body.size()
	).build(&body);

	utility::web::insertSizeHeaderToHTTPMessage(request);

	try
	{
		clientStream << request;
	}
	catch (const web::WebException&)
	{

	}
}

void exitFromApplication(UI::MainWindow& ref, streams::IOSocketStream<char>& clientSream)
{
	string request = web::HTTPBuilder().postRequest().headers
	(
		requestType::exitType, networkRequests::exit
	).build();

	utility::web::insertSizeHeaderToHTTPMessage(request);

	try
	{
		clientSream << request;
	}
	catch (const web::WebException&)
	{

	}
}

tuple<wstring, wstring> authorization(UI::MainWindow& ref, streams::IOSocketStream<char>& clientStream)
{
	wstring wLogin;
	wstring wPassword;
	string response;
	HWND authorizationLoginEdit;
	HWND authorizationPasswordEdit;

	authorizationLoginEdit = ref.getAuthorizationLoginEdit();
	authorizationPasswordEdit = ref.getAuthorizationPasswordEdit();

	wLogin.resize(GetWindowTextLengthW(authorizationLoginEdit));
	wPassword.resize(GetWindowTextLengthW(authorizationPasswordEdit));

	GetWindowTextW(authorizationLoginEdit, wLogin.data(), wLogin.size() + 1);
	GetWindowTextW(authorizationPasswordEdit, wPassword.data(), wPassword.size() + 1);

	string body = "login=" + utility::conversion::to_string(wLogin) + "&" + "password=" + utility::conversion::to_string(wPassword);

	string request = web::HTTPBuilder().postRequest().headers
	(
		requestType::accountType, accountRequests::authorization,
		"Content-Length", body.size()
	).build(&body);

	utility::web::insertSizeHeaderToHTTPMessage(request);

	try
	{
		clientStream << request;
	}
	catch (const web::WebException&)
	{
		UI::serverRequestError(ref);
		return { wstring(), wstring() };
	}

	try
	{
		clientStream >> response;
	}
	catch (const web::WebException&)
	{
		UI::serverResponseError(ref);
		return { wstring(), wstring() };
	}

	web::HTTPParser parser(response);

	if (parser.getHeaders().at("Error") == "0")
	{
		return { wLogin, wPassword };
	}
	else
	{
		if (authorizationLoginEdit && authorizationPasswordEdit)
		{
			SetWindowTextW(authorizationLoginEdit, L"");
			SetWindowTextW(authorizationPasswordEdit, L"");
		}

		MessageBoxW(ref.getMainWindow(), utility::conversion::to_wstring(parser.getBody()).data(), L"������", MB_OK);

		return { wstring(), wstring() };
	}
}

tuple<wstring, wstring> registration(UI::MainWindow& ref, streams::IOSocketStream<char>& clientStream)
{
	wstring wLogin;
	wstring wPassword;
	wstring wRepeatPassword;
	string response;

	HWND registrationLoginEdit = ref.getRegistrationLoginEdit();
	HWND registrationPasswordEdit = ref.getRegistrationPasswordEdit();
	HWND registrationRepeatPasswordEdit = ref.getRegistrationRepeatPasswordEdit();

	wLogin.resize(GetWindowTextLengthW(registrationLoginEdit));
	wPassword.resize(GetWindowTextLengthW(registrationPasswordEdit));
	wRepeatPassword.resize(GetWindowTextLengthW(registrationRepeatPasswordEdit));

	GetWindowTextW(registrationLoginEdit, wLogin.data(), wLogin.size() + 1);
	GetWindowTextW(registrationPasswordEdit, wPassword.data(), wPassword.size() + 1);
	GetWindowTextW(registrationRepeatPasswordEdit, wRepeatPassword.data(), wRepeatPassword.size() + 1);

	if (wPassword != wRepeatPassword)
	{
		SetWindowTextW(registrationLoginEdit, L"");
		SetWindowTextW(registrationPasswordEdit, L"");
		SetWindowTextW(registrationRepeatPasswordEdit, L"");

		MessageBoxW(ref.getMainWindow(), L"������ �� ���������", L"������", MB_OK);

		return { wstring(), wstring() };
	}

	string body = "login=" + utility::conversion::to_string(wLogin) + "&" + "password=" + utility::conversion::to_string(wPassword);

	string request = web::HTTPBuilder().postRequest().headers
	(
		requestType::accountType, accountRequests::registration,
		"Content-Length", body.size()
	).build(&body);

	utility::web::insertSizeHeaderToHTTPMessage(request);

	try
	{
		clientStream << request;
	}
	catch (const web::WebException&)
	{
		UI::serverRequestError(ref);
		return { wstring(), wstring() };
	}

	try
	{
		clientStream >> response;
	}
	catch (const web::WebException&)
	{
		UI::serverResponseError(ref);
		return { wstring(), wstring() };
	}

	web::HTTPParser parser(response);

	if (parser.getHeaders().at("Error") == "0")
	{
		return { wLogin, wPassword };
	}
	else
	{
		SetWindowTextW(registrationLoginEdit, L"");
		SetWindowTextW(registrationPasswordEdit, L"");
		SetWindowTextW(registrationRepeatPasswordEdit, L"");

		MessageBoxW(ref.getMainWindow(), utility::conversion::to_wstring(parser.getBody()).data(), L"������", MB_OK);

		return { wstring(), wstring() };
	}
}

////////////////////////////////////////////////////////////////

string cancelUploadFile(const string& fileName)
{
	string result = web::HTTPBuilder().postRequest().headers
	(
		requestType::cancelType, networkRequests::cancelOperation,
		"Operation-Type", filesRequests::uploadFile,
		"File-Name", fileName
	).build();

	utility::web::insertSizeHeaderToHTTPMessage(result);

	return result;
}

void asyncUploadFile(UI::MainWindow& ref, streams::IOSocketStream<char>& clientStream, const wstring& filePath, bool& isCancel)
{
	const filesystem::path file(filePath);
	intmax_t fileSize = filesystem::file_size(file);

	string fileData;
	string lastPacket;
	string response;
	bool isLast;

	ifstream in(file, ios::binary);

	static_cast<UI::UploadFilePopupWindow*>(ref.getCurrentPopupWindow())->setProgressBarRange(0, fileSize);

	fileData.resize(filePacketSize);

	isCancel = false;

	do
	{
		const string* data;
		intmax_t offset = in.tellg();

		if (isCancel)
		{
			string cancelRequest = cancelUploadFile(file.filename().string());

			clientStream << cancelRequest;

			clientStream >> cancelRequest;

			SendMessageW(ref.getMainWindow(), UI::events::deletePopupWindowE, NULL, NULL);
			SendMessageW(ref.getMainWindow(), UI::events::multipleUploadE, NULL, NULL);
			in.close();
			return;
		}

		if (fileSize - offset >= filePacketSize)
		{
			in.read(fileData.data(), fileData.size());

			data = &fileData;
			isLast = false;
		}
		else
		{
			lastPacket.resize(fileSize - offset);

			in.read(lastPacket.data(), lastPacket.size());

			data = &lastPacket;
			isLast = true;
		}

		string message = web::HTTPBuilder().postRequest().headers
		(
			requestType::filesType, filesRequests::uploadFile,
			"File-Name", file.filename().string(),
			"Range", offset,
			"Content-Length", data->size(),
			isLast ? "Total-File-Size" : "Reserved", isLast ? fileSize : 0
		).build(data);

		utility::web::insertSizeHeaderToHTTPMessage(message);

		try
		{
			clientStream << message;
		}
		catch (const web::WebException&)
		{
			UI::serverRequestError(ref);
			SendMessageW(ref.getMainWindow(), UI::events::deletePopupWindowE, NULL, NULL);
			in.close();
			return;
		}

		if (!isCancel)
		{
			SendMessageW(ref.getPopupWindow(), UI::events::updateProgressBarE, static_cast<WPARAM>(offset), NULL);
		}

	} while (!isLast);

	in.close();

	try
	{
		clientStream >> response;
	}
	catch (const web::WebException&)
	{
		UI::serverResponseError(ref);
		SendMessageW(ref.getMainWindow(), UI::events::deletePopupWindowE, NULL, NULL);
		return;
	}

	web::HTTPParser parser(response);

	if (!isCancel)
	{
		SendMessageW(ref.getPopupWindow(), UI::events::updateProgressBarE, static_cast<WPARAM>(fileSize), NULL);
	}

	this_thread::sleep_for(1s);

	const string& error = parser.getHeaders().at("Error");

	if (error == "1")
	{
		int ok = MessageBoxW
		(
			ref.getPopupWindow(),
			utility::conversion::to_wstring(parser.getBody()).data(),
			L"������",
			MB_OK
		);

		if (ok == IDOK)
		{
			SendMessageW(ref.getMainWindow(), UI::events::deletePopupWindowE, NULL, NULL);
			SendMessageW(ref.getMainWindow(), UI::events::multipleUploadE, NULL, NULL);
		}
	}
	else
	{
		int ok = MessageBoxW
		(
			ref.getPopupWindow(),
			L"���� ��������",
			L"����������",
			MB_OK
		);

		if (ok == IDOK)
		{
			SendMessageW(ref.getMainWindow(), UI::events::getFilesE, NULL, NULL);
			SendMessageW(ref.getMainWindow(), UI::events::deletePopupWindowE, NULL, NULL);
			SendMessageW(ref.getMainWindow(), UI::events::multipleUploadE, NULL, NULL);
		}
	}
}

void asyncDownloadFile(UI::MainWindow& ref, streams::IOSocketStream<char>& clientStream, const wstring& fileName, bool& isCancel)
{
	intmax_t offset = 0;
	intmax_t totalFileSize;
	const string sFileName = utility::conversion::to_string(fileName);
	bool lastPacket;
	string response;

	ofstream out(ref.getDownloadFolder().append(sFileName), ios::binary);

	isCancel = false;

	static_cast<UI::DownloadFilePopupWindow*>(ref.getCurrentPopupWindow())->startAnimateProgressBar();

	while (true)
	{
		string request = web::HTTPBuilder().postRequest().headers
		(
			requestType::filesType, filesRequests::downloadFile,
			"File-Name", sFileName,
			"Range", offset
		).build();

		utility::web::insertSizeHeaderToHTTPMessage(request);

		if (isCancel)
		{
			SendMessageW(ref.getMainWindow(), UI::events::deletePopupWindowE, NULL, NULL);
			SendMessageW(ref.getMainWindow(), UI::events::multipleDownloadE, NULL, NULL);
			out.close();

			filesystem::remove(filesystem::path(ref.getDownloadFolder()).append(sFileName));

			return;
		}

		try
		{
			clientStream << request;
		}
		catch (const web::WebException&)
		{
			UI::serverRequestError(ref);
			out.close();
			return;
		}

		try
		{
			clientStream >> response;
		}
		catch (const web::WebException&)
		{
			UI::serverResponseError(ref);
			out.close();
			return;
		}

		web::HTTPParser parser(response);
		const map<string, string>& headers = parser.getHeaders();
		auto it = headers.find("Total-File-Size");
		lastPacket = it != end(headers);
		const string& data = parser.getBody();

		out.write(data.data(), data.size());

		offset = out.tellp();

		if (lastPacket)
		{
			totalFileSize = stoull(it->second);
			break;
		}
	}

	out.close();

	this_thread::sleep_for(0.5s);

	static_cast<UI::DownloadFilePopupWindow*>(ref.getCurrentPopupWindow())->stopAnimateProgressBar();

	this_thread::sleep_for(0.5s);

	if (filesystem::file_size(ref.getDownloadFolder().append(sFileName)) == totalFileSize)
	{
		wstring message = fileName + L"\r\n" + filesResponses::successDownloadFile.data();

		int ok = MessageBoxW(ref.getPopupWindow(), message.data(), L"�����", MB_OK);

		if (ok == IDOK)
		{
			SendMessageW(ref.getMainWindow(), UI::events::deletePopupWindowE, NULL, NULL);
			SendMessageW(ref.getMainWindow(), UI::events::multipleDownloadE, NULL, NULL);
		}
	}
	else
	{
		wstring message = fileName + L"\r\n" + filesResponses::failDownloadFile.data();

		int ok = MessageBoxW(ref.getPopupWindow(), message.data(), L"������", MB_OK);

		if (ok == IDOK)
		{
			SendMessageW(ref.getMainWindow(), UI::events::deletePopupWindowE, NULL, NULL);
			SendMessageW(ref.getMainWindow(), UI::events::multipleDownloadE, NULL, NULL);
		}
	}
}

string folderControlMessages(streams::IOSocketStream<char>& clientStream, const string_view& controlRequest, string&& data)
{
	string request;
	string response;
	string body = "folder=" + move(data);

	if (controlRequest == controlRequests::prevFolder)
	{
		request = web::HTTPBuilder().postRequest().headers
		(
			requestType::controlType, controlRequest
		).build();
	}
	else
	{
		request = web::HTTPBuilder().postRequest().headers
		(
			requestType::controlType, controlRequest,
			"Content-Length", body.size()
		).build(&body);
	}

	utility::web::insertSizeHeaderToHTTPMessage(request);

	try
	{
		clientStream << request;
	}
	catch (const web::WebException&)
	{

	}

	try
	{
		clientStream >> response;
	}
	catch (const web::WebException&)
	{

	}

	return response;
}