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
	// �Ŵ�
	void OnZoomIn();

	// ��С
	void OnZoomOut();

	// �´���
	void OnNewWin();

	// ����ҳ
	void OnGo();

	// ��ʾ��Ȩ�Ի���
	void OnShowAuthorityDialog(int browserIdentifier, QString userName, QString userPassword);

	// ����������ɹ�
	void OnCreateBrowserSuccess(HWND parentHwnd, HWND browserHwnd, int browserIdentifier);

private:
	Ui::QtCefClass ui;

	// �������ʶ
	int m_browserIdentifier;
};

#endif // QTCEF_H
