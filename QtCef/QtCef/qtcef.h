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

private slots:
	// �Ŵ�
	void OnZoomIn();

	// ��С
	void OnZoomOut();

	// �´���
	void OnNewWin();

private:
	Ui::QtCefClass ui;
};

#endif // QTCEF_H
