#ifndef ACCOUNTMANAGER_H
#define ACCOUNTMANAGER_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QJsonObject>
#include <QJsonArray>

class SecureStorage;

struct Account {
    QString id;
    QString service;        // "mastodon", "bluesky", "microblog", "nostr"
    QString displayName;
    QString username;
    QString serverUrl;      // For Mastodon/MicroBlog
    QString accessToken;    // For Mastodon/MicroBlog/BlueSky
    QString privateKey;     // For Nostr
    QStringList relays;     // For Nostr
    bool defaultForPosting;
    bool enabled;
};

class ServiceInterface;
class MastodonService;
class BlueSkyService;
class MicroBlogService;
class NostrService;
class TestService;

class AccountManager : public QObject
{
    Q_OBJECT

public:
    explicit AccountManager(QObject *parent = nullptr);
    ~AccountManager();

    // Account management
    void addAccount(const Account &account);
    void removeAccount(const QString &accountId);
    void updateAccount(const Account &account);
    Account getAccount(const QString &accountId) const;
    QList<Account> getAllAccounts() const;
    QList<Account> getAccountsByService(const QString &service) const;
    
    // Default Nostr relays
    QStringList getDefaultNostrRelays() const;
    
    // Secure storage utilities
    void migrateToSecureStorage();
    bool hasPlainTextCredentials() const;
    
    // Posting
    void postToAccounts(const QString &text, const QStringList &imagePaths, 
                       const QStringList &accountIds);

    // Settings
    void loadSettings();
    void saveSettings();

signals:
    void postCompleted(const QString &accountId, bool success, const QString &error);
    void accountsChanged();

private slots:
    void onServicePostCompleted(bool success, const QString &error);

private:
    void initializeServices();
    ServiceInterface* getServiceForAccount(const Account &account);
    QString generateAccountId() const;
    
    QList<Account> m_accounts;
    
    // Service instances
    MastodonService *m_mastodonService;
    BlueSkyService *m_blueSkyService;
    MicroBlogService *m_microBlogService;
    NostrService *m_nostrService;
    TestService *m_testService;
    
    // Secure storage for credentials
    SecureStorage *m_secureStorage;
    
    // Default Nostr relays
    QStringList m_defaultNostrRelays;
};

#endif // ACCOUNTMANAGER_H
