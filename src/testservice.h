#ifndef TESTSERVICE_H
#define TESTSERVICE_H

#include "serviceinterface.h"
#include "accountmanager.h"

class TestService : public ServiceInterface
{
    Q_OBJECT

public:
    explicit TestService(QObject *parent = nullptr);

    QString serviceName() const override;
    void post(const Account &account, const QString &text, const QStringList &imagePaths) override;
    bool validateAccount(const Account &account) override;

private:
    void handleNetworkReply(QNetworkReply *reply) override;
};

#endif // TESTSERVICE_H
