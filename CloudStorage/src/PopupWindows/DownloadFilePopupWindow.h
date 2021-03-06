#pragma once

#include "../BaseClasses/BasePopupWindow.h"

namespace UI
{
	class DownloadFilePopupWindow : public BasePopupWindow
	{
	private:
		HWND progressBar;

	public:
		DownloadFilePopupWindow(HWND disableWindow, const std::wstring& message);

		HWND getDownloadProgressBar() const;

		void setProgressBarRange(int min, intmax_t max);

		~DownloadFilePopupWindow() = default;
	};
}