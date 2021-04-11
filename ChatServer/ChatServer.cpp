#include "ChatServer.h"

ChatServer::ChatServer()
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
    m_socket = new QTcpSocket(this);
    m_socket->setSocketDescriptor(socketDescriptor);

    connect(m_socket, SIGNAL(readyRead()), this, SLOT(socketReadReady()));
    connect(m_socket, SIGNAL(disconnected()), this, SLOT(deleteLater()));

    qDebug() << socketDescriptor << " succesfully connected";
}

void ChatServer::socketReadReady()
{

}
