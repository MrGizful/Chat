#pragma once
#include <QTime>
#include <QTcpServer>
#include <QTcpSocket>
#include "../Chat/ChatClient/Commands.h"

class ChatServer : public QTcpServer
{
    Q_OBJECT
private:
    QList<QTcpSocket*> m_clients;
    quint16 m_nextBlockSize;

    void sendToAllClients(QByteArray data);

public:
    ChatServer();

public slots:
    void startServer();
    void incomingConnection(qintptr socketDescriptor);
    void deleteSocket();
    void readClient();
};
