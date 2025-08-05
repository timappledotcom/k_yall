#ifndef MICROBLOGSERVICE_H
#define MICROBLOGSERVICE_H

#include "serviceinterface.h"
#include "accountmanager.h"

class MicroBlogService : public ServiceInterface
{
    Q_OBJECT

public:
    explicit MicroBlogService(QObject *parent = nullptr);

    QString serviceName() const override;
    void post(const Account &account, const QString &text, const QStringList &imagePaths) override;
    bool validateAccount(const Account &account) override;

private slots:
    void handleMediaUploadReply();
    void handleStatusPostReply();

private:
    void handleNetworkReply(QNetworkReply *reply) override;
    void uploadMedia(const Account &account, const QStringList &imagePaths);
    void postStatus(const Account &account, const QString &text, const QStringList &mediaUrls);
    
    struct PostData {
        Account account;
        QString text;
        QStringList imagePaths;
        QStringList mediaUrls;
        int pendingUploads;
    };
    
    QHash<QNetworkReply*, PostData> m_pendingPosts;
};

#endif // MICROBLOGSERVICE_H
