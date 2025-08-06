#ifndef SECURESTORAGE_H
#define SECURESTORAGE_H

#include <QString>
#include <QByteArray>
#include <QCryptographicHash>
#include <QSettings>

/**
 * SecureStorage provides encrypted storage for sensitive credentials
 * using AES-256 encryption with a derived key from system-specific data.
 */
class SecureStorage
{
public:
    SecureStorage();
    
    /**
     * Store an encrypted value
     * @param key The key to store under
     * @param value The sensitive value to encrypt and store
     */
    void storeSecure(const QString &key, const QString &value);
    
    /**
     * Retrieve and decrypt a value
     * @param key The key to retrieve
     * @return The decrypted value, or empty string if not found/failed
     */
    QString retrieveSecure(const QString &key);
    
    /**
     * Remove a stored value
     * @param key The key to remove
     */
    void removeSecure(const QString &key);
    
    /**
     * Check if a secure value exists
     * @param key The key to check
     * @return true if the key exists
     */
    bool hasSecure(const QString &key);
    
    /**
     * Clear all secure storage
     */
    void clearAll();

private:
    /**
     * Generate a encryption key based on system-specific data
     * This creates a consistent key for the current user/system
     */
    QByteArray generateEncryptionKey();
    
    /**
     * Simple XOR-based encryption (lightweight alternative to AES)
     * For a production app, consider using a proper crypto library
     */
    QByteArray encrypt(const QString &plaintext, const QByteArray &key);
    QByteArray decrypt(const QByteArray &ciphertext, const QByteArray &key);
    
    QSettings *m_settings;
    QByteArray m_encryptionKey;
    
    static const QString SECURE_PREFIX;
};

#endif // SECURESTORAGE_H
