#ifndef POSTWIDGET_H
#define POSTWIDGET_H

#include <QDialog>
#include <QTextEdit>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QComboBox>
#include <QCheckBox>
#include <QProgressBar>
#include <QListWidget>
#include <QGroupBox>

class AccountManager;
class ImageUploader;

class PostWidget : public QDialog
{
    Q_OBJECT

public:
    explicit PostWidget(AccountManager *accountManager, QWidget *parent = nullptr);

private slots:
    void onPostClicked();
    void onAddImageClicked();
    void onRemoveImageClicked();
    void onTextChanged();
    void onAccountSelectionChanged();
    void updateCharacterCount();
    void onPostCompleted(const QString &service, bool success, const QString &error);

private:
    void setupUI();
    void updateAccountCheckboxes();
    void clearForm();
    bool validatePost();
    
    AccountManager *m_accountManager;
    ImageUploader *m_imageUploader;
    
    QVBoxLayout *m_mainLayout;
    QTextEdit *m_postText;
    QLabel *m_charCountLabel;
    
    QGroupBox *m_accountsGroup;
    QVBoxLayout *m_accountsLayout;
    QList<QCheckBox*> m_accountCheckboxes;
    
    QGroupBox *m_imagesGroup;
    QVBoxLayout *m_imagesLayout;
    QListWidget *m_imagesList;
    QPushButton *m_addImageButton;
    QPushButton *m_removeImageButton;
    
    QHBoxLayout *m_buttonLayout;
    QPushButton *m_postButton;
    QPushButton *m_cancelButton;
    
    QProgressBar *m_progressBar;
    QLabel *m_statusLabel;
    
    static const int MAX_CHARACTER_COUNT = 500;
    QStringList m_imagePaths;
    
    // Track post completion to prevent multiple timer setups
    bool m_postCompletionHandled;
};

#endif // POSTWIDGET_H
