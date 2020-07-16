#include "pch.h"

#include "IOSocketStream.h"
#include "HTTPNetwork.h"
#include "UtilityFunctions.h"
#include "NetworkFunctions.h"
#include "Screens/ScreenFunctions.h"
#include "Screens/AuthorizationScreen.h"
#include "Screens/RegistrationScreen.h"
#include "Screens/CloudStorageScreen.h"
#include "fileData.h"
#include "UIConstants.h"
#include "MainWindow.h"
#include "Log.h"

#include <commctrl.h>

#pragma comment (lib, "HTTP.lib")
#pragma comment (lib, "SocketStreams.lib")
#pragma comment (lib, "Log.lib")

#pragma comment (lib, "Comctl32.lib")

#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

using namespace std;

LRESULT __stdcall MainWindowProcedure(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

namespace UI
{
	MainWindow::MainWindow() :
		currentScreen(nullptr)
	{
		INITCOMMONCONTROLSEX init = {};
		WNDCLASSEXW wndClass = {};
		POINT monitorCenter = utility::centerCoordinates(UI::mainWindowUI::mainWindowWidth, UI::mainWindowUI::mainWindowHeight);

		init.dwICC = ICC_LISTVIEW_CLASSES;
		wndClass.cbSize = sizeof(WNDCLASSEXW);
		wndClass.lpfnWndProc = MainWindowProcedure;
		wndClass.hbrBackground = CreateSolidBrush(RGB(255, 255, 255));
		wndClass.hCursor = LoadCursorW(nullptr, IDC_ARROW);
		wndClass.lpszClassName = L"MainWindow";
		wndClass.hIcon = LoadIconW(nullptr, IDI_APPLICATION);

		RegisterClassExW(&wndClass);

		mainWindow = CreateWindowExW
		(
			NULL,
			wndClass.lpszClassName,
			L"Cloud Storage",
			WS_OVERLAPPEDWINDOW | WS_VISIBLE,
			monitorCenter.x,
			monitorCenter.y,
			UI::mainWindowUI::mainWindowWidth,
			UI::mainWindowUI::mainWindowHeight,
			nullptr,
			HMENU(),
			nullptr,
			nullptr
		);

		InitCommonControlsEx(&init);

		initAuthorizationScreen(*this);

		currentScreen->pubShow();
	}

	HWND MainWindow::getHWND(elementsEnum id) const
	{
		switch (id)
		{
		case UI::MainWindow::elementsEnum::mainWindow:
			return mainWindow;

		case UI::MainWindow::elementsEnum::wrapper:
			return currentScreen->getWrapper();

		case UI::MainWindow::elementsEnum::refreshButton:
			return static_cast<CloudStorageScreen*>(currentScreen.get())->getRefreshButton();

		case UI::MainWindow::elementsEnum::downloadButton:
			return static_cast<CloudStorageScreen*>(currentScreen.get())->getDownloadButton();

		case UI::MainWindow::elementsEnum::removeButton:
			return static_cast<CloudStorageScreen*>(currentScreen.get())->getRemoveButton();

		case UI::MainWindow::elementsEnum::list:
			return static_cast<CloudStorageScreen*>(currentScreen.get())->getList();

		case UI::MainWindow::elementsEnum::authorizationLoginEdit:
			return static_cast<AuthorizationScreen*>(currentScreen.get())->getAuthorizationLoginEdit();

		case UI::MainWindow::elementsEnum::authorizationPasswordEdit:
			return static_cast<AuthorizationScreen*>(currentScreen.get())->getAuthorizationPasswordEdit();

		case UI::MainWindow::elementsEnum::registrationLoginEdit:
			return static_cast<RegistrationScreen*>(currentScreen.get())->getRegistrationLoginEdit();

		case UI::MainWindow::elementsEnum::registrationPasswordEdit:
			return static_cast<RegistrationScreen*>(currentScreen.get())->getRegistrationPasswordEdit();

		case UI::MainWindow::elementsEnum::registrationRepeatPasswordEdit:
			return static_cast<RegistrationScreen*>(currentScreen.get())->getRegistrationRepeatPasswordEdit();

		default:
			return nullptr;
		}
	}

	MainWindow::~MainWindow()
	{
		DestroyWindow(mainWindow);
	}

	MainWindow& MainWindow::get()
	{
		static MainWindow instance;

		return instance;
	}

	BaseScreen* MainWindow::getCurrentScreen()
	{
		return currentScreen.get();
	}

	void MainWindow::setCurrentScreen(BaseScreen* screen)
	{
		currentScreen.reset(screen);
	}

	void MainWindow::resize()
	{
		if (currentScreen.get())
		{
			currentScreen->pubResize();
		}
	}

	HWND MainWindow::getMainWindow() const
	{
		return this->getHWND(elementsEnum::mainWindow);
	}

	HWND MainWindow::getRefreshButton() const
	{
		return this->getHWND(elementsEnum::refreshButton);
	}

	HWND MainWindow::getDownloadButton() const
	{
		return this->getHWND(elementsEnum::downloadButton);
	}

	HWND MainWindow::getRemoveButton() const
	{
		return this->getHWND(elementsEnum::removeButton);
	}

	HWND MainWindow::getList() const
	{
		return this->getHWND(elementsEnum::list);
	}

	HWND MainWindow::getAuthorizationLoginEdit() const
	{
		return this->getHWND(elementsEnum::authorizationLoginEdit);
	}

	HWND MainWindow::getAuthorizationPasswordEdit() const
	{
		return this->getHWND(elementsEnum::authorizationPasswordEdit);
	}

	HWND MainWindow::getRegistrationLoginEdit() const
	{
		return this->getHWND(elementsEnum::registrationLoginEdit);
	}

	HWND MainWindow::getRegistrationPasswordEdit() const
	{
		return this->getHWND(elementsEnum::registrationPasswordEdit);
	}

	HWND MainWindow::getRegistrationRepeatPasswordEdit() const
	{
		return this->getHWND(elementsEnum::registrationRepeatPasswordEdit);
	}

	HWND MainWindow::getWrapper() const
	{
		return this->getHWND(elementsEnum::wrapper);
	}
}

LRESULT __stdcall MainWindowProcedure(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	static streams::IOSocketStream<char> clientStream(new buffers::IOSocketBuffer<char>(new web::HTTPNetwork()));
	static UI::MainWindow* ptr = nullptr;
	static vector<db::fileDataRepresentation> fileNames;
	static wstring login;

	switch (msg)
	{
	case WM_KEYUP:
		if (wparam == VK_TAB)
		{
			try
			{
				SetFocus(dynamic_cast<UI::IIterable<HWND>&>(*ptr->getCurrentScreen()).next(GetFocus()));
			}
			catch (const bad_cast&)
			{
				//TODO: info about which screen trying to be cast
			}
		}

		return 0;

	case WM_COMMAND:
		switch (wparam)
		{
		case UI::buttons::refresh:
			getFiles(*ptr, clientStream, fileNames, true);

			break;

		case UI::buttons::download:
			downloadFile(*ptr, clientStream, fileNames, login);

			break;

		case UI::buttons::remove:
			removeFile(*ptr, clientStream, fileNames, login);

			SendMessageW(ptr->getMainWindow(), UI::events::getFilesE, NULL, NULL);

			break;

		case UI::buttons::authorization:
			login = authorization(*ptr, clientStream);

			if (login.size())
			{
				initCloudStorageScreen(*ptr);

				ptr->getCurrentScreen()->pubShow();

				SendMessageW(ptr->getMainWindow(), UI::events::getFilesE, NULL, NULL);
			}

			break;

		case UI::buttons::toRegistrationScreen:
			initRegistrationScreen(*ptr);

			ptr->getCurrentScreen()->pubShow();

			break;

		case UI::buttons::registration:
			login = registration(*ptr, clientStream);

			if (login.size())
			{
				initCloudStorageScreen(*ptr);

				ptr->getCurrentScreen()->pubShow();

				SendMessageW(ptr->getMainWindow(), UI::events::getFilesE, NULL, NULL);
			}

			break;

		case UI::buttons::toAuthorizationScreen:
			initAuthorizationScreen(*ptr);

			ptr->getCurrentScreen()->pubShow();

			break;
		}

		return 0;

#pragma region CustomEvents
	case UI::events::getFilesE:
		getFiles(*ptr, clientStream, fileNames, false);

		return 0;

	case UI::events::uploadFileE:
		uploadFile(*ptr, clientStream, *reinterpret_cast<vector<wstring>*>(wparam), login);

		return 0;

	case UI::events::downLoadFilesE:
		downloadFile(*ptr, clientStream, fileNames, login);

		return 0;

	case UI::events::initMainWindowPtrE:
		ptr = reinterpret_cast<UI::MainWindow*>(wparam);

		return 0;
#pragma endregion

	case WM_SIZE:
		if (ptr)
		{
			ptr->resize();
		}

		return 0;

	case WM_DESTROY:
		exitFromApplication(*ptr, clientStream);

		this_thread::sleep_for(0.6s);

		PostQuitMessage(0);

		return 0;

	default:
		return DefWindowProcW(hwnd, msg, wparam, lparam);
	}
}
