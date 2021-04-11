#pragma once
#include <QTcpServer>
#include <QTcpSocket>

class ChatServer : public QTcpServer
{
    Q_OBJECT
private:
    QList<QTcpSocket*> m_clients;
    quint16 m_nextBlockSize;

    void sendToClients(QByteArray data);

public:
    ChatServer();

public slots:
    void startServer();
    void incomingConnection(qintptr socketDescriptor);
    void deleteSocket();
    void readClient();
};
