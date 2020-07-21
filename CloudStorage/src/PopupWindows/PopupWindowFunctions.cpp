#include "pch.h"

#include "PopupWindowFunctions.h"
#include "UploadFilePopupWindow.h"
#include "DownloadFilePopupWindow.h"
#include "../UIConstants.h"

using namespace std;

void initUploadFilePopupWindow(UI::MainWindow& ref, const wstring& message)
{
	ref.setCurrentPopupWindow
	(
		new UI::UploadFilePopupWindow
		(
			ref.getMainWindow(),
			message
		)
	);
}

void initDownloadFilePopupWindow(UI::MainWindow& ref, const wstring& message)
{
	ref.setCurrentPopupWindow
	(
		new UI::DownloadFilePopupWindow
		(
			ref.getMainWindow(),
			message
		)
	);
}