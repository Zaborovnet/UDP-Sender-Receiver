// ImageTransfer.cpp

#include "ImageTransfer.h"
#include <QBuffer>

ImageTransfer::ImageTransfer(int packetSize) : packetSize(packetSize) {}

QByteArray ImageTransfer::encodeImageToJPEG(const QImage &image) {
    QByteArray byteArray;
    QBuffer buffer(&byteArray);
    buffer.open(QIODevice::WriteOnly);
    if (image.save(&buffer, "JPEG")) {
        qDebug() << "Image encoded to JPEG successfully.";
    } else {
        qDebug() << "Failed to encode image to JPEG.";
    }
    return byteArray;
}

QVector<QByteArray> ImageTransfer::splitIntoPackets(const QByteArray &data) {
    QVector<QByteArray> packets;
    int totalSize = data.size();
    int offset = 0;

    while (offset < totalSize) {
        int size = qMin(packetSize, totalSize - offset);
        QByteArray packet = data.mid(offset, size);
        QByteArray crc = calculateCRC(packet);
        packet.append(crc); // Добавляем контрольную сумму к пакету
        //qDebug() << "СalculateCRC packet" << crc;
        packets.append(packet);
        offset += size;
    }

    qDebug() << "Data split into" << packets.size() << "packets.";
    return packets;
}

QByteArray ImageTransfer::calculateCRC(const QByteArray &data) {
    QByteArray crc = QCryptographicHash::hash(data, QCryptographicHash::Sha256).toHex();
    return crc;
}




