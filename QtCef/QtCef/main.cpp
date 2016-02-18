#include "qtcef.h"
#include <QApplication>
#include <qt_windows.h>
#include "include/base/cef_scoped_ptr.h"
#include "include/cef_command_line.h"
#include "include/cef_sandbox_win.h"
#include "cefclient/browser/client_app_browser.h"
#include "cefclient/browser/main_context_impl.h"
#include "cefclient/browser/main_message_loop_multithreaded_win.h"
#include "cefclient/browser/main_message_loop_std.h"
#include "cefclient/browser/root_window_manager.h"
#include "cefclient/browser/test_runner.h"
#include "cefclient/common/client_app_other.h"
#include "cefclient/renderer/client_app_renderer.h"

#include "cefsimple/simple_app.h"
#include "include/cef_sandbox_win.h"
#include <QMessageBox>

using namespace client;

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);

// 	QMessageBox::about(NULL, "main", "fuck");

	QtCef w;
	w.show();

	int result = a.exec();
	// Shut down CEF.
// 	CefShutdown();

	return result;
}
