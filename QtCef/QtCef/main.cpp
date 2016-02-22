#include "qtcef.h"
#include <QApplication>
#include <qt_windows.h>
#include "include/base/cef_scoped_ptr.h"
#include "include/cef_command_line.h"
#include "include/cef_sandbox_win.h"

#include "cefsimple/simple_app.h"
#include "include/cef_sandbox_win.h"
#include <QMessageBox>
#include "cefsimple/simple_handler.h"

int main(int argc, char *argv[])
{
// 	argc = 2;
// 	argv[1] = "--ppapi-flash-path=plugins\\pepflashplayer.dll";
	QApplication a(argc, argv);

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
// 	QMessageBox::about(NULL, "main", "1");
	CefRefPtr<SimpleApp> app(new SimpleApp());

	// CEF applications have multiple sub-processes (render, plugin, GPU, etc)
	// that share the same executable. This function checks the command-line and,
	// if this is a sub-process, executes the appropriate logic.
// 	QMessageBox::about(NULL, "main", "2");
	int exit_code = CefExecuteProcess(main_args, app.get(), sandbox_info);
// 	// The sub-process has completed so return here.
// 	QMessageBox::about(NULL, "main", QString("%1").arg(exit_code));
// 
	if (exit_code >= 0) {
// 		QMessageBox::about(NULL, "main", "3");		return 0;
	}
// 	QMessageBox::about(NULL, "main", "4");
	// Specify CEF global settings here.
	CefSettings settings;

#if !defined(CEF_USE_SANDBOX)
	settings.no_sandbox = true;
#endif
	settings.multi_threaded_message_loop = true;
// 	settings.single_process = true;
	// Initialize CEF.
	CefInitialize(main_args, settings, app.get(), sandbox_info);

	QtCef w;
	w.show();

	int result = a.exec();
	// Shut down CEF.
	CefShutdown();

	return result;
}
