#ifndef JPEGCOMPRESSOR_H
#define JPEGCOMPRESSOR_H

#include <QImage>
#include <QByteArray>
#include <jpeg/jpge.h>
#include <jpeg/jpgd.h>

class JpegCompressor {
public:
    static QByteArray compress(const QImage &image);
    static QImage decompress(const QByteArray &data);
};

#endif // JPEGCOMPRESSOR_H
