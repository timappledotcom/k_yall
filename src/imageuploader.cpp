#include "imageuploader.h"
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QHttpMultiPart>
#include <QHttpPart>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>

ImageUploader::ImageUploader(QObject *parent)
    : QObject(parent)
    , m_networkManager(new QNetworkAccessManager(this))
{
}

void ImageUploader::uploadImage(const QString &imagePath, const QString &service)
{
    if (service == "imgur") {
        uploadToImgur(imagePath);
    } else if (service == "cloudinary") {
        uploadToCloudinary(imagePath);
    } else {
        emit uploadCompleted(service, false, "Unsupported image service");
    }
}

void ImageUploader::uploadImages(const QStringList &imagePaths, const QString &service)
{
    for (const QString &imagePath : imagePaths) {
        uploadImage(imagePath, service);
    }
}

QString ImageUploader::getUploadUrl(const QString &service)
{
    if (service == "imgur") {
        return "https://api.imgur.com/3/image";
    } else if (service == "cloudinary") {
        // This would need to be configured with your Cloudinary account
        return "https://api.cloudinary.com/v1_1/your_cloud_name/image/upload";
    }
    return QString();
}

void ImageUploader::uploadToImgur(const QString &imagePath)
{
    QFile *file = new QFile(imagePath);
    if (!file->open(QIODevice::ReadOnly)) {
        emit uploadCompleted("imgur", false, QString("Failed to open image: %1").arg(imagePath));
        delete file;
        return;
    }
    
    QHttpMultiPart *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);
    
    QHttpPart imagePart;
    imagePart.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("application/octet-stream"));
    imagePart.setHeader(QNetworkRequest::ContentDispositionHeader, 
                       QVariant(QString("form-data; name=\"image\"; filename=\"%1\"")
                               .arg(QFileInfo(imagePath).fileName())));
    imagePart.setBodyDevice(file);
    file->setParent(multiPart);
    
    multiPart->append(imagePart);
    
    QNetworkRequest request;
    request.setUrl(QUrl("https://api.imgur.com/3/image"));
    // You would need to register for an Imgur API key
    request.setRawHeader("Authorization", "Client-ID YOUR_IMGUR_CLIENT_ID");
    
    QNetworkReply *reply = m_networkManager->post(request, multiPart);
    multiPart->setParent(reply);
    
    m_pendingUploads[reply] = imagePath;
    
    connect(reply, &QNetworkReply::finished,
            this, &ImageUploader::handleUploadReply);
}

void ImageUploader::uploadToCloudinary(const QString &imagePath)
{
    QFile *file = new QFile(imagePath);
    if (!file->open(QIODevice::ReadOnly)) {
        emit uploadCompleted("cloudinary", false, QString("Failed to open image: %1").arg(imagePath));
        delete file;
        return;
    }
    
    QHttpMultiPart *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);
    
    QHttpPart imagePart;
    imagePart.setHeader(QNetworkRequest::ContentDispositionHeader, 
                       QVariant("form-data; name=\"file\""));
    imagePart.setBodyDevice(file);
    file->setParent(multiPart);
    
    QHttpPart uploadPresetPart;
    uploadPresetPart.setHeader(QNetworkRequest::ContentDispositionHeader, 
                              QVariant("form-data; name=\"upload_preset\""));
    uploadPresetPart.setBody("your_upload_preset");
    
    multiPart->append(imagePart);
    multiPart->append(uploadPresetPart);
    
    QNetworkRequest request;
    request.setUrl(QUrl("https://api.cloudinary.com/v1_1/your_cloud_name/image/upload"));
    
    QNetworkReply *reply = m_networkManager->post(request, multiPart);
    multiPart->setParent(reply);
    
    m_pendingUploads[reply] = imagePath;
    
    connect(reply, &QNetworkReply::finished,
            this, &ImageUploader::handleUploadReply);
}

void ImageUploader::handleUploadReply()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) return;
    
    reply->deleteLater();
    
    QString imagePath = m_pendingUploads.value(reply);
    m_pendingUploads.remove(reply);
    
    if (reply->error() == QNetworkReply::NoError) {
        QByteArray data = reply->readAll();
        QJsonDocument doc = QJsonDocument::fromJson(data);
        QJsonObject obj = doc.object();
        
        QString imageUrl;
        if (obj.contains("data") && obj["data"].isObject()) {
            // Imgur response format
            QJsonObject dataObj = obj["data"].toObject();
            imageUrl = dataObj["link"].toString();
        } else if (obj.contains("secure_url")) {
            // Cloudinary response format
            imageUrl = obj["secure_url"].toString();
        }
        
        if (!imageUrl.isEmpty()) {
            emit uploadCompleted("image_service", true, imageUrl);
        } else {
            emit uploadCompleted("image_service", false, "Failed to parse upload response");
        }
    } else {
        QString error = reply->errorString();
        
        // Try to extract more specific error from JSON response
        QByteArray data = reply->readAll();
        QJsonDocument doc = QJsonDocument::fromJson(data);
        if (doc.isObject()) {
            QJsonObject obj = doc.object();
            if (obj.contains("data") && obj["data"].isObject()) {
                QJsonObject dataObj = obj["data"].toObject();
                if (dataObj.contains("error")) {
                    error = dataObj["error"].toString();
                }
            }
        }
        
        emit uploadCompleted("image_service", false, error);
    }
}
