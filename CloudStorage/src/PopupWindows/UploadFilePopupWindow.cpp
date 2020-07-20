#include "pch.h"

#include "UploadFilePopupWindow.h"
#include "UtilityFunctions.h"
#include "../UIConstants.h"

#include <commctrl.h>

using namespace std;

namespace UI
{
	UploadFilePopupWindow::UploadFilePopupWindow(HWND disableWindow, const std::wstring& popupWindowClassName, const std::wstring& popupWindowTitle, WNDPROC procedure, const std::wstring& message) :
		BasePopupWindow(disableWindow, popupWindowClassName, popupWindowTitle, procedure, message)
	{
		RECT messagePos = utility::getWindowPosition(BasePopupWindow::messageStatic);
		RECT sizes;
		LONG width;
		LONG height;

		GetClientRect(BasePopupWindow::popupWindow, &sizes);

		width = sizes.right - sizes.left;
		height = sizes.bottom - sizes.top;

		progressBar = CreateWindowExW
		(
			NULL,
			PROGRESS_CLASSW,
			nullptr,
			WS_CHILDWINDOW | WS_VISIBLE,
			0,
			messagePos.top + basePopupWindow::messageStaticHeight,
			width,
			GetSystemMetrics(SM_CYVSCROLL),
			BasePopupWindow::popupWindow,
			HMENU(),
			nullptr,
			nullptr
		);

		SendMessageW(BasePopupWindow::popupWindow, UI::events::initDisableWindow, reinterpret_cast<WPARAM>(BasePopupWindow::disableWindow), NULL);
	}
}