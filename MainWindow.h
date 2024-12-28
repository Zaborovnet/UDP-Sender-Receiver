// MainWindow.h
// Курсовой проект сделан студентами Митюшиной Е. В. и Авдеевом Д. И.  М8О-205М-23

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QImage>
#include <QPushButton>
#include <QVBoxLayout>
#include <QFileDialog>
#include <QLabel>
#include <QLineEdit>
#include <QGroupBox>
#include <QTextEdit>
#include <QUdpSocket>
#include "ImageSender.h"
#include "ImageReceiver.h"
#include "UdpConnection.h"
#include "JpegCompressor.h"
#include <MinimalSocket/udp/UdpSocket.h>
#include <QComboBox>




class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void bindPort();  // Объявление метода bindPort
    void selectImage(); // Слот для выбора изображения
    void sendMessage(); // Слот для отправки текстового сообщения
    void onImageReceived(const QByteArray &data); // Слот для обработки полученного изображения
    void onTextMessageReceived(const QString &message); // Слот для обработки текстовых сообщений
    void copyIpToClipboard(); // Слот для копирования IP-адреса в буфер обмена
    void establishConnection(); // Слот для установления соединения
    void readPendingDatagrams(); // Слот для обработки входящих сообщений

private:
    UdpConnection *udpConnection;
    QString localIP;
    QUdpSocket *udpSocket; // UDP сокет для связи
    MinimalSocket::udp::Udp<true> *minimalSocket; // Указатель на MinimalSocket
    QComboBox *socketTypeComboBox; // Новый элемент для выбора типа сокета
    ImageSender *imageSender = nullptr; // Указатель на объект ImageSender
    ImageReceiver *imageReceiver = nullptr; // Указатель на объект ImageReceiver
    QLabel *statusLabel; // Метка для отображения статуса
    QLabel *sentImageLabel; // Метка для отображения отправленного изображения
    QLabel *receivedImageLabel; // Метка для отображения полученного изображения
    QLabel *localIpLabel; // Метка для отображения локального IP-адреса
    QLineEdit *messageInput; // Поле для ввода текстового сообщения
    QLineEdit *portLineEdit; // Поле для ввода своего порта
    QLineEdit *receiverIpLineEdit; // Поле для ввода IP получателя
    QLineEdit *receiverPortLineEdit; // Поле для ввода порта получателя
    QLabel *udpStatusLabel; // Метка для отображения статуса UDP-сокета
    QPushButton *connectButton; // Кнопка для установления соединения
    QLabel *sentImageDisplay; // QLabel для отображения отправленного изображения
    QLabel *receivedImageDisplay; // QLabel для отображения полученного изображения
    QTextEdit *receivedMessages; // Поле для отображения полученных текстовых сообщений
    QLabel *sentChecksumLabel; // Метка для отображения отправленных контрольных сумм
    QLabel *receivedChecksumLabel; // Метка для отображения полученных контрольных сумм

    bool isPortBound = false; // Флаг для отслеживания состояния биндинга порта
};

#endif // MAINWINDOW_H
