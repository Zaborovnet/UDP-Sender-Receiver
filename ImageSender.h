// ImageSender.h

#ifndef IMAGESENDER_H
#define IMAGESENDER_H

#include "UdpConnection.h"
#include "ImageTransfer.h"
#include <QImage>
#include <QString>


class ImageSender : public QObject {
    Q_OBJECT

public:
    ImageSender(const QString &host, quint16 port, int packetSize, UdpConnection *udpconnection, QObject *parent = nullptr);
    ~ImageSender();

    void sendImage(const QImage &image);
    void sendTextMessage(const QString &message);
    void sendKey(const std::string &key); // Новый метод для отправки ключа


private:
    UdpConnection *udpConnection;
    ImageTransfer *imageTransfer;
    QString currentReceiverIp;
    quint16 currentReceiverPort;

signals:
    void sentChecksumUpdated(const QByteArray &checksum);
};

#endif // IMAGESENDER_H

