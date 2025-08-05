#ifndef SERVICEINTERFACE_H
#define SERVICEINTERFACE_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QNetworkAccessManager>
#include <QNetworkReply>

struct Account;

class ServiceInterface : public QObject
{
    Q_OBJECT

public:
    explicit ServiceInterface(QObject *parent = nullptr);
    virtual ~ServiceInterface() = default;

    virtual QString serviceName() const = 0;
    virtual void post(const Account &account, const QString &text, const QStringList &imagePaths) = 0;
    virtual bool validateAccount(const Account &account) = 0;

signals:
    void postCompleted(bool success, const QString &error);
    void authenticationRequired(const QString &authUrl);

protected:
    QNetworkAccessManager *m_networkManager;
    
    virtual void handleNetworkReply(QNetworkReply *reply) = 0;
    QString extractErrorFromReply(QNetworkReply *reply);
};

#endif // SERVICEINTERFACE_H
