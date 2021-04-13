#pragma once
#include <QTime>
#include <QMutex>
#include <QTcpServer>
#include <QTcpSocket>
#include "Client.h"
#include "../Chat/ChatClient/Commands.h"

class ChatServer : public QTcpServer
{
    Q_OBJECT
private:
    QList<Client*> m_clients;
    QMutex m_mutex;
    quint16 m_nextBlockSize;

    void sendToAllClients(QByteArray data);
    void sendToClient(QByteArray data, QTcpSocket* client);
    void sendMessage(QTcpSocket* sender);
    void sendServerMessage(QString message, QTcpSocket* client = nullptr);
    void authClient(QTcpSocket* client);
    void authFailed(QTcpSocket* client);
    void authSuccess(QTcpSocket* client);

public:
    ChatServer();

public slots:
    void startServer();
    void incomingConnection(qintptr socketDescriptor);
    void deleteSocket();
    void readClient();
};
