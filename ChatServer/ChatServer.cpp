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
    Client* client = new Client(socket, this);
    m_clients.append(client);

    connect(socket, SIGNAL(readyRead()), this, SLOT(readClient()));
    connect(socket, SIGNAL(disconnected()), this , SLOT(deleteSocket()));
}

void ChatServer::deleteSocket()
{
    QTcpSocket* snd = (QTcpSocket*)sender();

    m_mutex.lock();
    for(int i = 0; i < m_clients.size(); i++)
        if(m_clients.at(i)->socket() == snd)
        {
            sendServerMessage(m_clients.at(i)->name() + " leave the chat");
            qDebug() << m_clients.at(i)->name() + " disconnected";
            m_clients.removeAt(i);
        }
    m_mutex.unlock();

    snd->deleteLater();
}

void ChatServer::readClient()
{
    QTcpSocket* clientSocket = (QTcpSocket*)sender();
    QDataStream in(clientSocket);
    in.setVersion(QDataStream::Qt_6_0);

    if(m_nextBlockSize == 0)
    {
        if(clientSocket->bytesAvailable() < sizeof(quint16))
            return;
        in >> m_nextBlockSize;
    }

    if(clientSocket->bytesAvailable() < m_nextBlockSize)
        return;

    quint8 command;
    in >> command;

    switch (command)
    {
    case message:
    {
        sendMessage(clientSocket);
        break;
    }
    case clientInfo:
    {
        authClient(clientSocket);
        break;
    }
    }
    m_nextBlockSize = 0;
}

void ChatServer::sendMessage(QTcpSocket* sender)
{
    QByteArray data;

    QDataStream in(sender);
    in.setVersion(QDataStream::Qt_6_0);
    QTime time;
    QString name, msg;
    in >> time >> name >> msg;

    QDataStream out(&data, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_6_0);
    out << quint16(0) << quint8(message) << time << name << msg;

    out.device()->seek(0);
    out << quint16(data.size() - sizeof(quint16));
    sendToAllClients(data);
}

void ChatServer::authClient(QTcpSocket *client)
{
    QDataStream in(client);
    in.setVersion(QDataStream::Qt_6_0);
    QString name;
    in >> name;

    m_mutex.lock();
    foreach(Client* authClient, m_clients)
        if(authClient->name() == name)
        {
            authFailed(client);
            m_mutex.unlock();
            return;
        }

    for(int i = m_clients.size() - 1; i >= 0; i--)
        if(m_clients.at(i)->socket() == client)
            m_clients.at(i)->setName(name);

    authSuccess(client);
    m_mutex.unlock();
}

void ChatServer::authFailed(QTcpSocket *client)
{
    sendServerMessage("Authentication failed", client);

    for(int i = m_clients.size() - 1; i >= 0; i--)
        if(m_clients.at(i)->socket() == client)
            m_clients.removeAt(i);
    qDebug() << client << ": authentication failed";

    client->disconnectFromHost();
}

void ChatServer::authSuccess(QTcpSocket* client)
{
    QString name;
    for(int i = m_clients.size() - 1; i >= 0; i--)
        if(m_clients.at(i)->socket() == client)
            name = m_clients.at(i)->name();

    sendServerMessage(name + " joined to the chat");

    qDebug() << name + " successfully connected";
}

void ChatServer::sendServerMessage(QString message, QTcpSocket *client)
{
    QByteArray data;

    QDataStream out(&data, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_6_0);

    out << quint16(0) << quint8(serverMessage) << QTime::currentTime() << " " + message;

    out.device()->seek(0);
    out << quint16(data.size() - sizeof(quint16));

    if(client)
    {
        sendToClient(data, client);
        return;
    }
    sendToAllClients(data);
}

void ChatServer::sendToClient(QByteArray data, QTcpSocket *client)
{
    client->write(data);
}

void ChatServer::sendToAllClients(QByteArray data)
{
    foreach(Client* client, m_clients)
        client->socket()->write(data);
}
