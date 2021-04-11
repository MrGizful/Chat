#include "ChatClient.h"

ChatClient::ChatClient(const QString& host, int port, QWidget *parent)
    : QWidget(parent)
{
    m_socket = new QTcpSocket(this);
    m_socket->connectToHost(host, port);

    connect(m_socket, SIGNAL(readyRead()), this, SLOT(socketReadReady()));
    connect(m_socket, SIGNAL(connected()), this, SLOT(socketConnected()));
    connect(m_socket, SIGNAL(disconnected()), this, SLOT(socketDisconnected()));
    connect(m_socket, SIGNAL(errorOccurred(QAbstractSocket::SocketError)), this, SLOT(socketError(QAbstractSocket::SocketError)));

    m_messages = new QTextEdit;
    m_messages->setReadOnly(true);
    m_message = new QLineEdit;

    QPushButton* sendButton = new QPushButton("Send");

    QHBoxLayout* hLayout = new QHBoxLayout;
    hLayout->addWidget(m_message, 3);
    hLayout->addWidget(sendButton);
    QVBoxLayout* vLayout = new QVBoxLayout;
    vLayout->addWidget(new QLabel("Chat"));
    vLayout->addWidget(m_messages);
    vLayout->addLayout(hLayout);
    setLayout(vLayout);
}

void ChatClient::socketConnected()
{
    QTime time;
    m_messages->append(time.currentTime().toString() + " Connected to the chat");
}

void ChatClient::socketDisconnected()
{
    m_socket->deleteLater();
}

void ChatClient::socketReadReady()
{
    if(m_socket->waitForConnected(500))
    {
        m_socket->waitForReadyRead(500);
        data = m_socket->readAll();
    }
}

void ChatClient::socketError(QAbstractSocket::SocketError error)
{
    QString strError = "Error: " + QString(m_socket->errorString());
}
