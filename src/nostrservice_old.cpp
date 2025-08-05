#include "nostrservice.h"
#include <QWebSocket>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QCryptographicHash>
#include <QDateTime>
#include <QFile>
#include <QHttpMultiPart>
#include <QHttpPart>
#include <QFileInfo>
#include <QTimer>

NostrService::NostrService(QObject *parent)
    : ServiceInterface(parent)
    , m_posting(false)
{
}

QString NostrService::serviceName() const
{
    return "Nostr";
}

bool NostrService::validateAccount(const Account &account)
{
    return !account.privateKey.isEmpty() && !account.relays.isEmpty();
}

void NostrService::post(const Account &account, const QString &text, const QStringList &imagePaths)
{
    if (!validateAccount(account)) {
        emit postCompleted(false, "Invalid account configuration");
        return;
    }
    
    if (m_posting) {
        emit postCompleted(false, "Already posting, please wait");
        return;
    }
    
    m_posting = true;
    m_currentPost.account = account;
    m_currentPost.text = text;
    m_currentPost.imagePaths = imagePaths;
    m_currentPost.imageUrls.clear();
    m_currentPost.pendingUploads = imagePaths.size();
    
    if (imagePaths.isEmpty()) {
        // Post text only
        connectToRelays(account.relays);
        QJsonObject textEvent = createTextEvent(text, account.privateKey);
        sendEvent(textEvent);
    } else {
        // Upload images first
        for (const QString &imagePath : imagePaths) {
            uploadImageToNostrbuild(imagePath);
        }
    }
}

void NostrService::connectToRelays(const QStringList &relays)
{
    // Clean up existing connections
    for (QWebSocket *socket : m_relayConnections) {
        socket->deleteLater();
    }
    m_relayConnections.clear();
    
    // Connect to each relay
    for (const QString &relayUrl : relays) {
        QWebSocket *socket = new QWebSocket();
        m_relayConnections.append(socket);
        
        connect(socket, &QWebSocket::connected,
                this, &NostrService::onWebSocketConnected);
        connect(socket, &QWebSocket::disconnected,
                this, &NostrService::onWebSocketDisconnected);
        connect(socket, &QWebSocket::textMessageReceived,
                this, &NostrService::onWebSocketTextMessageReceived);
        
        socket->open(QUrl(relayUrl));
    }
}

QJsonObject NostrService::createTextEvent(const QString &content, const QString &privateKey)
{
    QJsonObject event;
    event["kind"] = 1; // Text note
    event["content"] = content;
    event["created_at"] = QDateTime::currentSecsSinceEpoch();
    event["tags"] = QJsonArray();
    
    // This is a simplified version - in reality you'd need proper secp256k1 signing
    QString signature = signEvent(event, privateKey);
    event["sig"] = signature;
    
    return event;
}

QString NostrService::signEvent(QJsonObject &event, const QString &privateKey)
{
    // This is a placeholder implementation
    // In a real Nostr client, you'd need to:
    // 1. Calculate the event ID (SHA256 of serialized event)
    // 2. Sign the event ID with secp256k1 using the private key
    // 3. Return the signature in hex format
    
    // For now, we'll just create a mock signature
    QString eventJson = QJsonDocument(event).toJson(QJsonDocument::Compact);
    QByteArray hash = QCryptographicHash::hash(eventJson.toUtf8(), QCryptographicHash::Sha256);
    
    event["id"] = QString(hash.toHex());
    // Mock public key (derived from private key in real implementation)
    event["pubkey"] = QString(QCryptographicHash::hash(privateKey.toUtf8(), QCryptographicHash::Sha256).toHex());
    
    return QString(hash.toHex()); // Mock signature
}

void NostrService::sendEvent(const QJsonObject &event)
{
    QJsonArray eventMessage;
    eventMessage.append("EVENT");
    eventMessage.append(event);
    
    QJsonDocument doc(eventMessage);
    QString message = doc.toJson(QJsonDocument::Compact);
    
    bool sentToAny = false;
    for (QWebSocket *socket : m_relayConnections) {
        if (socket->state() == QAbstractSocket::ConnectedState) {
            socket->sendTextMessage(message);
            sentToAny = true;
        }
    }
    
    if (sentToAny) {
        emit postCompleted(true, QString());
    } else {
        emit postCompleted(false, "Failed to connect to any relay");
    }
    
    m_posting = false;
}

void NostrService::uploadImageToNostrbuild(const QString &imagePath)
{
    QFile *file = new QFile(imagePath);
    if (!file->open(QIODevice::ReadOnly)) {
        emit postCompleted(false, QString("Failed to open image: %1").arg(imagePath));
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
    request.setUrl(QUrl("https://nostr.build/api/v2/upload/files"));
    
    QNetworkReply *reply = m_networkManager->post(request, multiPart);
    multiPart->setParent(reply);
    
    connect(reply, &QNetworkReply::finished, [this, reply]() {
        reply->deleteLater();
        
        if (reply->error() == QNetworkReply::NoError) {
            QByteArray data = reply->readAll();
            QJsonDocument doc = QJsonDocument::fromJson(data);
            QJsonObject obj = doc.object();
            
            if (obj.contains("data") && obj["data"].isArray()) {
                QJsonArray dataArray = obj["data"].toArray();
                if (!dataArray.isEmpty()) {
                    QJsonObject fileObj = dataArray[0].toObject();
                    if (fileObj.contains("url")) {
                        QString imageUrl = fileObj["url"].toString();
                        onImageUploadCompleted(imageUrl);
                        return;
                    }
                }
            }
        }
        
        emit postCompleted(false, extractErrorFromReply(reply));
        m_posting = false;
    });
}

void NostrService::onImageUploadCompleted(const QString &imageUrl)
{
    m_currentPost.imageUrls.append(imageUrl);
    m_currentPost.pendingUploads--;
    
    if (m_currentPost.pendingUploads <= 0) {
        // All images uploaded, create post with images
        QString content = m_currentPost.text;
        if (!m_currentPost.imageUrls.isEmpty()) {
            content += "\n\n" + m_currentPost.imageUrls.join("\n");
        }
        
        connectToRelays(m_currentPost.account.relays);
        QJsonObject textEvent = createTextEvent(content, m_currentPost.account.privateKey);
        sendEvent(textEvent);
    }
}

void NostrService::onWebSocketConnected()
{
    // Connected to relay - ready to send events
}

void NostrService::onWebSocketDisconnected()
{
    // Handle relay disconnection
}

void NostrService::onWebSocketTextMessageReceived(const QString &message)
{
    // Handle relay responses (OK, NOTICE, etc.)
    Q_UNUSED(message)
}

void NostrService::handleNetworkReply(QNetworkReply *reply)
{
    Q_UNUSED(reply)
}
