// ImageReceiver.h

#ifndef IMAGERECEIVER_H
#define IMAGERECEIVER_H

#include "UdpConnection.h"
#include "ImageTransfer.h"
#include <QImage>
#include <QVector>
#include <QByteArray>
#include <QWidget>
#include <QLabel>
#include <QCryptographicHash>
#include <rsa.h>

class ImageReceiver : public QWidget {
    Q_OBJECT

public:
    ImageReceiver(int packetSize, UdpConnection *udpconnection, QWidget *parent = nullptr);
    ~ImageReceiver();
    bool initialize(const QString &host, quint16 port);

signals:
    void dataReceived(const QByteArray &data);
    void textMessageReceived(const QString &message);
    void receivedChecksumUpdated(const QByteArray &checksum);
    void receivedKey(const std::string &key); // Новый сигнал для получения ключа
    void receivedImage(const QByteArray &data); // Новый сигнал для получения ключа

private slots:

public slots:
    void onDataReceived(const QByteArray &data);


private:
    UdpConnection *udpConnection;
    ImageTransfer *imageTransfer;
    QVector<QByteArray> packets;
    QLabel *imageLabel;
    QByteArray currentImageData;

    void assembleImage();
    bool verifyCRC(const QByteArray &data);
    void handleTextMessage(const QByteArray &data);
    void handleKeyMessage(const QByteArray &data); // Новый метод для обработки ключа
    int expectedPacketsCount = 0; // Ожидаемое количество пакетов
};

#endif // IMAGERECEIVER_H
