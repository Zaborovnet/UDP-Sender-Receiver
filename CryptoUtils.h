// CryptoUtils.h

#ifndef CRYPTOUTILS_H
#define CRYPTOUTILS_H

#include <cryptlib.h>
#include <osrng.h>
#include <rsa.h>
#include <aes.h>
#include <modes.h>
#include <filters.h>
#include <hex.h>
#include <string>
#include <QByteArray>

class CryptoUtils {
public:
    // Генерация RSA ключей
    static void GenerateRSAKeys(std::string &publicKey, std::string &privateKey);

    // Шифрование данных с помощью AES
    static std::string AESEncrypt(const std::string &plainText, const std::string &key);

    // Дешифрование данных с помощью AES
    static std::string AESDecrypt(const std::string &cipherText, const std::string &key);

    // Шифрование данных с помощью AES
    static QByteArray AESEncrypt(const QByteArray &plainText, const std::string &key);

    // Дешифрование данных с помощью AES
    static QByteArray AESDecrypt(const QByteArray &plainText, const std::string &key);

    // Шифрование данных с помощью RSA
    static std::string RSAEncrypt(const std::string &plainText, const CryptoPP::RSA::PublicKey &publicKey);

    // Дешифрование данных с помощью RSA
    static std::string RSADecrypt(const std::string &cipherText, const CryptoPP::RSA::PrivateKey &privateKey);
};

#endif // CRYPTOUTILS_H
