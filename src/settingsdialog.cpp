#include "settingsdialog.h"
#include "accountmanager.h"
#include "accountdialog.h"
#include <QTabWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLineEdit>
#include <QComboBox>
#include <QCheckBox>
#include <QPushButton>
#include <QListWidget>
#include <QGroupBox>
#include <QLabel>
#include <QMessageBox>
#include <QSettings>
#include <KLocalizedString>

SettingsDialog::SettingsDialog(AccountManager *accountManager, QWidget *parent)
    : QDialog(parent)
    , m_accountManager(accountManager)
{
    setWindowTitle(i18n("Settings - K, Y'all"));
    setModal(true);
    resize(600, 500);
    
    setupUI();
    loadSettings();
    refreshAccountsList();
    
    connect(m_accountManager, &AccountManager::accountsChanged,
            this, &SettingsDialog::refreshAccountsList);
}

void SettingsDialog::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    
    m_tabWidget = new QTabWidget(this);
    
    setupAccountsTab();
    setupNostrTab();
    setupGeneralTab();
    
    m_tabWidget->addTab(m_accountsTab, i18n("Accounts"));
    m_tabWidget->addTab(m_nostrTab, i18n("Nostr Settings"));
    m_tabWidget->addTab(m_generalTab, i18n("General"));
    
    // Buttons
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    m_saveButton = new QPushButton(i18n("Save"), this);
    m_cancelButton = new QPushButton(i18n("Cancel"), this);
    
    buttonLayout->addStretch();
    buttonLayout->addWidget(m_saveButton);
    buttonLayout->addWidget(m_cancelButton);
    
    mainLayout->addWidget(m_tabWidget);
    mainLayout->addLayout(buttonLayout);
    
    connect(m_saveButton, &QPushButton::clicked, this, &SettingsDialog::onSaveClicked);
    connect(m_cancelButton, &QPushButton::clicked, this, &SettingsDialog::onCancelClicked);
}

void SettingsDialog::setupAccountsTab()
{
    m_accountsTab = new QWidget();
    QHBoxLayout *accountsLayout = new QHBoxLayout(m_accountsTab);
    
    // Left side - accounts list
    QVBoxLayout *leftLayout = new QVBoxLayout();
    
    QLabel *accountsLabel = new QLabel(i18n("Configured Accounts:"), this);
    m_accountsList = new QListWidget(this);
    
    leftLayout->addWidget(accountsLabel);
    leftLayout->addWidget(m_accountsList);
    
    // Right side - buttons
    QVBoxLayout *rightLayout = new QVBoxLayout();
    
    m_addAccountButton = new QPushButton(i18n("Add Account"), this);
    m_editAccountButton = new QPushButton(i18n("Edit Account"), this);
    m_removeAccountButton = new QPushButton(i18n("Remove Account"), this);
    m_testConnectionButton = new QPushButton(i18n("Test Connection"), this);
    
    m_editAccountButton->setEnabled(false);
    m_removeAccountButton->setEnabled(false);
    m_testConnectionButton->setEnabled(false);
    
    rightLayout->addWidget(m_addAccountButton);
    rightLayout->addWidget(m_editAccountButton);
    rightLayout->addWidget(m_removeAccountButton);
    rightLayout->addWidget(m_testConnectionButton);
    rightLayout->addStretch();
    
    accountsLayout->addLayout(leftLayout, 3);
    accountsLayout->addLayout(rightLayout, 1);
    
    connect(m_addAccountButton, &QPushButton::clicked, this, &SettingsDialog::onAddAccountClicked);
    connect(m_editAccountButton, &QPushButton::clicked, this, &SettingsDialog::onEditAccountClicked);
    connect(m_removeAccountButton, &QPushButton::clicked, this, &SettingsDialog::onRemoveAccountClicked);
    connect(m_testConnectionButton, &QPushButton::clicked, this, &SettingsDialog::onTestConnectionClicked);
    connect(m_accountsList, &QListWidget::itemSelectionChanged, this, &SettingsDialog::onAccountSelectionChanged);
}

void SettingsDialog::setupNostrTab()
{
    m_nostrTab = new QWidget();
    QVBoxLayout *nostrLayout = new QVBoxLayout(m_nostrTab);
    
    QGroupBox *relaysGroup = new QGroupBox(i18n("Nostr Relays"), this);
    QVBoxLayout *relaysLayout = new QVBoxLayout(relaysGroup);
    
    QLabel *relaysLabel = new QLabel(i18n("Configure up to 10 Nostr relays (5 default + 5 custom):"), this);
    m_relaysList = new QListWidget(this);
    
    QHBoxLayout *addRelayLayout = new QHBoxLayout();
    m_newRelayEdit = new QLineEdit(this);
    m_newRelayEdit->setPlaceholderText(i18n("wss://relay.example.com"));
    m_addRelayButton = new QPushButton(i18n("Add Relay"), this);
    
    addRelayLayout->addWidget(m_newRelayEdit);
    addRelayLayout->addWidget(m_addRelayButton);
    
    QHBoxLayout *relayButtonsLayout = new QHBoxLayout();
    m_removeRelayButton = new QPushButton(i18n("Remove Selected"), this);
    m_resetRelaysButton = new QPushButton(i18n("Reset to Defaults"), this);
    m_removeRelayButton->setEnabled(false);
    
    relayButtonsLayout->addWidget(m_removeRelayButton);
    relayButtonsLayout->addWidget(m_resetRelaysButton);
    relayButtonsLayout->addStretch();
    
    relaysLayout->addWidget(relaysLabel);
    relaysLayout->addWidget(m_relaysList);
    relaysLayout->addLayout(addRelayLayout);
    relaysLayout->addLayout(relayButtonsLayout);
    
    nostrLayout->addWidget(relaysGroup);
    nostrLayout->addStretch();
    
    connect(m_addRelayButton, &QPushButton::clicked, [this]() {
        QString relay = m_newRelayEdit->text().trimmed();
        if (!relay.isEmpty() && !relay.startsWith("wss://")) {
            relay = "wss://" + relay;
        }
        if (!relay.isEmpty() && m_relaysList->findItems(relay, Qt::MatchExactly).isEmpty()) {
            if (m_relaysList->count() < 10) {
                m_relaysList->addItem(relay);
                m_newRelayEdit->clear();
            } else {
                QMessageBox::warning(this, i18n("Too Many Relays"),
                                    i18n("You can configure a maximum of 10 relays."));
            }
        }
    });
    
    connect(m_removeRelayButton, &QPushButton::clicked, [this]() {
        int currentRow = m_relaysList->currentRow();
        if (currentRow >= 0) {
            delete m_relaysList->takeItem(currentRow);
        }
    });
    
    connect(m_resetRelaysButton, &QPushButton::clicked, [this]() {
        m_relaysList->clear();
        QStringList defaultRelays = m_accountManager->getDefaultNostrRelays();
        for (const QString &relay : defaultRelays) {
            m_relaysList->addItem(relay);
        }
    });
    
    connect(m_relaysList, &QListWidget::itemSelectionChanged, [this]() {
        m_removeRelayButton->setEnabled(m_relaysList->currentItem() != nullptr);
    });
}

void SettingsDialog::setupGeneralTab()
{
    m_generalTab = new QWidget();
    QFormLayout *generalLayout = new QFormLayout(m_generalTab);
    
    m_startMinimizedCheck = new QCheckBox(i18n("Start minimized to system tray"), this);
    m_showNotificationsCheck = new QCheckBox(i18n("Show desktop notifications"), this);
    
    m_themeCombo = new QComboBox(this);
    m_themeCombo->addItem(i18n("System Default"));
    m_themeCombo->addItem(i18n("Light"));
    m_themeCombo->addItem(i18n("Dark"));
    
    generalLayout->addRow(i18n("Startup:"), m_startMinimizedCheck);
    generalLayout->addRow(i18n("Notifications:"), m_showNotificationsCheck);
    generalLayout->addRow(i18n("Theme:"), m_themeCombo);
}

void SettingsDialog::refreshAccountsList()
{
    m_accountsList->clear();
    
    QList<Account> accounts = m_accountManager->getAllAccounts();
    for (const Account &account : accounts) {
        QString displayText = QString("%1 (%2)")
                             .arg(account.displayName.isEmpty() ? account.username : account.displayName)
                             .arg(account.service);
        
        QListWidgetItem *item = new QListWidgetItem(displayText);
        item->setData(Qt::UserRole, account.id);
        
        if (!account.enabled) {
            item->setForeground(QBrush(Qt::gray));
            displayText += " " + i18n("(Disabled)");
            item->setText(displayText);
        }
        
        m_accountsList->addItem(item);
    }
}

void SettingsDialog::onAddAccountClicked()
{
    editAccount();
}

void SettingsDialog::onEditAccountClicked()
{
    QListWidgetItem *item = m_accountsList->currentItem();
    if (!item) return;
    
    QString accountId = item->data(Qt::UserRole).toString();
    Account account = m_accountManager->getAccount(accountId);
    
    editAccount(account);
}

void SettingsDialog::onRemoveAccountClicked()
{
    QListWidgetItem *item = m_accountsList->currentItem();
    if (!item) return;
    
    QString accountId = item->data(Qt::UserRole).toString();
    Account account = m_accountManager->getAccount(accountId);
    
    int ret = QMessageBox::question(this, i18n("Remove Account"),
                                   i18n("Are you sure you want to remove the account '%1'?")
                                   .arg(account.displayName.isEmpty() ? account.username : account.displayName),
                                   QMessageBox::Yes | QMessageBox::No);
    
    if (ret == QMessageBox::Yes) {
        m_accountManager->removeAccount(accountId);
    }
}

void SettingsDialog::onAccountSelectionChanged()
{
    bool hasSelection = m_accountsList->currentItem() != nullptr;
    m_editAccountButton->setEnabled(hasSelection);
    m_removeAccountButton->setEnabled(hasSelection);
    m_testConnectionButton->setEnabled(hasSelection);
}

void SettingsDialog::onTestConnectionClicked()
{
    QListWidgetItem *item = m_accountsList->currentItem();
    if (!item) return;
    
    QString accountId = item->data(Qt::UserRole).toString();
    Account account = m_accountManager->getAccount(accountId);
    
    // TODO: Implement connection testing for each service
    QMessageBox::information(this, i18n("Test Connection"),
                            i18n("Connection testing for %1 accounts is not yet implemented.")
                            .arg(account.service));
}

void SettingsDialog::editAccount(const Account &account)
{
    AccountDialog dialog(account, this);
    if (dialog.exec() == QDialog::Accepted) {
        Account newAccount = dialog.getAccount();
        if (account.id.isEmpty()) {
            m_accountManager->addAccount(newAccount);
        } else {
            m_accountManager->updateAccount(newAccount);
        }
    }
}

void SettingsDialog::loadSettings()
{
    QSettings settings;
    
    m_startMinimizedCheck->setChecked(settings.value("General/StartMinimized", false).toBool());
    m_showNotificationsCheck->setChecked(settings.value("General/ShowNotifications", true).toBool());
    m_themeCombo->setCurrentIndex(settings.value("General/Theme", 0).toInt());
    
    // Load custom Nostr relays
    m_relaysList->clear();
    QStringList allRelays = settings.value("Nostr/CustomRelays", m_accountManager->getDefaultNostrRelays()).toStringList();
    for (const QString &relay : allRelays) {
        m_relaysList->addItem(relay);
    }
}

void SettingsDialog::saveSettings()
{
    QSettings settings;
    
    settings.setValue("General/StartMinimized", m_startMinimizedCheck->isChecked());
    settings.setValue("General/ShowNotifications", m_showNotificationsCheck->isChecked());
    settings.setValue("General/Theme", m_themeCombo->currentIndex());
    
    // Save custom Nostr relays
    QStringList relays;
    for (int i = 0; i < m_relaysList->count(); ++i) {
        relays.append(m_relaysList->item(i)->text());
    }
    settings.setValue("Nostr/CustomRelays", relays);
}

void SettingsDialog::onSaveClicked()
{
    saveSettings();
    accept();
}

void SettingsDialog::onCancelClicked()
{
    reject();
}
