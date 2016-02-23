#include "newcefwin.h"
#include <qt_windows.h>
#include "cefsimple/simple_app.h"
#include "include/cef_sandbox_win.h"
#include "cefsimple/simple_handler.h"

NewCefWin *NewCefWin::instance = NULL;
NewCefWin *NewCefWin::getInstance()
{
	if (!instance)
	{
		instance = new NewCefWin(NULL);
	}
	return instance;
}


NewCefWin::NewCefWin(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);

	setAttribute(Qt::WA_DeleteOnClose);

	// Information used when creating the native window.
	CefWindowInfo window_info;

#if defined(OS_WIN)
	// On Windows we need to specify certain flags that will be passed to
	// CreateWindowEx().
	//   window_info.SetAsPopup(NULL, "cefsimple");
	RECT rect;
	rect.left = 0;
	rect.top = 0;
	rect.right = 1000;
	rect.bottom = 500;
	window_info.SetAsChild((HWND)this->winId(), rect);
#endif

	// SimpleHandler implements browser-level callbacks.
	//   CefRefPtr<SimpleHandler> handler(new SimpleHandler());

	// Specify CEF browser settings here.
	CefBrowserSettings browser_settings;

	std::string url = "http://www.baidu.com";

	// Create the first browser window.
	CefBrowserHost::CreateBrowser(window_info, SimpleHandler::GetInstance(), url,
		browser_settings, NULL);
}

NewCefWin::~NewCefWin()
{
// 	SimpleHandler::GetInstance()->CloseAllBrowsers(false);
}
