#include "mastodonservice.h"
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QHttpMultiPart>
#include <QHttpPart>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QFileInfo>
#include <QMimeDatabase>
#include <QUrlQuery>

MastodonService::MastodonService(QObject *parent)
    : ServiceInterface(parent)
{
}

QString MastodonService::serviceName() const
{
    return "Mastodon";
}

bool MastodonService::validateAccount(const Account &account)
{
    return !account.serverUrl.isEmpty() && !account.accessToken.isEmpty();
}

void MastodonService::post(const Account &account, const QString &text, const QStringList &imagePaths)
{
    if (!validateAccount(account)) {
        emit postCompleted(false, "Invalid account configuration");
        return;
    }
    
    if (imagePaths.isEmpty()) {
        // Post without media
        postStatus(account, text, QStringList());
    } else {
        // Upload media first, then post
        uploadMedia(account, imagePaths);
    }
}

void MastodonService::uploadMedia(const Account &account, const QStringList &imagePaths)
{
    PostData postData;
    postData.account = account;
    postData.imagePaths = imagePaths;
    postData.pendingUploads = imagePaths.size();
    
    for (const QString &imagePath : imagePaths) {
        QFile *file = new QFile(imagePath);
        if (!file->open(QIODevice::ReadOnly)) {
            emit postCompleted(false, QString("Failed to open image: %1").arg(imagePath));
            delete file;
            return;
        }
        
        QHttpMultiPart *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);
        
        QHttpPart imagePart;
        imagePart.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("application/octet-stream"));
        imagePart.setHeader(QNetworkRequest::ContentDispositionHeader, 
                           QVariant(QString("form-data; name=\"file\"; filename=\"%1\"")
                                   .arg(QFileInfo(imagePath).fileName())));
        imagePart.setBodyDevice(file);
        file->setParent(multiPart);
        
        multiPart->append(imagePart);
        
        QNetworkRequest request;
        request.setUrl(QUrl(account.serverUrl + "/api/v2/media"));
        request.setRawHeader("Authorization", QString("Bearer %1").arg(account.accessToken).toUtf8());
        
        QNetworkReply *reply = m_networkManager->post(request, multiPart);
        multiPart->setParent(reply);
        
        m_pendingPosts[reply] = postData;
        
        connect(reply, &QNetworkReply::finished,
                this, &MastodonService::handleMediaUploadReply);
    }
}

void MastodonService::postStatus(const Account &account, const QString &text, const QStringList &mediaIds)
{
    QJsonObject statusObject;
    statusObject["status"] = text;
    
    if (!mediaIds.isEmpty()) {
        QJsonArray mediaArray;
        for (const QString &mediaId : mediaIds) {
            mediaArray.append(mediaId);
        }
        statusObject["media_ids"] = mediaArray;
    }
    
    QJsonDocument doc(statusObject);
    QByteArray data = doc.toJson();
    
    QNetworkRequest request;
    request.setUrl(QUrl(account.serverUrl + "/api/v1/statuses"));
    request.setRawHeader("Authorization", QString("Bearer %1").arg(account.accessToken).toUtf8());
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    
    QNetworkReply *reply = m_networkManager->post(request, data);
    
    connect(reply, &QNetworkReply::finished,
            this, &MastodonService::handleStatusPostReply);
}

void MastodonService::handleMediaUploadReply()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) return;
    
    reply->deleteLater();
    
    if (!m_pendingPosts.contains(reply)) {
        return;
    }
    
    PostData &postData = m_pendingPosts[reply];
    
    if (reply->error() != QNetworkReply::NoError) {
        emit postCompleted(false, extractErrorFromReply(reply));
        m_pendingPosts.remove(reply);
        return;
    }
    
    // Parse media ID from response
    QByteArray data = reply->readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    QJsonObject obj = doc.object();
    
    if (obj.contains("id")) {
        QString mediaId = obj.value("id").toString();
        postData.mediaIds.append(mediaId);
    }
    
    postData.pendingUploads--;
    
    if (postData.pendingUploads <= 0) {
        // All media uploaded, now post the status
        postStatus(postData.account, postData.text, postData.mediaIds);
        m_pendingPosts.remove(reply);
    }
}

void MastodonService::handleStatusPostReply()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) return;
    
    reply->deleteLater();
    
    if (reply->error() == QNetworkReply::NoError) {
        emit postCompleted(true, QString());
    } else {
        emit postCompleted(false, extractErrorFromReply(reply));
    }
}

void MastodonService::handleNetworkReply(QNetworkReply *reply)
{
    // This method is called by the base class but we handle replies
    // in specific slot methods above
    Q_UNUSED(reply)
}
