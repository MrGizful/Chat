#pragma once
#include <QtWidgets>
#include <QTcpSocket>
#include <QMessageBox>

class ChatClient : public QWidget
{
    Q_OBJECT

private:
    QTcpSocket* m_socket;
    QTextEdit* m_messages;
    QLineEdit* m_message;

    QByteArray data;

public:
    ChatClient(const QString& host = "localhost", int port = 2323, QWidget *parent = nullptr);

public slots:
    void socketReadReady();
    void socketConnected();
    void socketDisconnected();
    void socketError(QAbstractSocket::SocketError error);
};
