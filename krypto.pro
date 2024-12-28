QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# Курсовой проект сделан студентами Митюшиной Е. В. и Авдеевом Д. И.  М8О-205М-23

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    CryptoUtils.cpp \
    ImageReceiver.cpp \
    ImageSender.cpp \
    ImageTransfer.cpp \
    JpegCompressor.cpp \
    MainWindow.cpp \
    MinimalSocket/Error.cpp \
    MinimalSocket/SocketAddress.cpp \
    MinimalSocket/SocketFunctions.cpp \
    MinimalSocket/SocketHandler.cpp \
    MinimalSocket/core/Address.cpp \
    MinimalSocket/core/Definitions.cpp \
    MinimalSocket/core/Receiver.cpp \
    MinimalSocket/core/Sender.cpp \
    MinimalSocket/core/Socket.cpp \
    MinimalSocket/core/SocketContext.cpp \
    MinimalSocket/tcp/TcpClient.cpp \
    MinimalSocket/tcp/TcpServer.cpp \
    MinimalSocket/udp/UdpSocket.cpp \
    UdpConnection.cpp \
    jpeg/jpgd.cpp \
    jpeg/jpge.cpp \
    jpeg/tga2jpg.cpp \
    jpeg/timer.cpp \
    main.cpp

HEADERS += \
    CryptoUtils.h \
    ImageReceiver.h \
    ImageSender.h \
    ImageTransfer.h \
    JpegCompressor.h \
    MainWindow.h \
    MinimalSocket/Error.h \
    MinimalSocket/NonCopiable.h \
    MinimalSocket/SocketAddress.h \
    MinimalSocket/SocketFunctions.h \
    MinimalSocket/SocketHandler.h \
    MinimalSocket/Utils.h \
    MinimalSocket/core/Address.h \
    MinimalSocket/core/Definitions.h \
    MinimalSocket/core/Receiver.h \
    MinimalSocket/core/Sender.h \
    MinimalSocket/core/Socket.h \
    MinimalSocket/core/SocketContext.h \
    MinimalSocket/tcp/TcpClient.h \
    MinimalSocket/tcp/TcpServer.h \
    MinimalSocket/udp/UdpSocket.h \
    UdpConnection.h \
    jpeg/jpgd.h \
    jpeg/jpgd_idct.h \
    jpeg/jpge.h \
    jpeg/stb_image.h \
    jpeg/stb_image_write.h \
    jpeg/timer.h

FORMS += \
    MainWindow.ui

INCLUDEPATH += /usr/include/cryptopp
LIBS += -lcryptopp  # Добавлено для линковки с библиотекой Crypto++  sudo apt install libcrypto++-dev

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    jpeg/jpge.sln \
    jpeg/jpge.vcxproj \
    jpeg/jpge.vcxproj.filters
