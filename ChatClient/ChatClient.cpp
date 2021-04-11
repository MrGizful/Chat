#include "ChatClient.h"

ChatClient::ChatClient(const QString& host, int port, QWidget *parent)
    : QWidget(parent), m_nextBlockSize(0)
{
    m_host = host;
    m_port = port;

    m_messages = new QTextEdit;
    m_messages->setReadOnly(true);
    m_message = new QLineEdit;
    m_name = new QLabel("Your name");

    QPushButton* sendButton = new QPushButton("Send");
    connect(sendButton, SIGNAL(clicked()), this, SLOT(sendMessage()));
    connect(m_message, SIGNAL(returnPressed()), this, SLOT(sendMessage()));

    QPushButton* connectButton = new QPushButton("Enter to the chat");
    connect(connectButton, SIGNAL(clicked()), this, SLOT(connectToServer()));

    QHBoxLayout* hLayout = new QHBoxLayout;
    hLayout->addWidget(m_name);
    hLayout->addWidget(m_message, 3);
    hLayout->addWidget(sendButton);
    QVBoxLayout* vLayout = new QVBoxLayout;
    vLayout->addWidget(new QLabel("Chat"));
    vLayout->addWidget(m_messages);
    vLayout->addLayout(hLayout);
    vLayout->addWidget(connectButton);
    setLayout(vLayout);
}

void ChatClient::connectToServer()
{
    if(m_socket->state() == QTcpSocket::ConnectedState)
        return;

    StartDialog* startDialog = new StartDialog;
    if(startDialog->exec() == QDialog::Accepted)
    {
        m_socket = new QTcpSocket(this);
        m_socket->connectToHost(m_host, m_port);
        m_name->setText(startDialog->name());

        connect(m_socket, SIGNAL(readyRead()), this, SLOT(socketReadReady()));
        connect(m_socket, SIGNAL(connected()), this, SLOT(socketConnected()));
        connect(m_socket, SIGNAL(disconnected()), this, SLOT(socketDisconnected()));
        connect(m_socket, SIGNAL(errorOccurred(QAbstractSocket::SocketError)), this, SLOT(socketError(QAbstractSocket::SocketError)));
    }
    delete startDialog;
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

        QDataStream in(m_socket);
        in.setVersion(QDataStream::Qt_6_0);

        if(m_nextBlockSize == 0)
        {
            if(m_socket->bytesAvailable() < sizeof(quint16))
                return;
            in >> m_nextBlockSize;
        }

        if(m_socket->bytesAvailable() < m_nextBlockSize)
            return;

        quint8 command;
        in >> command;

        switch (command)
        {
        case message:
        {
            QTime time;
            QString msg;
            QString name;
            in >> time >> name >> msg;

            m_messages->append(time.toString() + " <" + name + "> " + msg);
            break;
        }
        }

        m_nextBlockSize = 0;
    }
}

void ChatClient::sendMessage()
{
    if((m_message->text().remove(' ') != "") && !m_message->text().isEmpty() && m_socket->state() == QAbstractSocket::ConnectedState)
    {
        QByteArray data;
        QDataStream out(&data, QIODevice::WriteOnly);
        out.setVersion(QDataStream::Qt_6_0);
        out << quint16(0) << quint8(message) << QTime::currentTime() << m_name->text() << m_message->text();

        out.device()->seek(0);
        out << quint16(data.size() - sizeof(quint16));

        m_socket->write(data);
        m_message->setText("");
    }
}

void ChatClient::socketError(QAbstractSocket::SocketError)
{
    QString strError = "Error: " + m_socket->errorString();
}
