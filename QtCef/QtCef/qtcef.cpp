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

	// Enable High-DPI support on Windows 7 or newer.
	CefEnableHighDPISupport();

	void* sandbox_info = NULL;

#if defined(CEF_USE_SANDBOX)
	// Manage the life span of the sandbox information object. This is necessary
	// for sandbox support on Windows. See cef_sandbox_win.h for complete details.
	CefScopedSandboxInfo scoped_sandbox;
	sandbox_info = scoped_sandbox.sandbox_info();
#endif

	HINSTANCE hInstance = (HINSTANCE)GetModuleHandle(NULL);

	// Provide CEF with command-line arguments.
	CefMainArgs main_args(hInstance);

	// SimpleApp implements application-level callbacks. It will create the first
	// browser instance in OnContextInitialized() after CEF has initialized.
	CefRefPtr<SimpleApp> app(new SimpleApp((HWND)this->winId()));

	// CEF applications have multiple sub-processes (render, plugin, GPU, etc)
	// that share the same executable. This function checks the command-line and,
	// if this is a sub-process, executes the appropriate logic.
	int exit_code = CefExecuteProcess(main_args, app.get(), sandbox_info);
	if (exit_code >= 0) {
		// The sub-process has completed so return here.
		return;
	}

	// Specify CEF global settings here.
	CefSettings settings;

#if !defined(CEF_USE_SANDBOX)
	settings.no_sandbox = true;
#endif
	settings.multi_threaded_message_loop = true;
	settings.single_process = true;
	// Initialize CEF.
	CefInitialize(main_args, settings, app.get(), sandbox_info);

	// Run the CEF message loop. This will block until CefQuitMessageLoop() is
	// called.
// 	CefRunMessageLoop();

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
