#include "ChatServer.h"

//Define interaction elements and create layout
ChatServer::ChatServer(QWidget* parent) : QWidget(parent), m_nextBlockSize(0)
{
    m_server = new QTcpServer;
    m_console = new QTextEdit;
    m_command = new QLineEdit;

    m_console->setReadOnly(true);

    QLabel* commandLbl = new QLabel("Command");

    QHBoxLayout* hLayout = new QHBoxLayout;
    hLayout->addWidget(commandLbl);
    hLayout->addWidget(m_command, 3);
    QVBoxLayout* vLayout = new QVBoxLayout;
    vLayout->addWidget(m_console, 4);
    vLayout->addLayout(hLayout);
    setLayout(vLayout);

    setMinimumSize(350, 250);
    connect(m_server, SIGNAL(newConnection()), this, SLOT(slotNewConnection()));
    connect(m_command, SIGNAL(returnPressed()), this, SLOT(serverCommand()));
}

void ChatServer::startServer()
{
    if(m_server->isListening())
    {
        m_console->append(QTime::currentTime().toString() + " Can't start server: It's already started");
        return;
    }

    if(m_server->listen(QHostAddress::Any, 2323))
        m_console->append(QTime::currentTime().toString() + " Server started!");
    else
        m_console->append(QTime::currentTime().toString() + " Unable to start the server: " + m_server->errorString());
}

//Register new connection
void ChatServer::slotNewConnection()
{
    QTcpSocket* clientSocket = m_server->nextPendingConnection();
    Client* client = new Client(clientSocket, this);
    m_clients.append(client);

    connect(clientSocket, SIGNAL(disconnected()), this, SLOT(deleteSocket()));
    connect(clientSocket, SIGNAL(readyRead()), this, SLOT(readClient()));
}

//Find and delete disconnected client
//Send notification to server and other clients
void ChatServer::deleteSocket()
{
    QTcpSocket* snd = (QTcpSocket*)sender();

    m_mutex.lock();
    for(int i = 0; i < m_clients.size(); i++)
        if(m_clients.at(i)->socket() == snd)
        {
            sendServerMessage(m_clients.at(i)->name() + " leave the chat");
            m_console->append(QTime::currentTime().toString() + " " + m_clients.at(i)->name() + " disconnected");
            m_clients.removeAt(i);
        }
    m_mutex.unlock();

    snd->deleteLater();
}

void ChatServer::readClient()
{
    //Spot the sender
    QTcpSocket* clientSocket = (QTcpSocket*)sender();
    QDataStream in(clientSocket);
    in.setVersion(QDataStream::Qt_6_0);

    //Checking message integrity
    if(m_nextBlockSize == 0)
    {
        if(clientSocket->bytesAvailable() < sizeof(quint16))
            return;
        in >> m_nextBlockSize;
    }

    if(clientSocket->bytesAvailable() < m_nextBlockSize)
        return;

    //Define the command
    quint8 command;
    in >> command;

    //Execute command
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

//Send client message to all clients
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

    //Checking for the existing a client
    m_mutex.lock();
    foreach(Client* authClient, m_clients)
        if(authClient->name() == name)
        {
            //Client exists
            authFailed(client);
            m_mutex.unlock();
            return;
        }

    //Client not exists
    for(int i = m_clients.size() - 1; i >= 0; i--)
        if(m_clients.at(i)->socket() == client)
            m_clients.at(i)->setName(name);

    authSuccess(client);
    m_mutex.unlock();
}

void ChatServer::authFailed(QTcpSocket *client)
{
    //Messaging the client about failed authentication
    sendServerMessage("Authentication failed", client);

    //Delete client
    for(int i = m_clients.size() - 1; i >= 0; i--)
        if(m_clients.at(i)->socket() == client)
            m_clients.removeAt(i);
    //Console message about failed authentication
    m_console->append(QTime::currentTime().toString() + " " +
                      QString::number(client->socketDescriptor()) + ": authentication failed");

    client->disconnectFromHost();
}

void ChatServer::authSuccess(QTcpSocket* client)
{
    //Spot the client
    QString name;
    for(int i = m_clients.size() - 1; i >= 0; i--)
        if(m_clients.at(i)->socket() == client)
            name = m_clients.at(i)->name();

    //Send messages that client connected
    sendServerMessage(name + " joined to the chat");

    m_console->append(QTime::currentTime().toString() + " " + name +
                      "(" + QString::number(client->socketDescriptor()) + ") " + " successfully connected");
}

//Send server messages from server to client/clients
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

//Send message to client
void ChatServer::sendToClient(QByteArray data, QTcpSocket *client)
{
    client->write(data);
}

//Send message to all clients
void ChatServer::sendToAllClients(QByteArray data)
{
    foreach(Client* client, m_clients)
        client->socket()->write(data);
}

void ChatServer::stopServer()
{
    this->close();
}

//Defining and executing server commands
void ChatServer::serverCommand()
{
    QString consoleCmd = m_command->text();
    if(consoleCmd.isNull() || (consoleCmd.remove(' ') == ""))
        return;

    if(consoleCmd == "stop")
        stopServer();
    if(consoleCmd == "start")
        startServer();

    m_command->setText("");
}
