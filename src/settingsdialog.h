#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLineEdit>
#include <QTextEdit>
#include <QComboBox>
#include <QCheckBox>
#include <QPushButton>
#include <QListWidget>
#include <QGroupBox>
#include <QLabel>
#include "accountmanager.h"

class AccountDialog;

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(AccountManager *accountManager, QWidget *parent = nullptr);

private slots:
    void onAddAccountClicked();
    void onEditAccountClicked();
    void onRemoveAccountClicked();
    void onAccountSelectionChanged();
    void onTestConnectionClicked();
    void onSaveClicked();
    void onCancelClicked();

private:
    void setupUI();
    void setupAccountsTab();
    void setupNostrTab();
    void setupGeneralTab();
    void loadSettings();
    void saveSettings();
    void refreshAccountsList();
    void editAccount(const Account &account = {});
    
    AccountManager *m_accountManager;
    
    QTabWidget *m_tabWidget;
    
    // Accounts tab
    QWidget *m_accountsTab;
    QListWidget *m_accountsList;
    QPushButton *m_addAccountButton;
    QPushButton *m_editAccountButton;
    QPushButton *m_removeAccountButton;
    QPushButton *m_testConnectionButton;
    
    // Nostr tab
    QWidget *m_nostrTab;
    QListWidget *m_relaysList;
    QLineEdit *m_newRelayEdit;
    QPushButton *m_addRelayButton;
    QPushButton *m_removeRelayButton;
    QPushButton *m_resetRelaysButton;
    
    // General tab
    QWidget *m_generalTab;
    QCheckBox *m_startMinimizedCheck;
    QCheckBox *m_showNotificationsCheck;
    QComboBox *m_themeCombo;
    
    // Buttons
    QPushButton *m_saveButton;
    QPushButton *m_cancelButton;
};

#endif // SETTINGSDIALOG_H
