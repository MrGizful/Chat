#include "ChatClient.h"

ChatClient::ChatClient(const QString& host, int port, QWidget *parent)
    : QWidget(parent), m_nextBlockSize(0)
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
    connect(sendButton, SIGNAL(clicked()), this, SLOT(sendToServer()));
    connect(m_message, SIGNAL(returnPressed()), this, SLOT(sendToServer()));

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

        QDataStream in(m_socket);
        in.setVersion(QDataStream::Qt_6_0);

        while(true)
        {
            if(!m_nextBlockSize)
            {
                if(m_socket->bytesAvailable() < sizeof(quint16))
                    break;
                in >> m_nextBlockSize;
            }

            if(m_socket->bytesAvailable() < m_nextBlockSize)
                break;

            QTime time;
            QString msg;
            in >> time >> msg;

            m_messages->append(time.toString() + " " + msg);
            m_nextBlockSize = 0;
        }
    }
}

void ChatClient::sendToServer()
{
    if((m_message->text().remove(' ') != "") || !m_message->text().isEmpty())
    {
        QByteArray data;
        QDataStream out(&data, QIODevice::WriteOnly);
        out.setVersion(QDataStream::Qt_6_0);
        out << quint16(0) << QTime::currentTime() << m_message->text();

        out.device()->seek(0);
        out << quint16(data.size() - sizeof(quint16));

        m_socket->write(data);
        m_message->setText("");
    }
}

void ChatClient::socketError(QAbstractSocket::SocketError error)
{
    QString strError = "Error: " + QString(m_socket->errorString());
}
