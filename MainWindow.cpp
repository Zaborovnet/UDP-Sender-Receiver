// MainWindow.cpp

#include "MainWindow.h"
#include <QHostInfo>
#include <QNetworkInterface>
#include <QStyleFactory>
#include <QClipboard>
#include <QApplication>
#include <QFormLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QMessageBox>
#include "CryptoUtils.h"
#include "JpegCompressor.h"


MainWindow::MainWindow(QWidget *parent)
  : QMainWindow(parent), udpSocket(new QUdpSocket(this)), minimalSocket (nullptr) {
    // Устанавливаем стиль приложения
    setStyle(QStyleFactory::create("Fusion"));

    // Создаем центральный виджет и устанавливаем компоновку
    QWidget *centralWidget = new QWidget(this);
    QGridLayout *mainLayout = new QGridLayout(centralWidget);
    mainLayout->setSpacing(10);
    mainLayout->setContentsMargins(10, 10, 10, 10);

    // Создаем QComboBox для выбора типа сокета
    socketTypeComboBox = new QComboBox(this);
    socketTypeComboBox->addItem("QUdpSocket");
    socketTypeComboBox->addItem("MinimalSocket");
    mainLayout->addWidget(socketTypeComboBox, 0, 2); // Добавляем его в компоновку

    // Группа для ввода данных получателя
    QGroupBox *receiverGroup = new QGroupBox("Receiver Information", this);
    QVBoxLayout *receiverLayout = new QVBoxLayout(receiverGroup);

    // Поле для ввода IP-адреса получателя
    QLabel *receiverIpLabel = new QLabel("Receiver IP:", this);
    receiverIpLineEdit = new QLineEdit(this);
    receiverLayout->addWidget(receiverIpLabel);
    receiverLayout->addWidget(receiverIpLineEdit);

    // Поле для ввода порта получателя
    QLabel *receiverPortLabel = new QLabel("Receiver Port:", this);
    receiverPortLineEdit = new QLineEdit(this);
    receiverPortLineEdit->setPlaceholderText("Enter receiver port");
    receiverLayout->addWidget(receiverPortLabel);
    receiverLayout->addWidget(receiverPortLineEdit);

    mainLayout->addWidget(receiverGroup, 0, 0, 1, 2); // Добавляем группу в первую строку

    // Группа для ввода своего порта
    QGroupBox *localPortGroup = new QGroupBox("Local Port", this);
    QVBoxLayout *localPortLayout = new QVBoxLayout(localPortGroup);

    QLabel *localPortLabel = new QLabel("Your Port:", this);
    portLineEdit = new QLineEdit(this);
    portLineEdit->setPlaceholderText("Enter your port");
    localPortLayout->addWidget(localPortLabel);
    localPortLayout->addWidget(portLineEdit);

    QPushButton *bindPortButton = new QPushButton("Bind Port", this);
    localPortLayout->addWidget(bindPortButton);
    mainLayout->addWidget(localPortGroup, 1, 0, 1, 2); // Добавляем группу в следующую строку

    // Подключаем сигнал для кнопки биндинга порта
    connect(bindPortButton, &QPushButton::clicked, this, &MainWindow::bindPort);

    // Кнопка для установления соединения
    connectButton = new QPushButton("Establish Connection", this);
    mainLayout->addWidget(connectButton, 2, 0); // Кнопка занимает одну ячейку
    connect(connectButton, &QPushButton::clicked, this, &MainWindow::establishConnection);

    // Кнопка для выбора изображения
    QPushButton *selectImageButton = new QPushButton("Select Image", this);
    mainLayout->addWidget(selectImageButton, 2, 1); // Кнопка занимает одну ячейку
    connect(selectImageButton, &QPushButton::clicked, this, &MainWindow::selectImage);

    // Поле для ввода текстового сообщения
    QLabel *messageLabel = new QLabel("Message:", this);
    mainLayout->addWidget(messageLabel, 3, 0); // Метка занимает одну ячейку
    messageInput = new QLineEdit(this);
    mainLayout->addWidget(messageInput, 3, 1); // Поле для ввода в той же строке

    // Кнопка для отправки текстового сообщения
    QPushButton *sendMessageButton = new QPushButton("Send Message", this);
    mainLayout->addWidget(sendMessageButton, 4, 1); // Кнопка занимает одну ячейку
    connect(sendMessageButton, &QPushButton::clicked, this, &MainWindow::sendMessage);

    // Метка для отображения статуса
    statusLabel = new QLabel("Status: Ready", this);
    mainLayout->addWidget(statusLabel, 5, 0, 1, 2); // Статус занимает две ячейки

    // Метка для отображения статуса UDP-сокета
    udpStatusLabel = new QLabel("UDP Socket Status: Not Connected", this);
    mainLayout->addWidget(udpStatusLabel, 6, 0, 1, 2); // Статус UDP-сокета занимает две ячейки

    // Метка для отображения отправленного изображения
    sentImageLabel = new QLabel("Sent Image: None", this);
    mainLayout->addWidget(sentImageLabel, 7, 0); // Метка занимает одну ячейку
    sentImageDisplay = new QLabel(this);
    sentImageDisplay->setScaledContents(true);
    sentImageDisplay->setFixedSize(150, 150); // Установите размер для отображения
    mainLayout->addWidget(sentImageDisplay, 7, 1); // Отображение занимает одну ячейку

    // Метка для отображения полученного изображения
    receivedImageLabel = new QLabel("Received Image: None", this);
    mainLayout->addWidget(receivedImageLabel, 8, 0); // Метка занимает одну ячейку
    receivedImageDisplay = new QLabel(this);
    receivedImageDisplay->setScaledContents(true);
    receivedImageDisplay->setFixedSize(150, 150); // Установите размер для отображения
    mainLayout->addWidget(receivedImageDisplay, 8, 1); // Отображение занимает одну ячейку

    // Получение локального IP-адреса
    foreach (const QHostAddress &address, QNetworkInterface::allAddresses()) {
        if (address != QHostAddress::LocalHost && address.toIPv4Address() != 0) {
            localIP = address.toString();
            break; // Выходим из цикла, как только нашли подходящий адрес
        }
    }

    // Метка для отображения локального IP-адреса
    localIpLabel = new QLabel("Local IP: " + localIP, this);
    mainLayout->addWidget(localIpLabel, 9, 0, 1, 2); // Метка занимает две ячейки

    // Поле для отображения полученных текстовых сообщений
    receivedMessages = new QTextEdit(this);
    receivedMessages->setReadOnly(true); // Делаем поле только для чтения
    mainLayout->addWidget(receivedMessages, 10, 0, 1, 2); // Поле занимает две ячейки

    // Метки для отображения контрольных сумм
    sentChecksumLabel = new QLabel("Sent Checksum: None", this);
    mainLayout->addWidget(sentChecksumLabel, 11, 0); // Метка занимает одну ячейку
    receivedChecksumLabel = new QLabel("Received Checksum: None", this);
    mainLayout->addWidget(receivedChecksumLabel, 11, 1); // Метка занимает одну ячейку

    // Кнопка для копирования IP
    QPushButton *copyIpButton = new QPushButton("Copy Local IP", this);
    mainLayout->addWidget(copyIpButton, 12, 0, 1, 2); // Добавляем кнопку для копирования IP
    connect(copyIpButton, &QPushButton::clicked, this, &MainWindow::copyIpToClipboard);

    // Устанавливаем центральный виджет
    setCentralWidget(centralWidget);
    setWindowTitle("UDP Image and Message Sender and Receiver");
}

MainWindow::~MainWindow() {
    delete imageSender;
    delete imageReceiver;
}

void MainWindow::bindPort() {
    quint16 localPort = portLineEdit->text().toUInt();
    if (localPort == 0) {
        statusLabel->setText("Error: Please enter a valid local port.");
        return;
    }

    if (isPortBound) {
        statusLabel->setText("Port is already bound.");
        return; // Если порт уже связан, выходим
    }

    // Инициализация UDP-сокета и привязка к указанному порту
    if (udpSocket->bind(QHostAddress::Any, localPort)) {
        udpStatusLabel->setText("UDP Socket Status: Listening on port " + QString::number(localPort));
        //connect(udpSocket, &QUdpSocket::readyRead, this, &MainWindow::readPendingDatagrams);
        isPortBound = true; // Устанавливаем флаг после успешного биндинга
        udpConnection = new UdpConnection(udpSocket);
        udpConnection->initialize(localIP, localPort);

    }
    else {
        statusLabel->setText("Error: Unable to bind to port " + QString::number(localPort) + ": " + udpSocket->errorString());
    }
}

void MainWindow::sendMessage() {
    QString message = messageInput->text();
    if (!message.isEmpty()) {
        imageSender->sendTextMessage(message); // Отправляем текстовое сообщение
        statusLabel->setText("Status: Message sent.");
        messageInput->clear(); // Очищаем поле ввода
    } else {
        statusLabel->setText("Status: Message is empty."); // Уведомляем, если сообщение пустое
    }
}

void MainWindow::onImageReceived(const QByteArray &data) {
    // qDebug() << "Received data size:" << data.size();
    // statusLabel->setText("Status: Image received.");

    // // Получаем контрольную сумму
    // QByteArray receivedChecksum = data.right(64); // Предполагаем, что контрольная сумма 64 байта
    // receivedChecksumLabel->setText("Received Checksum: " + receivedChecksum.toHex()); // Обновляем метку

    // // Здесь вы должны обработать данные и создать изображение
    // QByteArray imageData = data.left(data.size() - 64); // Убираем контрольную сумму

    QImage image = JpegCompressor::decompress(data);
    if (!image.isNull()) {
        receivedImageLabel->setText("Received Image: Image Data"); // Обновляем метку
        receivedImageDisplay->setPixmap(QPixmap::fromImage(image)); // Отображаем полученное изображение
    } else {
        receivedImageLabel->setText("Received Image: Failed to load.");
    }
}

void MainWindow::copyIpToClipboard() {
    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(localIpLabel->text().split("Local IP: ").last());
    statusLabel->setText("Local IP copied to clipboard!");
}

void MainWindow::readPendingDatagrams() {
    // while (udpSocket->hasPendingDatagrams()) {
    //     QByteArray datagram;
    //     datagram.resize(udpSocket->pendingDatagramSize());
    //     QHostAddress sender;
    //     quint16 senderPort;

    //     udpSocket->readDatagram(datagram.data(), datagram.size(), &sender, &senderPort);
    //     qDebug() << "Received datagram from" << sender.toString() << ":" << senderPort << "Data:" << datagram;



//        // Обработка полученных данных
//        if (datagram.startsWith("KEY:")) {
//            // Обработка ключа
//            emit onTextMessageReceived("Received Key: " + QString(datagram));
//        } else if (datagram.startsWith("TEXT:")) {
//            // Обработка тёекстового сообщения
//            emit onTextMessageReceived(QString(datagram));
//        } else if (datagram.startsWith("IMAGE:")) {
//            // Обработка изображения
//            emit onImageReceived(datagram);
//        } else {
//            qDebug() << "Unknown data type received.";
//        }
    //}
}


void MainWindow::establishConnection() {
    QString receiverIp = receiverIpLineEdit->text();
    quint16 receiverPort = receiverPortLineEdit->text().toUInt();

    // Проверка на корректность введенного IP и порта
    if (receiverIp.isEmpty() || receiverPort == 0) {
        statusLabel->setText("Error: Please enter a valid IP and port.");
        return;
    }

    // Генерация ключей RSA
    std::string publicKey, privateKey;
    CryptoUtils::GenerateRSAKeys(publicKey, privateKey);

    // Создаем объекты после успешной привязки
    imageReceiver = new ImageReceiver(2048, udpConnection, this);
    imageSender = new ImageSender(receiverIp, receiverPort, 2048 ,udpConnection, this);

    // Отправляем публичный ключ
    imageSender->sendKey(publicKey);


    connect(imageReceiver, &ImageReceiver::textMessageReceived, this, &MainWindow::onTextMessageReceived);
    connect(imageReceiver, &ImageReceiver::receivedImage, this, &MainWindow::onImageReceived);


    // Подключаем сигналы после инициализации
    connect(imageSender, &ImageSender::sentChecksumUpdated, this, [this](const QByteArray &checksum) {
        sentChecksumLabel->setText("Sent Checksum: " + checksum.toHex());
    });

    connect(imageReceiver, &ImageReceiver::receivedChecksumUpdated, this, [this](const QByteArray &checksum) {
        receivedChecksumLabel->setText("Received Checksum: " + checksum.toHex());
    });

    statusLabel->setText("Connection established with " + receiverIp + ":" + QString::number(receiverPort));
}

void MainWindow::selectImage() {
    QString fileName = QFileDialog::getOpenFileName(this, "Open Image", "", "Images (*.png *.jpg *.jpeg *.bmp)");
    if (!fileName.isEmpty()) {
        QImage image(fileName);
        if (!image.isNull()) {
            sentImageLabel->setText("Sent Image: " + fileName); // Обновляем метку
            sentImageDisplay->setPixmap(QPixmap::fromImage(image)); // Отображаем отправленное изображение
            statusLabel->setText("Status: Sending image...");
            imageSender->sendImage(image); // Отправляем изображение
        } else {
            statusLabel->setText("Status: Failed to load image.");
        }
    }
}

// Метод для обработки текстовых сообщений
void MainWindow::onTextMessageReceived(const QString &message) {
    statusLabel->setText("Received Message: " + message); // Обновляем статус с текстом сообщения
    receivedMessages->append("Received: " + message); // Добавляем полученное сообщение в текстовое поле
}

//10.128.18.45
