#ifndef IMAGEUPLOADER_H
#define IMAGEUPLOADER_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QNetworkAccessManager>
#include <QNetworkReply>

class ImageUploader : public QObject
{
    Q_OBJECT

public:
    explicit ImageUploader(QObject *parent = nullptr);

    void uploadImage(const QString &imagePath, const QString &service);
    void uploadImages(const QStringList &imagePaths, const QString &service);

signals:
    void uploadCompleted(const QString &service, bool success, const QString &error);
    void uploadProgress(const QString &imagePath, int percentage);

private slots:
    void handleUploadReply();

private:
    QString getUploadUrl(const QString &service);
    void uploadToImgur(const QString &imagePath);
    void uploadToCloudinary(const QString &imagePath);
    
    QNetworkAccessManager *m_networkManager;
    QHash<QNetworkReply*, QString> m_pendingUploads;
};

#endif // IMAGEUPLOADER_H
