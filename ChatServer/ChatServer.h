#pragma once
#include <QTcpServer>
#include <QTcpSocket>

class ChatServer : public QTcpServer
{
    Q_OBJECT
private:
    QTcpSocket* m_socket;
    QByteArray data;

public:
    ChatServer();

public slots:
    void startServer();
    void incomingConnection(qintptr socketDescriptor);
    void socketReadReady();
};
