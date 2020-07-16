#include "pch.h"

#include "ScreenFunctions.h"
#include "CloudStorageScreen.h"
#include "RegistrationScreen.h"
#include "AuthorizationScreen.h"
#include "UtilityFunctions.h"
#include "../UIConstants.h"
#include "../WindowProcedures/AuthorizationScreenProcedure.h"
#include "../WindowProcedures/RegistrationScreenProcedure.h"
#include "../WindowProcedures/CloudStorageScreenProcedure.h"

#include <commctrl.h>

using namespace std;

void clearColumns(UI::MainWindow& ref);

void updateNameColumn(UI::MainWindow& ref, const vector<db::fileDataRepresentation>& data);

void updateDateColumn(UI::MainWindow& ref, const vector<db::fileDataRepresentation>& data);

void updateSizeColumn(UI::MainWindow& ref, const vector<db::fileDataRepresentation>& data);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void initCloudStorageScreen(UI::MainWindow& ref)
{
	ref.setCurrentScreen(new UI::CloudStorageScreen(ref.getMainWindow(), UI::screens::cloudStorageScreen, CloudStorageScreenProcedure));
}

void initRegistrationScreen(UI::MainWindow& ref)
{
	ref.setCurrentScreen(new UI::RegistrationScreen(ref.getMainWindow(), UI::screens::registrationScreen, RegistrationScreenProcedure));
}

void initAuthorizationScreen(UI::MainWindow& ref)
{
	ref.setCurrentScreen(new UI::AuthorizationScreen(ref.getMainWindow(), UI::screens::authorizationScreen, AuthorizationScreenProcedure));
}

bool removeFileDialog(UI::MainWindow& ref, const std::wstring& fileName)
{
	int dialog = MessageBoxW
	(
		ref.getMainWindow(),
		wstring(L"�� ������������� ������ ������� " + fileName).data(),
		L"�������� �����",
		MB_YESNO | MB_ICONWARNING
	);

	return dialog == IDYES;
}

void updateColumns(UI::MainWindow& ref, const vector<db::fileDataRepresentation>& data)
{
	clearColumns(ref);

	updateNameColumn(ref, data);

	updateDateColumn(ref, data);

	updateSizeColumn(ref, data);

	//TODO: update other columns
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void clearColumns(UI::MainWindow& ref)
{
	bool success = false;

	do
	{
		LVITEMW item = {};
		item.iSubItem = UI::mainWindowUI::nameColumnIndex;

		success = SendMessageW(ref.getList(), LVM_GETITEMW, NULL, reinterpret_cast<LPARAM>(&item));

		SendMessageW(ref.getList(), LVM_DELETEITEM, item.iItem, NULL);
	} while (success);
}

void updateNameColumn(UI::MainWindow& ref, const vector<db::fileDataRepresentation>& data)
{
	LVITEMW item = {};

	RECT rect;

	GetClientRect(ref.getList(), &rect);

	item.mask = LVIF_TEXT;

	for (size_t i = 0; i < data.size(); i++)
	{
		item.pszText = const_cast<wchar_t*>(data[i].fileName.data());
		item.iItem = i;
		item.iSubItem = UI::mainWindowUI::nameColumnIndex;

		SendMessageW(ref.getList(), LVM_INSERTITEM, NULL, reinterpret_cast<LPARAM>(&item));
	}

	InvalidateRect(ref.getList(), &rect, TRUE);
}

void updateDateColumn(UI::MainWindow& ref, const vector<db::fileDataRepresentation>& data)
{
	LVITEMW item = {};

	RECT rect;

	GetClientRect(ref.getList(), &rect);

	item.mask = LVIF_TEXT;

	for (size_t i = 0; i < data.size(); i++)
	{
		item.pszText = const_cast<wchar_t*>(data[i].dateOfChange.data());
		item.iItem = i;
		item.iSubItem = UI::mainWindowUI::dateColumnIndex;
		
		SendMessageW(ref.getList(), LVM_SETITEMW, NULL, reinterpret_cast<LPARAM>(&item));
	}

	InvalidateRect(ref.getList(), &rect, TRUE);
}

void updateSizeColumn(UI::MainWindow& ref, const vector<db::fileDataRepresentation>& data)
{
	LVITEMW item = {};

	RECT rect;

	GetClientRect(ref.getList(), &rect);

	item.mask = LVIF_TEXT;

	for (size_t i = 0; i < data.size(); i++)
	{
		item.pszText = const_cast<wchar_t*>(data[i].fileSize.data());
		item.iItem = i;
		item.iSubItem = UI::mainWindowUI::sizeColumnIndex;

		SendMessageW(ref.getList(), LVM_SETITEMW, NULL, reinterpret_cast<LPARAM>(&item));
	}

	InvalidateRect(ref.getList(), &rect, TRUE);
}