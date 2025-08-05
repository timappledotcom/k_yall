#include "microblogservice.h"
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QHttpMultiPart>
#include <QHttpPart>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QFileInfo>
#include <QUrlQuery>

MicroBlogService::MicroBlogService(QObject *parent)
    : ServiceInterface(parent)
{
}

QString MicroBlogService::serviceName() const
{
    return "MicroBlog";
}

bool MicroBlogService::validateAccount(const Account &account)
{
    return !account.serverUrl.isEmpty() && !account.accessToken.isEmpty();
}

void MicroBlogService::post(const Account &account, const QString &text, const QStringList &imagePaths)
{
    if (!validateAccount(account)) {
        emit postCompleted(false, "Invalid account configuration");
        return;
    }
    
    if (imagePaths.isEmpty()) {
        postStatus(account, text, QStringList());
    } else {
        uploadMedia(account, imagePaths);
    }
}

void MicroBlogService::uploadMedia(const Account &account, const QStringList &imagePaths)
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
        // MicroBlog typically uses Mastodon-compatible API
        request.setUrl(QUrl(account.serverUrl + "/api/v2/media"));
        request.setRawHeader("Authorization", QString("Bearer %1").arg(account.accessToken).toUtf8());
        
        QNetworkReply *reply = m_networkManager->post(request, multiPart);
        multiPart->setParent(reply);
        
        m_pendingPosts[reply] = postData;
        
        connect(reply, &QNetworkReply::finished,
                this, &MicroBlogService::handleMediaUploadReply);
    }
}

void MicroBlogService::postStatus(const Account &account, const QString &text, const QStringList &mediaUrls)
{
    QJsonObject statusObject;
    statusObject["content"] = text;
    
    if (!mediaUrls.isEmpty()) {
        QJsonArray mediaArray;
        for (const QString &mediaUrl : mediaUrls) {
            mediaArray.append(mediaUrl);
        }
        statusObject["media_ids"] = mediaArray;
    }
    
    QJsonDocument doc(statusObject);
    QByteArray data = doc.toJson();
    
    QNetworkRequest request;
    // MicroBlog typically uses its own API endpoint
    request.setUrl(QUrl(account.serverUrl + "/micropub"));
    request.setRawHeader("Authorization", QString("Bearer %1").arg(account.accessToken).toUtf8());
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    
    QNetworkReply *reply = m_networkManager->post(request, data);
    
    connect(reply, &QNetworkReply::finished,
            this, &MicroBlogService::handleStatusPostReply);
}

void MicroBlogService::handleMediaUploadReply()
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
    
    QByteArray data = reply->readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    QJsonObject obj = doc.object();
    
    if (obj.contains("url")) {
        QString mediaUrl = obj.value("url").toString();
        postData.mediaUrls.append(mediaUrl);
    } else if (obj.contains("id")) {
        QString mediaId = obj.value("id").toString();
        postData.mediaUrls.append(mediaId);
    }
    
    postData.pendingUploads--;
    
    if (postData.pendingUploads <= 0) {
        postStatus(postData.account, postData.text, postData.mediaUrls);
        m_pendingPosts.remove(reply);
    }
}

void MicroBlogService::handleStatusPostReply()
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

void MicroBlogService::handleNetworkReply(QNetworkReply *reply)
{
    Q_UNUSED(reply)
}
