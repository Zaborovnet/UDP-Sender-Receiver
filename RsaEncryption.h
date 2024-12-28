// RsaEncryption.h

#ifndef RSAENCRYPTION_H
#define RSAENCRYPTION_H

#include <QByteArray>
#include <QCryptographicHash>
#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QtCrypto>

class RsaEncryption {
public:
    RsaEncryption();
    ~RsaEncryption();

    // Генерация пары ключей
    void generateKeys(int keySize);

    // Шифрование данных
    QByteArray encrypt(const QByteArray &data, const QString &publicKeyFile);

    // Дешифрование данных
    QByteArray decrypt(const QByteArray &data, const QString &privateKeyFile);

private:
    QString publicKey;
    QString privateKey;
};

#endif // RSAENCRYPTION_H
