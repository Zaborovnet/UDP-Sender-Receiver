#include "JpegCompressor.h"
#include <QDebug>

QByteArray JpegCompressor::compress(const QImage &image) {
    QByteArray byteArray;
    int width = image.width();
    int height = image.height();
    int numChannels = 3; // RGB

    // Выделяем буфер для изображения
    uint8_t *imageData = new uint8_t[width * height * numChannels];
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            QColor color = image.pixelColor(x, y);
            imageData[(y * width + x) * numChannels + 0] = color.red();
            imageData[(y * width + x) * numChannels + 1] = color.green();
            imageData[(y * width + x) * numChannels + 2] = color.blue();
        }
    }

    // Подготовка буфера для сжатого изображения
    int jpegSize = width * height * numChannels; // Предполагаем, что сжатое изображение не будет больше исходного
    byteArray.resize(jpegSize); // Устанавливаем размер буфера

    jpge::params compressionParams;
    compressionParams.m_quality = 90; // Установите качество сжатия

    // Сжимаем изображение в JPEG
    if (!jpge::compress_image_to_jpeg_file_in_memory(byteArray.data(), jpegSize, width, height, numChannels, imageData, compressionParams)) {
        qDebug() << "JPEG compression failed!";
        delete[] imageData;
        return QByteArray(); // Возвращаем пустой QByteArray в случае ошибки
    }

    delete[] imageData;
    byteArray.resize(jpegSize); // Устанавливаем фактический размер сжатого изображения
    return byteArray;
}


QImage JpegCompressor::decompress(const QByteArray &data) {
    int width, height, actualComps;
    unsigned char *decompressedData = jpgd::decompress_jpeg_image_from_memory(reinterpret_cast<const unsigned char*>(data.data()), data.size(), &width, &height, &actualComps, 3);

    if (!decompressedData) {
        qDebug() << "JPEG decompression failed!";
        return QImage(); // Возвращаем пустое изображение в случае ошибки
    }

    QImage image(width, height, QImage::Format_RGB888);
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int index = (y * width + x) * 3;
            QColor color(decompressedData[index], decompressedData[index + 1], decompressedData[index + 2]);
            image.setPixelColor(x, y, color);
        }
    }

    free(decompressedData);
    return image;
}

