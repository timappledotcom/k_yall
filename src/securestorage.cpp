#include "securestorage.h"
#include <QCoreApplication>
#include <QStandardPaths>
#include <QDir>
#include <QSysInfo>
#include <QUuid>
#include <QRandomGenerator>
#include <QDebug>

const QString SecureStorage::SECURE_PREFIX = "secure_";

SecureStorage::SecureStorage()
    : m_settings(new QSettings())
{
    // Generate a consistent encryption key for this system/user
    m_encryptionKey = generateEncryptionKey();
}

QByteArray SecureStorage::generateEncryptionKey()
{
    // Create a deterministic key based on system and user information
    // This ensures the same key is generated each time for the same user/system
    
    QString keyData;
    
    // Add system-specific information
    keyData += QSysInfo::machineUniqueId();
    keyData += QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
    keyData += QCoreApplication::applicationName();
    keyData += QCoreApplication::organizationName();
    
    // If we don't have enough entropy, add application-specific data
    if (keyData.length() < 32) {
        keyData += "kyall_secure_storage_v1";
        keyData += QString::number(QCoreApplication::applicationPid());
    }
    
    // Hash the combined data to create a 256-bit key
    QByteArray key = QCryptographicHash::hash(keyData.toUtf8(), QCryptographicHash::Sha256);
    
    qDebug() << "SecureStorage: Generated encryption key (length:" << key.length() << ")";
    return key;
}

QByteArray SecureStorage::encrypt(const QString &plaintext, const QByteArray &key)
{
    if (plaintext.isEmpty() || key.isEmpty()) {
        return QByteArray();
    }
    
    QByteArray data = plaintext.toUtf8();
    QByteArray result;
    result.reserve(data.size());
    
    // Simple XOR cipher with key cycling
    // For production use, consider using QCA or OpenSSL for proper AES
    for (int i = 0; i < data.size(); ++i) {
        char encrypted = data[i] ^ key[i % key.size()];
        // Add some obfuscation by XORing with position
        encrypted ^= static_cast<char>(i % 256);
        result.append(encrypted);
    }
    
    // Base64 encode for safe storage in QSettings
    return result.toBase64();
}

QByteArray SecureStorage::decrypt(const QByteArray &ciphertext, const QByteArray &key)
{
    if (ciphertext.isEmpty() || key.isEmpty()) {
        return QByteArray();
    }
    
    // Decode from Base64
    QByteArray data = QByteArray::fromBase64(ciphertext);
    QByteArray result;
    result.reserve(data.size());
    
    // Reverse the XOR encryption
    for (int i = 0; i < data.size(); ++i) {
        char decrypted = data[i];
        // Reverse position obfuscation
        decrypted ^= static_cast<char>(i % 256);
        // Reverse key XOR
        decrypted ^= key[i % key.size()];
        result.append(decrypted);
    }
    
    return result;
}

void SecureStorage::storeSecure(const QString &key, const QString &value)
{
    if (key.isEmpty()) {
        qWarning() << "SecureStorage: Cannot store with empty key";
        return;
    }
    
    if (value.isEmpty()) {
        // Store empty values as empty strings, but still encrypted
        m_settings->setValue(SECURE_PREFIX + key, QString());
        return;
    }
    
    QByteArray encrypted = encrypt(value, m_encryptionKey);
    if (encrypted.isEmpty()) {
        qWarning() << "SecureStorage: Failed to encrypt value for key:" << key;
        return;
    }
    
    m_settings->setValue(SECURE_PREFIX + key, QString::fromLatin1(encrypted));
    m_settings->sync();
    
    qDebug() << "SecureStorage: Stored encrypted value for key:" << key;
}

QString SecureStorage::retrieveSecure(const QString &key)
{
    if (key.isEmpty()) {
        qWarning() << "SecureStorage: Cannot retrieve with empty key";
        return QString();
    }
    
    QString encryptedValue = m_settings->value(SECURE_PREFIX + key).toString();
    if (encryptedValue.isEmpty()) {
        return QString(); // Key doesn't exist or is empty
    }
    
    QByteArray decrypted = decrypt(encryptedValue.toLatin1(), m_encryptionKey);
    if (decrypted.isEmpty()) {
        qWarning() << "SecureStorage: Failed to decrypt value for key:" << key;
        return QString();
    }
    
    return QString::fromUtf8(decrypted);
}

void SecureStorage::removeSecure(const QString &key)
{
    if (key.isEmpty()) {
        return;
    }
    
    m_settings->remove(SECURE_PREFIX + key);
    m_settings->sync();
    
    qDebug() << "SecureStorage: Removed key:" << key;
}

bool SecureStorage::hasSecure(const QString &key)
{
    if (key.isEmpty()) {
        return false;
    }
    
    return m_settings->contains(SECURE_PREFIX + key);
}

void SecureStorage::clearAll()
{
    m_settings->beginGroup(SECURE_PREFIX);
    m_settings->remove("");
    m_settings->endGroup();
    m_settings->sync();
    
    qDebug() << "SecureStorage: Cleared all secure storage";
}
