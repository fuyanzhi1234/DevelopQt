#include "qtcef.h"
#include <qt_windows.h>
#include "cefsimple/simple_app.h"
#include "include/cef_sandbox_win.h"
#include "cefsimple/simple_handler.h"
#include "newcefwin.h"
#include <QMessageBox>

QtCef::QtCef(QWidget *parent)
	: QMainWindow(parent)
	, m_browserIdentifier(0)
{
	ui.setupUi(this);

// 	setWindowFlags(windowFlags() | Qt::FramelessWindowHint);

	NewCefWin::getInstance()->hide();

	// Information used when creating the native window.
	CefWindowInfo window_info;

#if defined(OS_WIN)
	// On Windows we need to specify certain flags that will be passed to
	RECT rect;
	rect.left = 0;
	rect.top = 50;
	rect.right = 1024;
	rect.bottom = 818;
	window_info.SetAsChild((HWND)this->winId(), rect);
#endif

	// Specify CEF browser settings here.
	CefBrowserSettings browser_settings;

	std::string url = "http://www.tudou.com/albumplay/LN2umj8JZlQ/2Or-emCvxZM.html";
	// Create the first browser window.
	CefBrowserHost::CreateBrowser(window_info, SimpleHandler::GetInstance(), url,
		browser_settings, NULL);
	connect(ui.pushButton_zoom_in, SIGNAL(clicked()), SLOT(OnZoomIn()));
	connect(ui.pushButton_zoom_out, SIGNAL(clicked()), SLOT(OnZoomOut()));
	connect(ui.pushButton_newwin, SIGNAL(clicked()), SLOT(OnNewWin()));
	connect(ui.pushButton_go, SIGNAL(clicked()), SLOT(OnGo()));

	connect(SimpleHandler::GetInstance(), SIGNAL(showAuthorityDialog(int, QString, QString)), SLOT(OnShowAuthorityDialog(int, QString, QString)));
	qRegisterMetaType<HWND>("HWND");
	connect(SimpleHandler::GetInstance(), SIGNAL(creatBrowserSuccess(HWND, HWND, int)), SLOT(OnCreateBrowserSuccess(HWND, HWND, int)));
}

QtCef::~QtCef()
{
}

void QtCef::closeEvent(QCloseEvent *e)
{
}

// 放大
void QtCef::OnZoomIn()
{
	SimpleHandler::GetInstance()->SetZoom(m_browserIdentifier, 0.5);
}

// 缩小
void QtCef::OnZoomOut()
{
	SimpleHandler::GetInstance()->SetZoom(m_browserIdentifier, -0.5);
}

// 新窗口
void QtCef::OnNewWin()
{
	SimpleHandler::GetInstance()->showDevTool(m_browserIdentifier);
}

// 打开网页
void QtCef::OnGo()
{
	SimpleHandler::GetInstance()->loadURL(m_browserIdentifier, ui.lineEdit_url->text());

}

// 显示授权对话框
void QtCef::OnShowAuthorityDialog(int browserIdentifier, QString userName, QString userPassword)
{
	if (browserIdentifier == m_browserIdentifier)
	{
		userName = "fuyanzhi";
		userPassword = "linlin1314";
		QMessageBox::about(NULL, "1", "2");
		SimpleHandler::GetInstance()->RefreshWithAuthInfo(m_browserIdentifier, userName, userPassword);
	}
}

// 浏览器创建成功
void QtCef::OnCreateBrowserSuccess(HWND parentHwnd, HWND browserHwnd, int browserIdentifier)
{
	if (parentHwnd == (HWND)this->winId())
	{
		m_browserIdentifier = browserIdentifier;
	}
}
