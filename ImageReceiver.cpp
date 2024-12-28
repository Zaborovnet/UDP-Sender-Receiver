// ImageReceiver.cpp

#include "ImageReceiver.h"
#include "CryptoUtils.h"
#include "JpegCompressor.h"

ImageReceiver::ImageReceiver(int packetSize, UdpConnection *udpconnection, QWidget *parent)
: QWidget(parent), packets() {
    udpConnection = udpconnection;
    imageTransfer = new ImageTransfer(packetSize);

    // Подключаем сигнал получения данных к слоту
    connect(udpConnection, &UdpConnection::dataReceived, this, &ImageReceiver::onDataReceived);

    // Создаем метку для отображения изображения
    imageLabel = new QLabel(this);
    imageLabel->setScaledContents(true);
    setWindowTitle("Image Receiver");
    resize(800, 600);
}

bool ImageReceiver::initialize(const QString &host, quint16 port) {
    // Инициализируем UDP-соединение на указанном хосте и порту
    if (!udpConnection->initialize(host, port)) {
        qDebug() << "Failed to initialize UDP connection.";
        return false;
    }
    return true;
}

ImageReceiver::~ImageReceiver() {
    delete imageTransfer;
}

void ImageReceiver::onDataReceived(const QByteArray &data) {
    if (data.startsWith("KEY:")) {
        handleKeyMessage(data);
        return;
    }

    if (data.startsWith("TEXT:")) {
        handleTextMessage(data.mid(5)); // Извлекаем текст, отбрасывая префикс
        return;
    }

    if (data.startsWith("PACKET_COUNT:")) {
        bool ok;
        int expectedPacketCount = data.mid(13).toInt(&ok); // Извлекаем количество пакетов
        if (ok) {
            expectedPacketsCount = expectedPacketCount; // Сохраняем ожидаемое количество пакетов
            packets.clear(); // Очищаем вектор перед началом новой передачи
            packets.resize(expectedPacketsCount); // Инициализация вектора с размером
            qDebug() << "Expecting" << expectedPacketsCount << "packets.";
        }
        return;
    }

    if (data.startsWith("IMAGE:")) {
        QByteArray imageData = data.mid(6); // Извлекаем данные
        int index = imageData.indexOf(':'); // Находим разделитель
        if (index != -1) {
            int packetIndex = imageData.left(index).toInt(); // Извлекаем индекс
            imageData = imageData.mid(index + 1); // Извлекаем данные после индекса

            // Проверка контрольной суммы
            if (verifyCRC(imageData)) {
                // Убедимся, что вектор достаточно велик
                if (packetIndex >= 0 && packetIndex < expectedPacketsCount) {
                    packets[packetIndex] = imageData; // Сохраняем пакет
                    qDebug() << "Received valid image packet of size:" << imageData.size() << "at index:" << packetIndex << "in all packets" << expectedPacketsCount;

                    // Проверяем, если все пакеты получены
                    if (std::all_of(packets.begin(), packets.end(), [](const QByteArray &packet) { return !packet.isEmpty(); })) {
                        qDebug() << "All packets received, assembling image.";
                        assembleImage();
                    }
                } else {
                    qDebug() << "Received packet index out of range:" << packetIndex;
                }
            } else {
                qDebug() << "Received invalid image packet, CRC check failed.";
            }
        }
    }
}

void ImageReceiver::assembleImage() {
    // Проверяем, что все пакеты получены
    if (packets.size() != expectedPacketsCount) {
        qDebug() << "Not all packets received! Expected:" << expectedPacketsCount << "Received:" << packets.size();
        return; // Проверка на количество пакетов
    }

    // Собираем данные изображения из пакетов
    for (const QByteArray &packet : packets) {
        // Собираем и откидываем контрольную сумму
        currentImageData.append(packet.left(packet.size() - 64));
    }

    // Декодируем данные изображения
    QImage image = JpegCompressor::decompress(currentImageData);
    if (!image.isNull()) {
        emit receivedImage(currentImageData);
    } else {
        qDebug() << "Failed to decode image from received data.";
    }

    // Очищаем пакеты для следующего изображения
    packets.clear();
    currentImageData.clear();
}


void ImageReceiver::handleTextMessage(const QByteArray &data) {
    QString decryptedMessage = QString::fromStdString(CryptoUtils::AESDecrypt(data.toStdString(), "1234567890123456"));
    emit textMessageReceived(decryptedMessage); // Испускаем сигнал о получении текстового сообщения
    expectedPacketsCount = 0;
}

bool ImageReceiver::verifyCRC(const QByteArray &data) {
    QByteArray receivedData = data.left(data.size() - 64); // Предполагаем, что контрольная сумма 64 байта
    QByteArray receivedCRC = data.right(64); // Получаем контрольную сумму

    // Вычисляем контрольную сумму для полученных данных
    QByteArray calculatedCRC = QCryptographicHash::hash(receivedData, QCryptographicHash::Sha256).toHex();

    // Логируем вычисленную контрольную сумму
    qDebug() << "Calculated Checksum: " << calculatedCRC;
    return (receivedCRC == calculatedCRC);
}


void ImageReceiver::handleKeyMessage(const QByteArray &data) {
    std::string key = data.mid(4).toStdString(); // Получаем ключ, отбрасывая префикс "KEY:"
    emit receivedKey(key); // Испускаем сигнал с ключом
}





