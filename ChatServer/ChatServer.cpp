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
        QByteArray data;
        QDataStream out(&data, QIODevice::WriteOnly);
        out.setVersion(QDataStream::Qt_6_0);
        QTime time;
        QString name, msg;
        in >> time >> name >> msg;
        out << quint16(0) << quint8(message) << time << name << msg;

        out.device()->seek(0);
        out << quint16(data.size() - sizeof(quint16));
        sendToAllClients(data);
        break;
    }
    }
    m_nextBlockSize = 0;
}

void ChatServer::sendToAllClients(QByteArray data)
{
    foreach(QTcpSocket* client, m_clients)
        client->write(data);
}
