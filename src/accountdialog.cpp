#include "accountdialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QComboBox>
#include <QLineEdit>
#include <QTextEdit>
#include <QCheckBox>
#include <QLabel>
#include <QPushButton>
#include <QMessageBox>
#include <QStackedWidget>
#include <QPushButton>
#include <QStackedWidget>
#include <QGroupBox>
#include <QLabel>
#include <QMessageBox>
#include <QUuid>
#include <KLocalizedString>

AccountDialog::AccountDialog(const Account &account, QWidget *parent)
    : QDialog(parent)
    , m_originalAccount(account)
{
    setWindowTitle(account.id.isEmpty() ? i18n("Add Account") : i18n("Edit Account"));
    setModal(true);
    resize(500, 400);
    
    setupUI();
    loadAccount(account);
}

void AccountDialog::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    
    // Service selection
    m_formLayout = new QFormLayout();
    
    m_serviceCombo = new QComboBox(this);
    m_serviceCombo->addItem("Mastodon", "mastodon");
    m_serviceCombo->addItem("BlueSky", "bluesky");
    m_serviceCombo->addItem("MicroBlog", "microblog");
    m_serviceCombo->addItem("Nostr", "nostr");
    m_serviceCombo->addItem("Test Service", "test");
    
    m_displayNameEdit = new QLineEdit(this);
    m_usernameEdit = new QLineEdit(this);
    
    m_formLayout->addRow(i18n("Service:"), m_serviceCombo);
    m_formLayout->addRow(i18n("Display Name:"), m_displayNameEdit);
    m_formLayout->addRow(i18n("Username:"), m_usernameEdit);
    
    // Service-specific forms
    m_serviceStack = new QStackedWidget(this);
    
    setupMastodonForm();
    setupBlueSkyForm();
    setupMicroBlogForm();
    setupNostrForm();
    setupTestForm();
    
    // Options
    QGroupBox *optionsGroup = new QGroupBox(i18n("Options"), this);
    QVBoxLayout *optionsLayout = new QVBoxLayout(optionsGroup);
    
    m_enabledCheck = new QCheckBox(i18n("Account enabled"), this);
    m_enabledCheck->setChecked(true);
    
    m_defaultPostingCheck = new QCheckBox(i18n("Use as default for posting"), this);
    
    optionsLayout->addWidget(m_enabledCheck);
    optionsLayout->addWidget(m_defaultPostingCheck);
    
    // Buttons
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    m_testButton = new QPushButton(i18n("Test Connection"), this);
    m_saveButton = new QPushButton(i18n("Save"), this);
    m_cancelButton = new QPushButton(i18n("Cancel"), this);
    
    buttonLayout->addWidget(m_testButton);
    buttonLayout->addStretch();
    buttonLayout->addWidget(m_saveButton);
    buttonLayout->addWidget(m_cancelButton);
    
    // Layout assembly
    m_mainLayout->addLayout(m_formLayout);
    m_mainLayout->addWidget(m_serviceStack);
    m_mainLayout->addWidget(optionsGroup);
    m_mainLayout->addLayout(buttonLayout);
    
    // Connections
    connect(m_serviceCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &AccountDialog::onServiceChanged);
    connect(m_testButton, &QPushButton::clicked,
            this, &AccountDialog::onTestConnectionClicked);
    connect(m_saveButton, &QPushButton::clicked, [this]() {
        if (validateForm()) {
            accept();
        }
    });
    connect(m_cancelButton, &QPushButton::clicked, this, &QDialog::reject);
    
    onServiceChanged(); // Initialize form
}

void AccountDialog::setupMastodonForm()
{
    m_mastodonWidget = new QWidget();
    QFormLayout *layout = new QFormLayout(m_mastodonWidget);
    
    m_mastodonServerEdit = new QLineEdit(this);
    m_mastodonServerEdit->setPlaceholderText("https://mastodon.social");
    
    m_mastodonTokenEdit = new QLineEdit(this);
    m_mastodonTokenEdit->setEchoMode(QLineEdit::Password);
    
    m_mastodonOAuthButton = new QPushButton(i18n("Get Token via OAuth"), this);
    
    layout->addRow(i18n("Server URL:"), m_mastodonServerEdit);
    layout->addRow(i18n("Access Token:"), m_mastodonTokenEdit);
    layout->addRow("", m_mastodonOAuthButton);
    
    m_serviceStack->addWidget(m_mastodonWidget);
    
    connect(m_mastodonOAuthButton, &QPushButton::clicked,
            this, &AccountDialog::onOAuthClicked);
}

void AccountDialog::setupBlueSkyForm()
{
    m_blueSkyWidget = new QWidget();
    QFormLayout *layout = new QFormLayout(m_blueSkyWidget);
    
    m_blueSkyPasswordEdit = new QLineEdit(this);
    m_blueSkyPasswordEdit->setEchoMode(QLineEdit::Password);
    
    QLabel *helpLabel = new QLabel(i18n("Use your BlueSky app password, not your main password."), this);
    helpLabel->setWordWrap(true);
    helpLabel->setStyleSheet("color: orange; font-style: italic;");
    
    layout->addRow(i18n("App Password:"), m_blueSkyPasswordEdit);
    layout->addRow("", helpLabel);
    
    m_serviceStack->addWidget(m_blueSkyWidget);
}

void AccountDialog::setupMicroBlogForm()
{
    // MicroBlog uses the same form as Mastodon for now
    m_serviceStack->addWidget(m_mastodonWidget);
}

void AccountDialog::setupNostrForm()
{
    m_nostrWidget = new QWidget();
    QFormLayout *layout = new QFormLayout(m_nostrWidget);
    
    m_nostrPrivateKeyEdit = new QLineEdit(this);
    m_nostrPrivateKeyEdit->setEchoMode(QLineEdit::Password);
    
    m_generateKeyButton = new QPushButton(i18n("Generate New Key"), this);
    
    m_nostrRelaysEdit = new QTextEdit(this);
    m_nostrRelaysEdit->setMaximumHeight(100);
    m_nostrRelaysEdit->setPlainText(
        "wss://relay.damus.io\n"
        "wss://nos.lol\n"
        "wss://relay.snort.social\n"
        "wss://relay.current.fyi\n"
        "wss://brb.io"
    );
    
    layout->addRow(i18n("Private Key:"), m_nostrPrivateKeyEdit);
    layout->addRow("", m_generateKeyButton);
    layout->addRow(i18n("Relays (one per line):"), m_nostrRelaysEdit);
    
    m_serviceStack->addWidget(m_nostrWidget);
    
    connect(m_generateKeyButton, &QPushButton::clicked, [this]() {
        // Generate a mock private key (in real implementation, use proper secp256k1)
        QString newKey = QUuid::createUuid().toString(QUuid::WithoutBraces);
        m_nostrPrivateKeyEdit->setText(newKey);
    });
}

void AccountDialog::setupTestForm()
{
    m_testWidget = new QWidget;
    QFormLayout *testLayout = new QFormLayout(m_testWidget);
    QLabel *testLabel = new QLabel(tr("Test service - posts will not be sent anywhere, just for testing the UI"));
    testLabel->setWordWrap(true);
    testLabel->setStyleSheet("color: blue; font-style: italic;");
    testLayout->addWidget(testLabel);
    
    m_serviceStack->addWidget(m_testWidget);
}

void AccountDialog::loadAccount(const Account &account)
{
    if (account.id.isEmpty()) {
        return; // New account
    }
    
    // Set service
    int serviceIndex = m_serviceCombo->findData(account.service);
    if (serviceIndex >= 0) {
        m_serviceCombo->setCurrentIndex(serviceIndex);
    }
    
    m_displayNameEdit->setText(account.displayName);
    m_usernameEdit->setText(account.username);
    m_enabledCheck->setChecked(account.enabled);
    m_defaultPostingCheck->setChecked(account.defaultForPosting);
    
    // Service-specific fields
    if (account.service == "mastodon" || account.service == "microblog") {
        m_mastodonServerEdit->setText(account.serverUrl);
        m_mastodonTokenEdit->setText(account.accessToken);
    } else if (account.service == "bluesky") {
        m_blueSkyPasswordEdit->setText(account.accessToken);
    } else if (account.service == "nostr") {
        m_nostrPrivateKeyEdit->setText(account.privateKey);
        m_nostrRelaysEdit->setPlainText(account.relays.join("\n"));
    }
}

Account AccountDialog::createAccountFromForm() const
{
    Account account = m_originalAccount; // Keep existing ID if editing
    
    account.service = m_serviceCombo->currentData().toString();
    account.displayName = m_displayNameEdit->text().trimmed();
    account.username = m_usernameEdit->text().trimmed();
    account.enabled = m_enabledCheck->isChecked();
    account.defaultForPosting = m_defaultPostingCheck->isChecked();
    
    // Service-specific fields
    if (account.service == "mastodon" || account.service == "microblog") {
        account.serverUrl = m_mastodonServerEdit->text().trimmed();
        account.accessToken = m_mastodonTokenEdit->text().trimmed();
    } else if (account.service == "bluesky") {
        account.serverUrl = "https://bsky.social"; // Default
        account.accessToken = m_blueSkyPasswordEdit->text().trimmed();
    } else if (account.service == "nostr") {
        account.privateKey = m_nostrPrivateKeyEdit->text().trimmed();
        QString relaysText = m_nostrRelaysEdit->toPlainText();
        account.relays = relaysText.split('\n', Qt::SkipEmptyParts);
        
        // Ensure wss:// prefix
        for (QString &relay : account.relays) {
            relay = relay.trimmed();
            if (!relay.startsWith("wss://") && !relay.startsWith("ws://")) {
                relay = "wss://" + relay;
            }
        }
    }
    
    return account;
}

bool AccountDialog::validateForm() const
{
    if (m_usernameEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(const_cast<AccountDialog*>(this), i18n("Validation Error"),
                            i18n("Username is required."));
        return false;
    }
    
    QString service = m_serviceCombo->currentData().toString();
    
    if (service == "mastodon" || service == "microblog") {
        if (m_mastodonServerEdit->text().trimmed().isEmpty()) {
            QMessageBox::warning(const_cast<AccountDialog*>(this), i18n("Validation Error"),
                                i18n("Server URL is required."));
            return false;
        }
        if (m_mastodonTokenEdit->text().trimmed().isEmpty()) {
            QMessageBox::warning(const_cast<AccountDialog*>(this), i18n("Validation Error"),
                                i18n("Access token is required."));
            return false;
        }
    } else if (service == "bluesky") {
        if (m_blueSkyPasswordEdit->text().trimmed().isEmpty()) {
            QMessageBox::warning(const_cast<AccountDialog*>(this), i18n("Validation Error"),
                                i18n("App password is required."));
            return false;
        }
    } else if (service == "nostr") {
        if (m_nostrPrivateKeyEdit->text().trimmed().isEmpty()) {
            QMessageBox::warning(const_cast<AccountDialog*>(this), i18n("Validation Error"),
                                i18n("Private key is required."));
            return false;
        }
    }
    
    return true;
}

Account AccountDialog::getAccount() const
{
    return createAccountFromForm();
}

void AccountDialog::onServiceChanged()
{
    QString service = m_serviceCombo->currentData().toString();
    
    if (service == "mastodon") {
        m_serviceStack->setCurrentWidget(m_mastodonWidget);
    } else if (service == "bluesky") {
        m_serviceStack->setCurrentWidget(m_blueSkyWidget);
    } else if (service == "microblog") {
        m_serviceStack->setCurrentWidget(m_mastodonWidget); // Reuse Mastodon form
    } else if (service == "nostr") {
        m_serviceStack->setCurrentWidget(m_nostrWidget);
    } else if (service == "test") {
        m_serviceStack->setCurrentWidget(m_testWidget);
    }
}

void AccountDialog::onOAuthClicked()
{
    QMessageBox::information(this, i18n("OAuth Authentication"),
                            i18n("OAuth authentication is not yet implemented.\n\n"
                                 "Please obtain an access token from your Mastodon instance:\n"
                                 "1. Go to Settings > Development\n"
                                 "2. Create a new application\n"
                                 "3. Copy the access token and paste it here"));
}

void AccountDialog::onTestConnectionClicked()
{
    if (!validateForm()) {
        return;
    }
    
    Account account = createAccountFromForm();
    
    // TODO: Implement actual connection testing
    QMessageBox::information(this, i18n("Test Connection"),
                            i18n("Connection testing for %1 accounts is not yet implemented.")
                            .arg(account.service));
}
