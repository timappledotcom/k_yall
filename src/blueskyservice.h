#ifndef BLUESKYSERVICE_H
#define BLUESKYSERVICE_H

#include "serviceinterface.h"
#include "accountmanager.h"
#include <QNetworkRequest>

class BlueSkyService : public ServiceInterface
{
    Q_OBJECT

public:
    explicit BlueSkyService(QObject *parent = nullptr);

    QString serviceName() const override;
    void post(const Account &account, const QString &text, const QStringList &imagePaths) override;
    bool validateAccount(const Account &account) override;

private slots:
    void handleAuthReply();
    void handleUploadReply();
    void handlePostReply();

private:
    void handleNetworkReply(QNetworkReply *reply) override;
    void authenticateAndPost(const Account &account, const QString &text, const QStringList &imagePaths);
    void uploadBlobs(const Account &account, const QString &text, const QStringList &imagePaths, const QString &accessJwt);
    void createPost(const Account &account, const QString &text, const QStringList &blobRefs, const QStringList &mimeTypes, const QString &accessJwt);
    
    struct PostData {
        Account account;
        QString text;
        QStringList imagePaths;
        QStringList blobRefs;
        QStringList mimeTypes;
        QString accessJwt;
        int pendingUploads;
        QHash<QNetworkReply*, QString> replyMimeTypes; // Track MIME type per reply
    };
    
    QHash<QNetworkReply*, PostData> m_pendingPosts;
    static const QString BLUESKY_API_URL;
};

#endif // BLUESKYSERVICE_H
