#include "pch.h"

#include "BasePopupWindow.h"
#include "UtilityFunctions.h"
#include "../UIConstants.h"

using namespace std;

namespace UI
{
	BasePopupWindow::BasePopupWindow(HWND disableWindow, const wstring& popupWindowClassName, const wstring& popupWindowTitle, WNDPROC procedure, const wstring& message) : 
		disableWindow(disableWindow)
	{
		WNDCLASSEXW popup = {};
		POINT centerOfDesktop = utility::centerCoordinates(popupWindows::basePopupWindow::popupWindowWidth, popupWindows::basePopupWindow::popupWindowHeight);
		POINT centerPopup;
		RECT sizes;
		LONG width;
		LONG height;

		popup.cbSize = sizeof(popup);
		popup.lpfnWndProc = procedure;
		popup.lpszClassName = popupWindowClassName.data();
		popup.hbrBackground = CreateSolidBrush(RGB(255, 255, 255));

		RegisterClassExW(&popup);

		EnableWindow(disableWindow, FALSE);

		popupWindow = CreateWindowExW
		(
			NULL,
			popup.lpszClassName,
			popupWindowTitle.data(),
			WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME | WS_VISIBLE,
			centerOfDesktop.x,
			centerOfDesktop.y,
			popupWindows::basePopupWindow::popupWindowWidth,
			popupWindows::basePopupWindow::popupWindowHeight,
			nullptr,
			HMENU(),
			nullptr,
			nullptr
		);
		
		GetClientRect(popupWindow, &sizes);

		width = sizes.right - sizes.left;
		height = sizes.bottom - sizes.top;
		centerPopup = utility::centerCoordinates(width, popupWindows::basePopupWindow::messageStaticHeight, popupWindow);
		centerPopup.y /= 2;

		messageStatic = CreateWindowExW
		(
			NULL,
			L"STATIC",
			message.data(),
			WS_CHILDWINDOW | SS_CENTER | WS_VISIBLE,
			0,
			centerPopup.y,
			width,
			popupWindows::basePopupWindow::messageStaticHeight,
			popupWindow,
			HMENU(),
			nullptr,
			nullptr
		);

		cancelButton = CreateWindowExW
		(
			NULL,
			L"BUTTON",
			L"������",
			WS_CHILDWINDOW | WS_VISIBLE,
			width - popupWindows::basePopupWindow::cancelButtonWidth,
			height - popupWindows::basePopupWindow::cancelButtonHeight,
			popupWindows::basePopupWindow::cancelButtonWidth,
			popupWindows::basePopupWindow::cancelButtonHeight,
			popupWindow,
			HMENU(cancel),
			nullptr,
			nullptr
		);
	}

	HWND BasePopupWindow::getPopupWindow() const
	{
		return popupWindow;
	}

	HWND BasePopupWindow::getDisableWindow() const
	{
		return disableWindow;
	}

	HWND BasePopupWindow::getCancelButton() const
	{
		return cancelButton;
	}

	BasePopupWindow::~BasePopupWindow()
	{
		EnableWindow(disableWindow, TRUE);
		DestroyWindow(popupWindow);
	}
}