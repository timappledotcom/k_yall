#ifndef MASTODONSERVICE_H
#define MASTODONSERVICE_H

#include "serviceinterface.h"
#include "accountmanager.h"
#include <QNetworkRequest>
#include <QHttpMultiPart>

class MastodonService : public ServiceInterface
{
    Q_OBJECT

public:
    explicit MastodonService(QObject *parent = nullptr);

    QString serviceName() const override;
    void post(const Account &account, const QString &text, const QStringList &imagePaths) override;
    bool validateAccount(const Account &account) override;

private slots:
    void handleMediaUploadReply();
    void handleStatusPostReply();

private:
    void handleNetworkReply(QNetworkReply *reply) override;
    void uploadMedia(const Account &account, const QStringList &imagePaths);
    void postStatus(const Account &account, const QString &text, const QStringList &mediaIds);
    
    struct PostData {
        Account account;
        QString text;
        QStringList imagePaths;
        QStringList mediaIds;
        int pendingUploads;
    };
    
    QHash<QNetworkReply*, PostData> m_pendingPosts;
};

#endif // MASTODONSERVICE_H
