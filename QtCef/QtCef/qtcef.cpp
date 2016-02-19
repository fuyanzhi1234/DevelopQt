#include "qtcef.h"
#include <qt_windows.h>
#include "cefsimple/simple_app.h"
#include "include/cef_sandbox_win.h"
#include "cefsimple/simple_handler.h"
#include "newcefwin.h"

QtCef::QtCef(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);

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
	connect(ui.pushButton_zoom_in, SIGNAL(clicked()), SLOT(OnZoomIn()));
	connect(ui.pushButton_zoom_out, SIGNAL(clicked()), SLOT(OnZoomOut()));
	connect(ui.pushButton_newwin, SIGNAL(clicked()), SLOT(OnNewWin()));
}

QtCef::~QtCef()
{

}

// 放大
void QtCef::OnZoomIn()
{
	SimpleHandler::GetInstance()->setZoom(0.5);
}

// 缩小
void QtCef::OnZoomOut()
{
	SimpleHandler::GetInstance()->setZoom(-0.5);
}

// 新窗口
void QtCef::OnNewWin()
{
	NewCefWin *newWin = new NewCefWin();
	newWin->show();
}
