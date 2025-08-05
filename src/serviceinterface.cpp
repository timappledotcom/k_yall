#include "serviceinterface.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>

ServiceInterface::ServiceInterface(QObject *parent)
    : QObject(parent)
    , m_networkManager(new QNetworkAccessManager(this))
{
}

QString ServiceInterface::extractErrorFromReply(QNetworkReply *reply)
{
    if (reply->error() == QNetworkReply::NoError) {
        return QString();
    }
    
    QString errorMessage = reply->errorString();
    
    // Try to extract more specific error from JSON response
    QByteArray data = reply->readAll();
    
    // Log raw response for debugging
    qDebug() << "HTTP Error:" << reply->error() << errorMessage;
    qDebug() << "HTTP Status:" << reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    qDebug() << "Raw Response:" << data;
    
    QJsonDocument doc = QJsonDocument::fromJson(data);
    
    if (doc.isObject()) {
        QJsonObject obj = doc.object();
        if (obj.contains("error")) {
            errorMessage = obj.value("error").toString();
        } else if (obj.contains("message")) {
            errorMessage = obj.value("message").toString();
        } else if (obj.contains("error_description")) {
            errorMessage = obj.value("error_description").toString();
        }
    }
    
    return errorMessage;
}
