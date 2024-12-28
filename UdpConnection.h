// UdpConnection.h

#ifndef UDPCONNECTION_H
#define UDPCONNECTION_H

#include <QUdpSocket>
#include <QObject>
#include <QHostAddress>
#include <QByteArray>
#include <QDebug>
#include "MinimalSocket/udp/UdpSocket.h"

class UdpConnection : public QObject {
    Q_OBJECT

public:
    UdpConnection(QUdpSocket *udpSocket, MinimalSocket::udp::Udp<true> *minimalSocket = nullptr, QObject *parent = nullptr);
    ~UdpConnection();

    // Метод для инициализации соединения
    bool initialize(const QString &host, quint16 port);

    // Метод для отправки данных
    void sendData(const QByteArray &data);

    // Метод для получения данных
    QByteArray receiveData();

signals:
    void dataReceived(const QByteArray &data);

private slots:
    void onReadyRead();

private:
    MinimalSocket::udp::Udp<true> *minimalSocket; // Указатель на сокет MinimalSocket
    QUdpSocket *udpSocket; // Сокет для работы с UDP
    QHostAddress remoteHost; // Хост для отправки данных
    quint16 remotePort; // Порт для отправки данных
};

#endif // UDPCONNECTION_H
