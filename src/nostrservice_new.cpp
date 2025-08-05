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
#include <QDebug>
#include <secp256k1.h>
#include <cstring>

NostrService::NostrService(QObject *parent)
    : ServiceInterface(parent)
    , m_posting(false)
    , m_secp256k1Context(nullptr)
{
    // Initialize secp256k1 context
    m_secp256k1Context = secp256k1_context_create(SECP256K1_CONTEXT_SIGN | SECP256K1_CONTEXT_VERIFY);
}

NostrService::~NostrService()
{
    if (m_secp256k1Context) {
        secp256k1_context_destroy(static_cast<secp256k1_context*>(m_secp256k1Context));
    }
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
    
    qDebug() << "NostrService: Starting post to" << account.relays.size() << "relays";
    
    m_posting = true;
    m_currentPost.account = account;
    m_currentPost.text = text;
    m_currentPost.imagePaths = imagePaths;
    m_currentPost.imageUrls.clear();
    m_currentPost.pendingUploads = imagePaths.size();
    m_relaySuccessCount = 0;
    m_relayAttemptCount = 0;
    
    // For now, skip image uploads and post directly
    if (imagePaths.isEmpty()) {
        sendToRelays();
    } else {
        emit postCompleted(false, "Image uploads not yet supported for Nostr");
        m_posting = false;
    }
}

void NostrService::sendToRelays()
{
    qDebug() << "NostrService: Sending to relays";
    connectToRelays(m_currentPost.account.relays);
}

void NostrService::connectToRelays(const QStringList &relays)
{
    qDebug() << "NostrService: Connecting to" << relays.size() << "relays:" << relays;
    
    // Clean up existing connections
    for (QWebSocket *socket : m_relayConnections) {
        socket->deleteLater();
    }
    m_relayConnections.clear();
    
    m_relayAttemptCount = relays.size();
    
    // Connect to each relay
    for (const QString &relayUrl : relays) {
        QWebSocket *socket = new QWebSocket();
        m_relayConnections.append(socket);
        
        connect(socket, &QWebSocket::connected, [this, relayUrl]() {
            qDebug() << "NostrService: Connected to relay:" << relayUrl;
            onWebSocketConnected();
        });
        
        connect(socket, &QWebSocket::disconnected, [this, relayUrl]() {
            qDebug() << "NostrService: Disconnected from relay:" << relayUrl;
            onWebSocketDisconnected();
        });
        
        connect(socket, QOverload<QAbstractSocket::SocketError>::of(&QWebSocket::error),
                [this, relayUrl](QAbstractSocket::SocketError error) {
            qDebug() << "NostrService: WebSocket error for" << relayUrl << ":" << error;
            onWebSocketError();
        });
        
        connect(socket, &QWebSocket::textMessageReceived,
                this, &NostrService::onWebSocketTextMessageReceived);
        
        qDebug() << "NostrService: Opening connection to" << relayUrl;
        socket->open(QUrl(relayUrl));
    }
    
    // Set a timeout for connections
    QTimer::singleShot(10000, this, [this]() {
        if (m_posting && m_relaySuccessCount == 0) {
            qDebug() << "NostrService: Connection timeout, no relays connected";
            emit postCompleted(false, "Failed to connect to any relay (timeout)");
            m_posting = false;
        }
    });
}

void NostrService::onWebSocketConnected()
{
    qDebug() << "NostrService: WebSocket connected, creating and sending event";
    
    // Create the Nostr event
    QJsonObject event = createTextEvent(m_currentPost.text, m_currentPost.account.privateKey);
    
    // Create REQ message for Nostr relay
    QJsonArray reqMessage;
    reqMessage.append("EVENT");
    reqMessage.append(event);
    
    QString message = QJsonDocument(reqMessage).toJson(QJsonDocument::Compact);
    qDebug() << "NostrService: Sending event:" << message;
    
    QWebSocket *socket = qobject_cast<QWebSocket*>(sender());
    if (socket) {
        socket->sendTextMessage(message);
    }
}

void NostrService::onWebSocketDisconnected()
{
    qDebug() << "NostrService: WebSocket disconnected";
}

void NostrService::onWebSocketError()
{
    qDebug() << "NostrService: WebSocket error occurred";
    m_relayAttemptCount--;
    
    if (m_relayAttemptCount <= 0 && m_relaySuccessCount == 0) {
        emit postCompleted(false, "Failed to connect to any relay (connection errors)");
        m_posting = false;
    }
}

void NostrService::onWebSocketTextMessageReceived(const QString &message)
{
    qDebug() << "NostrService: Received message:" << message;
    
    // Parse the response
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(message.toUtf8(), &error);
    
    if (error.error != QJsonParseError::NoError) {
        qDebug() << "NostrService: Failed to parse JSON response:" << error.errorString();
        return;
    }
    
    QJsonArray response = doc.array();
    if (response.size() >= 2) {
        QString type = response[0].toString();
        if (type == "OK") {
            bool success = response[2].toBool();
            if (success) {
                m_relaySuccessCount++;
                qDebug() << "NostrService: Event posted successfully to relay";
                
                // Consider it a success if we get at least one successful post
                if (m_relaySuccessCount == 1) {
                    emit postCompleted(true, "");
                    m_posting = false;
                }
            } else {
                QString reason = response.size() > 3 ? response[3].toString() : "Unknown error";
                qDebug() << "NostrService: Event rejected by relay:" << reason;
            }
        } else if (type == "NOTICE") {
            QString notice = response[1].toString();
            qDebug() << "NostrService: Relay notice:" << notice;
        }
    }
}

QJsonObject NostrService::createTextEvent(const QString &content, const QString &privateKey)
{
    QJsonObject event;
    event["kind"] = 1; // Text note
    event["content"] = content;
    event["created_at"] = QDateTime::currentSecsSinceEpoch();
    event["tags"] = QJsonArray();
    
    // Convert private key from hex
    QByteArray privKeyBytes = QByteArray::fromHex(privateKey.toUtf8());
    if (privKeyBytes.size() != 32) {
        qDebug() << "NostrService: Invalid private key length:" << privKeyBytes.size();
        return event;
    }
    
    // Generate public key
    secp256k1_pubkey pubkey;
    if (!secp256k1_ec_pubkey_create(static_cast<const secp256k1_context*>(m_secp256k1Context), 
                                   &pubkey, 
                                   reinterpret_cast<const unsigned char*>(privKeyBytes.constData()))) {
        qDebug() << "NostrService: Failed to create public key";
        return event;
    }
    
    // Serialize public key
    unsigned char pubkeyBytes[33];
    size_t pubkeyLen = 33;
    secp256k1_ec_pubkey_serialize(static_cast<const secp256k1_context*>(m_secp256k1Context),
                                 pubkeyBytes,
                                 &pubkeyLen,
                                 &pubkey,
                                 SECP256K1_EC_COMPRESSED);
    
    // Take x-coordinate only (32 bytes) for Nostr pubkey
    QByteArray pubkeyHex = QByteArray(reinterpret_cast<const char*>(pubkeyBytes + 1), 32).toHex();
    event["pubkey"] = QString(pubkeyHex);
    
    // Sign the event
    QString signature = signEvent(event, privateKey);
    event["sig"] = signature;
    
    return event;
}

QString NostrService::signEvent(QJsonObject &event, const QString &privateKey)
{
    // Calculate event ID according to NIP-01
    QJsonArray idArray;
    idArray.append(0); // Reserved
    idArray.append(event["pubkey"].toString()); // Public key
    idArray.append(event["created_at"].toInt()); // Created at
    idArray.append(event["kind"].toInt()); // Kind
    idArray.append(event["tags"]); // Tags
    idArray.append(event["content"].toString()); // Content
    
    QString idString = QJsonDocument(idArray).toJson(QJsonDocument::Compact);
    QByteArray idBytes = QCryptographicHash::hash(idString.toUtf8(), QCryptographicHash::Sha256);
    event["id"] = QString(idBytes.toHex());
    
    // Sign the event ID
    QByteArray privKeyBytes = QByteArray::fromHex(privateKey.toUtf8());
    if (privKeyBytes.size() != 32) {
        qDebug() << "NostrService: Invalid private key for signing";
        return QString();
    }
    
    secp256k1_ecdsa_signature signature;
    if (!secp256k1_ecdsa_sign(static_cast<const secp256k1_context*>(m_secp256k1Context),
                             &signature,
                             reinterpret_cast<const unsigned char*>(idBytes.constData()),
                             reinterpret_cast<const unsigned char*>(privKeyBytes.constData()),
                             nullptr,
                             nullptr)) {
        qDebug() << "NostrService: Failed to sign event";
        return QString();
    }
    
    // Serialize signature
    unsigned char sigBytes[64];
    secp256k1_ecdsa_signature_serialize_compact(static_cast<const secp256k1_context*>(m_secp256k1Context),
                                               sigBytes,
                                               &signature);
    
    return QString(QByteArray(reinterpret_cast<const char*>(sigBytes), 64).toHex());
}

void NostrService::uploadImages(const QStringList &imagePaths)
{
    // TODO: Implement image upload to a file hosting service
    // For now, we'll skip image uploads for Nostr
    emit postCompleted(false, "Image uploads not yet supported for Nostr");
}
