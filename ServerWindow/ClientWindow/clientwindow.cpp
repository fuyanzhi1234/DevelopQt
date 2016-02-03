#include "clientwindow.h"
#include <qt_windows.h>

ClientWindow::ClientWindow(QWidget *parent, HWND hParent)
	: QMainWindow(parent)
{
	ui.setupUi(this);
	setWindowFlags(windowFlags() | Qt::FramelessWindowHint | Qt::WindowMinimizeButtonHint);

	SetParent((HWND)winId(), hParent);
	MoveWindow((HWND)winId(), 0, 0, 600, 600, true);
	ShowWindow((HWND)winId(), SW_SHOW);
	UpdateWindow((HWND)winId());
	SetFocus((HWND)winId());
	
	m_LocalSocket = new QLocalSocket(this);
	m_LocalSocket->connectToServer("fucking");

	connect(m_LocalSocket, SIGNAL(connected()), SLOT(connectedToServer()));
	connect(m_LocalSocket, SIGNAL(disconnected()), SLOT(disConnectedToServer()));
	connect(m_LocalSocket, SIGNAL(readyRead()), SLOT(readFromServer()));
	connect(m_LocalSocket, SIGNAL(error(QAbstractSocket::SocketError)), SLOT(socketError(QAbstractSocket::SocketError)));

	connect(ui.pushButton_client_send, SIGNAL(clicked()), SLOT(sendToServer()));

}

ClientWindow::~ClientWindow()
{

}

void ClientWindow::sendToServer()
{
	QString sendContent = ui.lineEdit_client_send->text();
	ui.lineEdit_client_send->clear();
	qDebug() << sendContent;
	if (m_LocalSocket)
	{
		m_LocalSocket->write(sendContent.toUtf8());
		ui.listWidget_client_recieve->addItem("Send : " + sendContent);
	}
}

void ClientWindow::connectedToServer()
{
	qDebug()<<"connectedToServer";
}

void ClientWindow::disConnectedToServer()
{
	qDebug() << "disConnectedToServer";
	ui.listWidget_client_recieve->addItem("disConnectedToServer");
}

void ClientWindow::readFromServer()
{
	qDebug() << "readFromServer";

	while (m_LocalSocket->bytesAvailable())
	{
		QString data;
		data.append(m_LocalSocket->readLine());
		ui.listWidget_client_recieve->addItem("Rev : " + data);
	}
}

void ClientWindow::socketError(QAbstractSocket::SocketError)
{
	qDebug() << "socketError";
}