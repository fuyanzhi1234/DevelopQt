#include "serverwindow.h"

ServerWindow::ServerWindow(QWidget *parent)
	: QMainWindow(parent)
	, m_LocalSocket(NULL)
{
	ui.setupUi(this);


	m_LocalServer = new QLocalServer(this);
	if (!m_LocalServer->listen("fucking"))
	{
		qDebug() << "Server listen failed!";
		return;
	}

	connect(ui.pushButton_server_send, SIGNAL(clicked()), SLOT(sendToClinet()));
	connect(m_LocalServer, SIGNAL(newConnection()), SLOT(newClientConnetion()));

	// start child process
	QString clientWindow = QString("ClientWindow.exe");
	process.start(clientWindow, QStringList() << QString::number(ui.widget_clientwin->winId()));
	process.waitForStarted();
}

ServerWindow::~ServerWindow()
{
}

void ServerWindow::sendToClinet()
{
	QString sendContent = ui.lineEdit_server_send->text();
	ui.lineEdit_server_send->clear();
	qDebug()<<sendContent;
	if (m_LocalSocket)
	{
		m_LocalSocket->write(sendContent.toUtf8());
		ui.listWidget_server_recieve->addItem("Send : " + sendContent);
	}
}

void ServerWindow::newClientConnetion()
{
	qDebug() << "newClientConnetion";
	m_LocalSocket = m_LocalServer->nextPendingConnection();
	qDebug()<<m_LocalSocket->socketDescriptor();
	connect(m_LocalSocket, SIGNAL(disconnected()), SLOT(socketDisconnect()));
	connect(m_LocalSocket, SIGNAL(readyRead()), SLOT(readFromClient()));

}

void ServerWindow::socketDisconnect()
{
	ui.listWidget_server_recieve->addItem("socketDisconnect");
	disconnect(m_LocalSocket, SIGNAL(disconnected()), this, SLOT(socketDisconnect()));
	disconnect(m_LocalSocket, SIGNAL(readyRead()), this, SLOT(readFromClient()));
	m_LocalSocket->deleteLater();
}

void ServerWindow::readFromClient()
{
	qDebug() << "readyReadClientData";

	while (m_LocalSocket->bytesAvailable())
	{
		QString data;
		data.append(m_LocalSocket->readLine());
		ui.listWidget_server_recieve->addItem("Rev : " + data);
	}
}