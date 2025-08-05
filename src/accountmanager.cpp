#include "accountmanager.h"
#include "mastodonservice.h"
#include "blueskyservice.h"
#include "microblogservice.h"
#include "nostrservice.h"
#include "testservice.h"
#include <QSettings>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QUuid>
#include <QStandardPaths>
#include <QDir>
#include <QDebug>

AccountManager::AccountManager(QObject *parent)
    : QObject(parent)
    , m_mastodonService(nullptr)
    , m_blueSkyService(nullptr)
    , m_microBlogService(nullptr)
    , m_nostrService(nullptr)
    , m_testService(nullptr)
{
    // Initialize default Nostr relays (5 most popular)
    m_defaultNostrRelays = {
        "wss://relay.damus.io",
        "wss://nos.lol",
        "wss://relay.snort.social",
        "wss://relay.current.fyi",
        "wss://brb.io"
    };
    
    initializeServices();
    loadSettings();
}

AccountManager::~AccountManager() = default;

void AccountManager::initializeServices()
{
    m_mastodonService = new MastodonService(this);
    m_blueSkyService = new BlueSkyService(this);
    m_microBlogService = new MicroBlogService(this);
    m_nostrService = new NostrService(this);
    m_testService = new TestService(this);
    
    connect(m_mastodonService, &ServiceInterface::postCompleted,
            this, &AccountManager::onServicePostCompleted);
    connect(m_blueSkyService, &ServiceInterface::postCompleted,
            this, &AccountManager::onServicePostCompleted);
    connect(m_microBlogService, &ServiceInterface::postCompleted,
            this, &AccountManager::onServicePostCompleted);
    connect(m_nostrService, &ServiceInterface::postCompleted,
            this, &AccountManager::onServicePostCompleted);
    connect(m_testService, &ServiceInterface::postCompleted,
            this, &AccountManager::onServicePostCompleted);
}

void AccountManager::addAccount(const Account &account)
{
    Account newAccount = account;
    if (newAccount.id.isEmpty()) {
        newAccount.id = generateAccountId();
    }
    
    // Remove existing account with same ID if exists
    removeAccount(newAccount.id);
    
    m_accounts.append(newAccount);
    saveSettings();
    emit accountsChanged();
}

void AccountManager::removeAccount(const QString &accountId)
{
    for (int i = 0; i < m_accounts.size(); ++i) {
        if (m_accounts[i].id == accountId) {
            m_accounts.removeAt(i);
            saveSettings();
            emit accountsChanged();
            break;
        }
    }
}

void AccountManager::updateAccount(const Account &account)
{
    for (int i = 0; i < m_accounts.size(); ++i) {
        if (m_accounts[i].id == account.id) {
            m_accounts[i] = account;
            saveSettings();
            emit accountsChanged();
            break;
        }
    }
}

Account AccountManager::getAccount(const QString &accountId) const
{
    for (const Account &account : m_accounts) {
        if (account.id == accountId) {
            return account;
        }
    }
    return Account{};
}

QList<Account> AccountManager::getAllAccounts() const
{
    return m_accounts;
}

QList<Account> AccountManager::getAccountsByService(const QString &service) const
{
    QList<Account> serviceAccounts;
    for (const Account &account : m_accounts) {
        if (account.service == service && account.enabled) {
            serviceAccounts.append(account);
        }
    }
    return serviceAccounts;
}

QStringList AccountManager::getDefaultNostrRelays() const
{
    return m_defaultNostrRelays;
}

void AccountManager::postToAccounts(const QString &text, const QStringList &imagePaths, 
                                   const QStringList &accountIds)
{
    qDebug() << "AccountManager: Posting to" << accountIds.size() << "accounts";
    
    if (accountIds.isEmpty()) {
        emit postCompleted("none", false, "No accounts provided");
        return;
    }
    
    for (const QString &accountId : accountIds) {
        Account account = getAccount(accountId);
        if (account.id.isEmpty()) {
            qDebug() << "Account not found:" << accountId;
            emit postCompleted("unknown", false, QString("Account not found: %1").arg(accountId));
            continue;
        }
        
        if (!account.enabled) {
            qDebug() << "Account disabled:" << account.displayName;
            emit postCompleted(account.service, false, QString("Account disabled: %1").arg(account.displayName));
            continue;
        }
        
        ServiceInterface *service = getServiceForAccount(account);
        if (service) {
            qDebug() << "Posting to service:" << account.service << "for account:" << account.displayName;
            service->post(account, text, imagePaths);
        } else {
            qDebug() << "No service found for:" << account.service;
            emit postCompleted(account.service, false, QString("No service implementation found for %1").arg(account.service));
        }
    }
}

ServiceInterface* AccountManager::getServiceForAccount(const Account &account)
{
    if (account.service == "mastodon") {
        return m_mastodonService;
    } else if (account.service == "bluesky") {
        return m_blueSkyService;
    } else if (account.service == "microblog") {
        return m_microBlogService;
    } else if (account.service == "nostr") {
        return m_nostrService;
    } else if (account.service == "test") {
        return m_testService;
    }
    return nullptr;
}

void AccountManager::onServicePostCompleted(bool success, const QString &error)
{
    ServiceInterface *service = qobject_cast<ServiceInterface*>(sender());
    if (service) {
        QString serviceName = service->serviceName();
        emit postCompleted(serviceName, success, error);
    }
}

QString AccountManager::generateAccountId() const
{
    return QUuid::createUuid().toString(QUuid::WithoutBraces);
}

void AccountManager::loadSettings()
{
    QSettings settings;
    settings.beginGroup("Accounts");
    
    QStringList accountIds = settings.childGroups();
    
    for (const QString &accountId : accountIds) {
        settings.beginGroup(accountId);
        
        Account account;
        account.id = accountId;
        account.service = settings.value("service").toString();
        account.displayName = settings.value("displayName").toString();
        account.username = settings.value("username").toString();
        account.serverUrl = settings.value("serverUrl").toString();
        account.accessToken = settings.value("accessToken").toString();
        account.privateKey = settings.value("privateKey").toString();
        account.defaultForPosting = settings.value("defaultForPosting", false).toBool();
        account.enabled = settings.value("enabled", true).toBool();
        
        // Load Nostr relays
        QStringList relays = settings.value("relays").toStringList();
        if (relays.isEmpty() && account.service == "nostr") {
            relays = m_defaultNostrRelays;
        }
        account.relays = relays;
        
        if (!account.service.isEmpty()) {
            m_accounts.append(account);
        }
        
        settings.endGroup();
    }
    
    settings.endGroup();
}

void AccountManager::saveSettings()
{
    QSettings settings;
    
    // Clear existing accounts
    settings.beginGroup("Accounts");
    settings.remove("");
    settings.endGroup();
    
    // Save all accounts
    settings.beginGroup("Accounts");
    
    for (const Account &account : m_accounts) {
        settings.beginGroup(account.id);
        
        settings.setValue("service", account.service);
        settings.setValue("displayName", account.displayName);
        settings.setValue("username", account.username);
        settings.setValue("serverUrl", account.serverUrl);
        settings.setValue("accessToken", account.accessToken);
        settings.setValue("privateKey", account.privateKey);
        settings.setValue("relays", account.relays);
        settings.setValue("defaultForPosting", account.defaultForPosting);
        settings.setValue("enabled", account.enabled);
        
        settings.endGroup();
    }
    
    settings.endGroup();
}
