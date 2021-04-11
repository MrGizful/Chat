#include "ChatServer.h"

ChatServer::ChatServer() : m_nextBlockSize(0)
{

}

void ChatServer::startServer()
{
    if(listen(QHostAddress::Any, 2323))
        qDebug() << "Server started!";
    else
        qDebug() << "Unable to start the server: " + errorString();
}

void ChatServer::incomingConnection(qintptr socketDescriptor)
{
    QTcpSocket* socket = new QTcpSocket(this);
    socket->setSocketDescriptor(socketDescriptor);
    m_clients.append(socket);

    connect(socket, SIGNAL(readyRead()), this, SLOT(readClient()));
    connect(socket, SIGNAL(disconnected()), this , SLOT(deleteSocket()));

    qDebug() << socketDescriptor << " succesfully connected";
}

void ChatServer::deleteSocket()
{
    QTcpSocket* snd = (QTcpSocket*)sender();
    for(int i = 0; i < m_clients.size(); i++)
        if(m_clients.at(i) == snd)
            m_clients.removeAt(i);
    snd->deleteLater();
}

void ChatServer::readClient()
{
    QTcpSocket* clientSocket = (QTcpSocket*)sender();
    QByteArray data = clientSocket->readAll();
    sendToClients(data);
}

void ChatServer::sendToClients(QByteArray data)
{
    foreach(QTcpSocket* client, m_clients)
        client->write(data);
}
