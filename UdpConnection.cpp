// UdpConnection.cpp

#include "UdpConnection.h"

UdpConnection::UdpConnection(QUdpSocket *udpSocket, MinimalSocket::udp::Udp<true> *minimalSocket, QObject *parent)
  : QObject(parent), minimalSocket(minimalSocket), udpSocket(udpSocket) {
  if (udpSocket) {
    connect(udpSocket, &QUdpSocket::readyRead, this, &UdpConnection::onReadyRead);
  }
}

UdpConnection::~UdpConnection() {
    delete udpSocket;
    delete minimalSocket;

}

bool UdpConnection::initialize(const QString &host, quint16 port) {
    remoteHost = QHostAddress(host);
    remotePort = port;

    if (minimalSocket) {
    return minimalSocket->open(); // Открываем сокет MinimalSocket
    }
    return true;
}






void UdpConnection::sendData(const QByteArray &data) {
    if (udpSocket) {
      if (udpSocket->writeDatagram(data, remoteHost, remotePort) == -1) {
          qDebug() << "Failed to send data:" << udpSocket->errorString();
          return; // Добавьте возврат, чтобы избежать дальнейших действий
      }
      qDebug() << "Data sent:" << data;
    } else if (minimalSocket) {
      minimalSocket->sendTo(data.toStdString(), MinimalSocket::Address{remoteHost.toString().toStdString(), remotePort});
    }

}


void UdpConnection::onReadyRead() {
    while (udpSocket->hasPendingDatagrams()) {
        QByteArray buffer;
        buffer.resize(udpSocket->pendingDatagramSize());
        QHostAddress sender;
        quint16 senderPort;

        udpSocket->readDatagram(buffer.data(), buffer.size(), &sender, &senderPort);
        qDebug() << "Data received from" << sender.toString() << ":" << senderPort << "Data:" << buffer;

        emit dataReceived(buffer); // Генерируем сигнал о получении данных
    }

    // Для MinimalSocket
    if (minimalSocket) {
        auto received_message = minimalSocket->receive(1024);
        if (received_message) {
          emit dataReceived(QByteArray(received_message->received_message.data(), received_message->received_message.size()));
        }
    }
}


QByteArray UdpConnection::receiveData() {
    // Здесь мы можем использовать сигнал dataReceived, чтобы вернуть данные
    QByteArray buffer;
    if (udpSocket->hasPendingDatagrams()) {
        buffer.resize(udpSocket->pendingDatagramSize());
        QHostAddress sender;
        quint16 senderPort;
        udpSocket->readDatagram(buffer.data(), buffer.size(), &sender, &senderPort);
    }
    return buffer; // Возвращаем полученные данные
}
