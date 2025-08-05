#ifndef ACCOUNTDIALOG_H
#define ACCOUNTDIALOG_H

#include <QDialog>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QComboBox>
#include <QLineEdit>
#include <QTextEdit>
#include <QCheckBox>
#include <QPushButton>
#include <QStackedWidget>
#include <QGroupBox>
#include "accountmanager.h"

class AccountDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AccountDialog(const Account &account = Account{}, QWidget *parent = nullptr);
    
    Account getAccount() const;

private slots:
    void onServiceChanged();
    void onOAuthClicked();
    void onTestConnectionClicked();

private:
    void setupUI();
    void setupMastodonForm();
    void setupBlueSkyForm();
    void setupMicroBlogForm();
    void setupNostrForm();
    void setupTestForm();
    void loadAccount(const Account &account);
    Account createAccountFromForm() const;
    bool validateForm() const;
    
    Account m_originalAccount;
    
    QVBoxLayout *m_mainLayout;
    QFormLayout *m_formLayout;
    
    QComboBox *m_serviceCombo;
    QLineEdit *m_displayNameEdit;
    QLineEdit *m_usernameEdit;
    
    QStackedWidget *m_serviceStack;
    
    // Mastodon/MicroBlog widgets
    QWidget *m_mastodonWidget;
    QLineEdit *m_mastodonServerEdit;
    QLineEdit *m_mastodonTokenEdit;
    QPushButton *m_mastodonOAuthButton;
    
    // BlueSky widgets
    QWidget *m_blueSkyWidget;
    QLineEdit *m_blueSkyPasswordEdit;
    
    // Nostr widgets
    QWidget *m_nostrWidget;
    QLineEdit *m_nostrPrivateKeyEdit;
    QPushButton *m_generateKeyButton;
    QTextEdit *m_nostrRelaysEdit;
    
    // Test service widgets
    QWidget *m_testWidget;
    
    QCheckBox *m_enabledCheck;
    QCheckBox *m_defaultPostingCheck;
    
    QPushButton *m_testButton;
    QPushButton *m_saveButton;
    QPushButton *m_cancelButton;
};

#endif // ACCOUNTDIALOG_H
