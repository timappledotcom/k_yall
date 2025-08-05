#ifndef NOSTRSERVICE_H
#define NOSTRSERVICE_H

#include "serviceinterface.h"
#include "accountmanager.h"
#include <QWebSocket>
#include <QJsonObject>

class NostrService : public ServiceInterface
{
    Q_OBJECT

public:
    explicit NostrService(QObject *parent = nullptr);
    ~NostrService();

    QString serviceName() const override;
    void post(const Account &account, const QString &text, const QStringList &imagePaths) override;
    bool validateAccount(const Account &account) override;

private slots:
    void onWebSocketConnected();
    void onWebSocketDisconnected();
    void onWebSocketTextMessageReceived(const QString &message);
    void onWebSocketError();

private:
    void handleNetworkReply(QNetworkReply *reply) override;
    void connectToRelays(const QStringList &relays);
    void sendToRelays();
    void postWithRustHelper(const Account &account, const QString &text);
    QJsonObject createTextEvent(const QString &content, const QString &privateKey);
    QString signEvent(QJsonObject &event, const QString &privateKey);
    void uploadImages(const QStringList &imagePaths);
    
    struct PostData {
        Account account;
        QString text;
        QStringList imagePaths;
        QStringList imageUrls;
        int pendingUploads;
    };
    
    QList<QWebSocket*> m_relayConnections;
    PostData m_currentPost;
    bool m_posting;
    int m_relaySuccessCount;
    int m_relayAttemptCount;
    void *m_secp256k1Context; // secp256k1_context*
};

#endif // NOSTRSERVICE_H
