// ImageTransfer.h

#ifndef IMAGETRANSFER_H
#define IMAGETRANSFER_H

#include <QImage>
#include <QByteArray>
#include <QVector>
#include <QCryptographicHash>
#include <QDebug>

class ImageTransfer {
public:
    ImageTransfer(int packetSize);

    // Метод для кодирования изображения в JPEG
    QByteArray encodeImageToJPEG(const QImage &image);

    // Метод для разбиения закодированного изображения на пакеты
    QVector<QByteArray> splitIntoPackets(const QByteArray &data);

    // Метод для вычисления контрольной суммы CRC
    QByteArray calculateCRC(const QByteArray &data);

private:
    int packetSize; // Размер пакета
};

#endif // IMAGETRANSFER_H

