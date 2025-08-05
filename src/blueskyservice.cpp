#include "blueskyservice.h"
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QFileInfo>
#include <QDateTime>
#include <QMimeDatabase>
#include <QMimeType>

const QString BlueSkyService::BLUESKY_API_URL = "https://bsky.social/xrpc";

BlueSkyService::BlueSkyService(QObject *parent)
    : ServiceInterface(parent)
{
}

QString BlueSkyService::serviceName() const
{
    return "BlueSky";
}

bool BlueSkyService::validateAccount(const Account &account)
{
    return !account.username.isEmpty() && !account.accessToken.isEmpty();
}

void BlueSkyService::post(const Account &account, const QString &text, const QStringList &imagePaths)
{
    if (!validateAccount(account)) {
        emit postCompleted(false, "Invalid account configuration");
        return;
    }
    
    authenticateAndPost(account, text, imagePaths);
}

void BlueSkyService::authenticateAndPost(const Account &account, const QString &text, const QStringList &imagePaths)
{
    // For BlueSky, we assume the accessToken is actually the app password
    // In a real implementation, you'd want to handle the full OAuth flow
    
    QJsonObject authObject;
    authObject["identifier"] = account.username;
    authObject["password"] = account.accessToken; // This should be app password
    
    QJsonDocument doc(authObject);
    QByteArray data = doc.toJson();
    
    qDebug() << "BlueSky: Authenticating with credentials for:" << account.username;
    
    QNetworkRequest request;
    request.setUrl(QUrl(BLUESKY_API_URL + "/com.atproto.server.createSession"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    
    QNetworkReply *reply = m_networkManager->post(request, data);
    
    PostData postData;
    postData.account = account;
    postData.text = text;
    postData.imagePaths = imagePaths;
    postData.pendingUploads = imagePaths.size();
    
    m_pendingPosts[reply] = postData;
    
    connect(reply, &QNetworkReply::finished,
            this, &BlueSkyService::handleAuthReply);
}

void BlueSkyService::uploadBlobs(const Account &account, const QString &text, const QStringList &imagePaths, const QString &accessJwt)
{
    if (imagePaths.isEmpty()) {
        createPost(account, text, QStringList(), QStringList(), accessJwt);
        return;
    }
    
    PostData postData;
    postData.account = account;
    postData.text = text;
    postData.imagePaths = imagePaths;
    postData.accessJwt = accessJwt;
    postData.pendingUploads = imagePaths.size();
    
    for (const QString &imagePath : imagePaths) {
        QFile file(imagePath);
        if (!file.open(QIODevice::ReadOnly)) {
            emit postCompleted(false, QString("Failed to open image: %1").arg(imagePath));
            return;
        }
        
        QByteArray imageData = file.readAll();
        file.close();
        
        // Detect proper MIME type
        QMimeDatabase mimeDb;
        QMimeType mimeType = mimeDb.mimeTypeForFile(imagePath);
        QString contentType = mimeType.name();
        
        // Ensure we have a valid image MIME type
        if (!contentType.startsWith("image/")) {
            contentType = "image/jpeg"; // fallback
        }
        
        QNetworkRequest request;
        request.setUrl(QUrl(BLUESKY_API_URL + "/com.atproto.repo.uploadBlob"));
        request.setRawHeader("Authorization", QString("Bearer %1").arg(accessJwt).toUtf8());
        request.setHeader(QNetworkRequest::ContentTypeHeader, contentType);
        
        qDebug() << "BlueSky: Uploading blob with content type:" << contentType << "for file:" << imagePath;
        
        // Store the MIME type for this upload
        postData.mimeTypes.append(contentType);
        
        QNetworkReply *reply = m_networkManager->post(request, imageData);
        
        m_pendingPosts[reply] = postData;
        
        connect(reply, &QNetworkReply::finished,
                this, &BlueSkyService::handleUploadReply);
    }
}

void BlueSkyService::createPost(const Account &account, const QString &text, const QStringList &blobRefs, const QStringList &mimeTypes, const QString &accessJwt)
{
    QJsonObject recordObject;
    recordObject["$type"] = "app.bsky.feed.post";
    recordObject["text"] = text;
    recordObject["createdAt"] = QDateTime::currentDateTimeUtc().toString(Qt::ISODate);
    
    if (!blobRefs.isEmpty()) {
        QJsonArray embedImages;
        for (int i = 0; i < blobRefs.size(); ++i) {
            QJsonObject imageObject;
            
            // Parse the stored blob JSON string back to object
            QJsonDocument blobDoc = QJsonDocument::fromJson(blobRefs[i].toUtf8());
            QJsonObject blobObject = blobDoc.object();
            
            imageObject["image"] = blobObject;
            imageObject["alt"] = "";
            
            embedImages.append(imageObject);
        }
        
        QJsonObject embedObject;
        embedObject["$type"] = "app.bsky.embed.images";
        embedObject["images"] = embedImages;
        
        recordObject["embed"] = embedObject;
    }
    
    QJsonObject postObject;
    postObject["repo"] = account.username;
    postObject["collection"] = "app.bsky.feed.post";
    postObject["record"] = recordObject;
    
    QJsonDocument doc(postObject);
    QByteArray data = doc.toJson();
    
    qDebug() << "BlueSky: Creating post with data:" << doc.toJson(QJsonDocument::Compact);
    
    QNetworkRequest request;
    request.setUrl(QUrl(BLUESKY_API_URL + "/com.atproto.repo.createRecord"));
    request.setRawHeader("Authorization", QString("Bearer %1").arg(accessJwt).toUtf8());
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    
    QNetworkReply *reply = m_networkManager->post(request, data);
    
    connect(reply, &QNetworkReply::finished,
            this, &BlueSkyService::handlePostReply);
}

void BlueSkyService::handleAuthReply()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) return;
    
    reply->deleteLater();
    
    if (!m_pendingPosts.contains(reply)) {
        return;
    }
    
    PostData postData = m_pendingPosts[reply];
    m_pendingPosts.remove(reply);
    
    if (reply->error() != QNetworkReply::NoError) {
        qDebug() << "BlueSky: Authentication failed";
        emit postCompleted(false, extractErrorFromReply(reply));
        return;
    }
    
    QByteArray data = reply->readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    QJsonObject obj = doc.object();
    
    qDebug() << "BlueSky: Auth response:" << doc.toJson(QJsonDocument::Compact);
    
    if (!obj.contains("accessJwt")) {
        emit postCompleted(false, "Authentication failed - no accessJwt in response");
        return;
    }
    
    QString accessJwt = obj.value("accessJwt").toString();
    qDebug() << "BlueSky: Authentication successful, proceeding with upload";
    uploadBlobs(postData.account, postData.text, postData.imagePaths, accessJwt);
}

void BlueSkyService::handleUploadReply()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) return;
    
    reply->deleteLater();
    
    if (!m_pendingPosts.contains(reply)) {
        return;
    }
    
    PostData &postData = m_pendingPosts[reply];
    
    if (reply->error() != QNetworkReply::NoError) {
        QString error = extractErrorFromReply(reply);
        qDebug() << "BlueSky upload error:" << reply->error() << error;
        qDebug() << "Response data:" << reply->readAll();
        emit postCompleted(false, QString("Upload failed: %1").arg(error));
        m_pendingPosts.remove(reply);
        return;
    }
    
    QByteArray data = reply->readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    QJsonObject obj = doc.object();
    
    qDebug() << "BlueSky: Blob upload response:" << doc.toJson(QJsonDocument::Compact);
    
    if (obj.contains("blob") && obj["blob"].isObject()) {
        QJsonObject blobObj = obj["blob"].toObject();
        // Store the entire blob object, not just the ref
        QJsonDocument blobDoc(blobObj);
        QString blobJsonString = blobDoc.toJson(QJsonDocument::Compact);
        postData.blobRefs.append(blobJsonString);
        
        qDebug() << "BlueSky: Stored blob ref:" << blobJsonString;
    }
    
    postData.pendingUploads--;
    
    if (postData.pendingUploads <= 0) {
        createPost(postData.account, postData.text, postData.blobRefs, postData.mimeTypes, postData.accessJwt);
        m_pendingPosts.remove(reply);
    }
}

void BlueSkyService::handlePostReply()
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

void BlueSkyService::handleNetworkReply(QNetworkReply *reply)
{
    Q_UNUSED(reply)
}
