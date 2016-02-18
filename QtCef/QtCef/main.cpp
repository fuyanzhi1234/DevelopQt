#include "qtcef.h"
#include <QApplication>
#include <qt_windows.h>
#include "include/base/cef_scoped_ptr.h"
#include "include/cef_command_line.h"
#include "include/cef_sandbox_win.h"

#include "cefsimple/simple_app.h"
#include "include/cef_sandbox_win.h"
#include <QMessageBox>

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
