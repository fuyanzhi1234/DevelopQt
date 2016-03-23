#ifndef QTCEF_H
#define QTCEF_H

#include <QtWidgets/QMainWindow>
#include "ui_qtcef.h"

class QtCef : public QMainWindow
{
	Q_OBJECT

public:
	QtCef(QWidget *parent = 0);
	~QtCef();

protected:
	void closeEvent(QCloseEvent *e);

private slots:
	// 放大
	void OnZoomIn();

	// 缩小
	void OnZoomOut();

	// 新窗口
	void OnNewWin();

	// 打开网页
	void OnGo();

	// 显示授权对话框
	void OnShowAuthorityDialog(int browserIdentifier, QString userName, QString userPassword);

	// 浏览器创建成功
	void OnCreateBrowserSuccess(HWND parentHwnd, HWND browserHwnd, int browserIdentifier);

private:
	Ui::QtCefClass ui;

	// 浏览器标识
	int m_browserIdentifier;
};

#endif // QTCEF_H
