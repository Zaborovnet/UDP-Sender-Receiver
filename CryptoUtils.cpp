// CryptoUtils.cpp

#include "CryptoUtils.h"
#include <iostream> // Добавлено для std::cerr
#include <ostream>
#include <cryptlib.h>
#include <osrng.h>
#include <aes.h>
#include <filters.h>
#include <hex.h>

using namespace CryptoPP;

void CryptoUtils::GenerateRSAKeys(std::string &publicKey, std::string &privateKey) {
    AutoSeededRandomPool rng;

    // Генерация ключей
    InvertibleRSAFunction params;
    params.GenerateRandomWithKeySize(rng, 2048);

    RSA::PrivateKey privateKeyObj(params);
    RSA::PublicKey publicKeyObj(params);

    // Создание BufferedTransformation
    StringSink privateKeySink(privateKey);
    StringSink publicKeySink(publicKey);

    // Сохранение ключей в строки
    privateKeyObj.DEREncode(privateKeySink);
    publicKeyObj.DEREncode(publicKeySink);
}

std::string CryptoUtils::AESEncrypt(const std::string &plainText, const std::string &key) {
    std::string cipherText;
    byte iv[AES::BLOCKSIZE]; // Вектор инициализации
    AutoSeededRandomPool rng;
    rng.GenerateBlock(iv, sizeof(iv)); // Генерация случайного IV

    // Шифрование
    try {
        // Создание шифра
        CBC_Mode<AES>::Encryption encryption;
        encryption.SetKeyWithIV((byte*)key.data(), key.size(), iv);

        // Формирование потока шифрования
        StringSink cipherTextSink(cipherText);
        cipherTextSink.Put(iv, sizeof(iv)); // Добавляем IV в начало

        // Создаем фильтр
        StreamTransformationFilter stfEncryptor(encryption, new Redirector(cipherTextSink));
        stfEncryptor.Put((byte*)plainText.data(), plainText.size());
        stfEncryptor.MessageEnd();
    }
    catch (const Exception& e) {
        std::cerr << "Error during AES encryption: " << e.what() << std::endl;
    }

    return cipherText;
}

std::string CryptoUtils::AESDecrypt(const std::string &cipherText, const std::string &key) {
    std::string decryptedText;

    // Извлечение IV из начала зашифрованного текста
    byte iv[AES::BLOCKSIZE];
    memcpy(iv, cipherText.data(), AES::BLOCKSIZE);

    try {
        // Дешифрование
        CBC_Mode<AES>::Decryption decryption;
        decryption.SetKeyWithIV((byte*)key.data(), key.size(), iv);

        // Формирование потока дешифрования
        StreamTransformationFilter stfDecryptor(decryption, new StringSink(decryptedText));
        stfDecryptor.Put((byte*)cipherText.data() + AES::BLOCKSIZE, cipherText.size() - AES::BLOCKSIZE);
        stfDecryptor.MessageEnd();
    }
    catch (const Exception& e) {
        std::cerr << "Error during AES decryption: " << e.what() << std::endl;
    }

    return decryptedText;
}

QByteArray CryptoUtils::AESEncrypt(const QByteArray &plainText, const std::string &key) {
    QByteArray cipherText;
    byte iv[AES::BLOCKSIZE]; // Вектор инициализации
    AutoSeededRandomPool rng;
    rng.GenerateBlock(iv, sizeof(iv)); // Генерация случайного IV

    // Шифрование
    try {
        // Создание шифра
        CBC_Mode<AES>::Encryption encryption;
        encryption.SetKeyWithIV((byte*)key.data(), key.size(), iv);

        // Добавляем IV в начало зашифрованного текста
        cipherText.append(reinterpret_cast<const char*>(iv), sizeof(iv));

        // Создаем фильтр
        CryptoPP::ArraySink arraySink((byte*)cipherText.data() + sizeof(iv), cipherText.size() - sizeof(iv));
        StreamTransformationFilter stfEncryptor(encryption, new Redirector(arraySink));
        stfEncryptor.Put(reinterpret_cast<const byte*>(plainText.data()), plainText.size());
        stfEncryptor.MessageEnd();
    }
    catch (const CryptoPP::Exception& e) {
        std::cerr << "Error during AES encryption: " << e.what() << std::endl;
    }

    return cipherText;
}




QByteArray CryptoUtils::AESDecrypt(const QByteArray &cipherText, const std::string &key) {
    QByteArray decryptedText;

    // Извлечение IV из начала зашифрованного текста
    byte iv[AES::BLOCKSIZE];
    memcpy(iv, cipherText.data(), AES::BLOCKSIZE);

    try {
        // Дешифрование
        CBC_Mode<AES>::Decryption decryption;
        decryption.SetKeyWithIV((byte*)key.data(), key.size(), iv);

        // Используем ArraySink для записи расшифрованных данных
        CryptoPP::ArraySink arraySink((byte*)decryptedText.data(), decryptedText.size());
        StreamTransformationFilter stfDecryptor(decryption, new Redirector(arraySink));
        stfDecryptor.Put(reinterpret_cast<const byte*>(cipherText.data()) + AES::BLOCKSIZE, cipherText.size() - AES::BLOCKSIZE);
        stfDecryptor.MessageEnd();
    }
    catch (const CryptoPP::Exception& e) {
        std::cerr << "Error during AES decryption: " << e.what() << std::endl;
    }

    return decryptedText;
}













std::string CryptoUtils::RSAEncrypt(const std::string &plainText, const RSA::PublicKey &publicKey) {
    std::string cipherText;
    AutoSeededRandomPool rng;

    try {
        RSAES_OAEP_SHA_Encryptor encryptor(publicKey);
        StringSource ss(plainText, true,
            new PK_EncryptorFilter(rng, encryptor,
                new StringSink(cipherText)
            )
        );
    }
    catch (const Exception &e) {
        std::cerr << "Error during RSA encryption: " << e.what() << std::endl;
    }

    return cipherText;
}

std::string CryptoUtils::RSADecrypt(const std::string &cipherText, const RSA::PrivateKey &privateKey) {
    std::string recoveredText;
    AutoSeededRandomPool rng;

    try {
        RSAES_OAEP_SHA_Decryptor decryptor(privateKey);
        StringSource ss(cipherText, true,
            new PK_DecryptorFilter(rng, decryptor,
                new StringSink(recoveredText)
            )
        );
    }
    catch (const Exception &e) {
        std::cerr << "Error during RSA decryption: " << e.what() << std::endl;
    }

    return recoveredText;
}
