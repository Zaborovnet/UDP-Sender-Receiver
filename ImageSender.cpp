// ImageSender.cpp

#include "ImageSender.h"
#include "CryptoUtils.h"
#include "JpegCompressor.h"
#include <QThread>


ImageSender::ImageSender(const QString &host, quint16 port, int packetSize, UdpConnection *udpconnection, QObject *parent)
  : QObject(parent), currentReceiverIp(host), currentReceiverPort(port) {
    udpConnection = udpconnection;
    imageTransfer = new ImageTransfer(packetSize);

    // Инициализируем UDP-соединение
    if (!udpConnection->initialize(host, port)) {
        qDebug() << "Failed to initialize UDP connection.";
    }
}

ImageSender::~ImageSender() {
    delete udpConnection;
    delete imageTransfer;
}

void ImageSender::sendImage(const QImage &image) {
    if (image.isNull()) {
        qDebug() << "Error: Image is null!";
        return;
    }


    QByteArray encodedImage = JpegCompressor::compress(image);
    //QByteArray encodedImage = imageTransfer->encodeImageToJPEG(image);
    qDebug() << "encodedImage" << encodedImage.size();


    if (encodedImage.isEmpty()) {
        qDebug() << "Error: Encoded image is empty!";
        return;
    }

    // Шифрование изображения
    //QByteArray encryptedData = CryptoUtils::AESEncrypt(encodedImage.toStdString(), "1234567890123456").c_str();

    //QByteArray encryptedData = CryptoUtils::AESEncrypt(encodedImage, "1234567890123456");
    QByteArray encryptedData = encodedImage;

    // Вычисляем контрольную сумму для закодированного изображения
    //QByteArray checksum = imageTransfer->calculateCRC(encodedImage);

    // Разбиваем изображение на пакеты
    QVector<QByteArray> packets = imageTransfer->splitIntoPackets(encodedImage);

    // Отправляем количество пакетов
    QByteArray packetCountMessage = "PACKET_COUNT:" + QByteArray::number(packets.size());
    udpConnection->sendData(packetCountMessage);
    qDebug() << packets.size() << "количество пакетов";


    for (int i = 0; i < packets.size(); ++i) {
        QByteArray packetWithPrefix = "IMAGE:" + QByteArray::number(i) + ":" + packets[i];
        udpConnection->sendData(packetWithPrefix);
        QThread::msleep(100); // Задержка в 1 секунду между отправками
    }

    qDebug() << "Image sent successfully.";
}

void ImageSender::sendTextMessage(const QString &message) {
    // Шифруем сообщение
    std::string encryptedMessage = CryptoUtils::AESEncrypt(message.toStdString(), "1234567890123456");

    // Создаем QByteArray из зашифрованного сообщения с префиксом
    QByteArray messageToSend = "TEXT:" + QByteArray::fromStdString(encryptedMessage);

    // Отправляем зашифрованное сообщение через UDP
    udpConnection->sendData(messageToSend);
}

void ImageSender::sendKey(const std::string &key) {
    QByteArray keyMessage = "KEY:" + QByteArray::fromStdString(key);
    udpConnection->sendData(keyMessage);
}


