#include "testservice.h"
#include <QTimer>
#include <QDebug>

TestService::TestService(QObject *parent)
    : ServiceInterface(parent)
{
}

QString TestService::serviceName() const
{
    return "Test Service";
}

bool TestService::validateAccount(const Account &account)
{
    return !account.username.isEmpty();
}

void TestService::post(const Account &account, const QString &text, const QStringList &imagePaths)
{
    qDebug() << "TestService: Posting for account" << account.displayName;
    qDebug() << "Text:" << text;
    qDebug() << "Images:" << imagePaths;
    
    // Simulate posting delay
    QTimer::singleShot(1000, this, [this]() {
        // Simulate successful post
        emit postCompleted(true, QString());
    });
}

void TestService::handleNetworkReply(QNetworkReply *reply)
{
    Q_UNUSED(reply)
}
