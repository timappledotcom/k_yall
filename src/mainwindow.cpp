#include "mainwindow.h"
#include "postwidget.h"
#include "accountmanager.h"
#include "settingsdialog.h"
#include <QApplication>
#include <QCloseEvent>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QAction>
#include <QMessageBox>
#include <KLocalizedString>
#include <KStatusNotifierItem>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_trayIcon(nullptr)
    , m_trayIconMenu(nullptr)
    , m_postWidget(nullptr)
    , m_accountManager(nullptr)
    , m_settingsDialog(nullptr)
{
    setWindowTitle(i18n("K, Y'all"));
    setWindowIcon(QIcon(":/icons/kyall.svg"));
    setFixedSize(400, 300);
    
    m_accountManager = new AccountManager(this);
    
    setupUI();
    createTrayIcon();
    createMenus();
    
    // Check for and migrate plain text credentials
    checkCredentialSecurity();
    
    // Hide on startup, let tray icon handle showing
    hide();
}

MainWindow::~MainWindow() = default;

void MainWindow::setupUI()
{
    m_centralWidget = new QWidget(this);
    setCentralWidget(m_centralWidget);
    
    m_mainLayout = new QVBoxLayout(m_centralWidget);
    
    m_welcomeLabel = new QLabel(i18n("Welcome to K, Y'all!"), this);
    m_welcomeLabel->setAlignment(Qt::AlignCenter);
    m_welcomeLabel->setStyleSheet("font-size: 18px; font-weight: bold; margin: 20px;");
    
    m_newPostButton = new QPushButton(i18n("New Post"), this);
    m_newPostButton->setMinimumHeight(40);
    
    m_manageAccountsButton = new QPushButton(i18n("Manage Accounts"), this);
    m_manageAccountsButton->setMinimumHeight(40);
    
    m_mainLayout->addWidget(m_welcomeLabel);
    m_mainLayout->addStretch();
    m_mainLayout->addWidget(m_newPostButton);
    m_mainLayout->addWidget(m_manageAccountsButton);
    m_mainLayout->addStretch();
    
    connect(m_newPostButton, &QPushButton::clicked, this, &MainWindow::showPostWindow);
    connect(m_manageAccountsButton, &QPushButton::clicked, this, &MainWindow::showSettings);
}

void MainWindow::createTrayIcon()
{
    m_trayIcon = new KStatusNotifierItem(this);
    m_trayIcon->setIconByPixmap(QIcon(":/icons/kyall.svg").pixmap(32, 32));
    m_trayIcon->setTitle(i18n("K, Y'all"));
    m_trayIcon->setToolTip(QIcon(":/icons/kyall.svg"), i18n("K, Y'all"), i18n("Social posting app"));
    m_trayIcon->setStatus(KStatusNotifierItem::Active);
    
    connect(m_trayIcon, &KStatusNotifierItem::activateRequested,
            this, &MainWindow::showPostWindow);
}

void MainWindow::createMenus()
{
    m_trayIconMenu = new QMenu(this);
    
    m_postAction = new QAction(i18n("New Post"), this);
    m_postAction->setIcon(QIcon::fromTheme("mail-message-new"));
    connect(m_postAction, &QAction::triggered, this, &MainWindow::showPostWindow);
    
    m_settingsAction = new QAction(i18n("Settings"), this);
    m_settingsAction->setIcon(QIcon::fromTheme("configure"));
    connect(m_settingsAction, &QAction::triggered, this, &MainWindow::showSettings);
    
    m_aboutAction = new QAction(i18n("About"), this);
    m_aboutAction->setIcon(QIcon::fromTheme("help-about"));
    connect(m_aboutAction, &QAction::triggered, this, &MainWindow::showAbout);
    
    m_quitAction = new QAction(i18n("Quit"), this);
    m_quitAction->setIcon(QIcon::fromTheme("application-exit"));
    connect(m_quitAction, &QAction::triggered, this, &MainWindow::quit);
    
    m_trayIconMenu->addAction(m_postAction);
    m_trayIconMenu->addSeparator();
    m_trayIconMenu->addAction(m_settingsAction);
    m_trayIconMenu->addAction(m_aboutAction);
    m_trayIconMenu->addSeparator();
    m_trayIconMenu->addAction(m_quitAction);
    
    m_trayIcon->setContextMenu(m_trayIconMenu);
}

void MainWindow::showPostWindow()
{
    if (!m_postWidget) {
        m_postWidget = new PostWidget(m_accountManager);
    }
    
    // Clear the form for a fresh start
    m_postWidget->clearForm();
    
    m_postWidget->show();
    m_postWidget->raise();
    m_postWidget->activateWindow();
    
    // Hide main window when showing post widget
    hide();
}

void MainWindow::showSettings()
{
    if (!m_settingsDialog) {
        m_settingsDialog = new SettingsDialog(m_accountManager, this);
    }
    
    m_settingsDialog->show();
    m_settingsDialog->raise();
    m_settingsDialog->activateWindow();
}

void MainWindow::showAbout()
{
    QMessageBox::about(this, i18n("About K, Y'all"),
                      i18n("K, Y'all is a social posting app for KDE and Plasma.\n\n"
                           "Features:\n"
                           "• Post to Mastodon, BlueSky, MicroBlog, and Nostr\n"
                           "• Multiple account support\n"
                           "• Image uploads\n"
                           "• System tray integration\n\n"
                           "Version 1.0.0"));
}

void MainWindow::onTrayActivated(QSystemTrayIcon::ActivationReason reason)
{
    switch (reason) {
    case QSystemTrayIcon::Trigger:
    case QSystemTrayIcon::DoubleClick:
        showPostWindow();
        break;
    default:
        break;
    }
}

void MainWindow::quit()
{
    QApplication::quit();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (m_trayIcon && m_trayIcon->status() == KStatusNotifierItem::Active) {
        hide();
        event->ignore();
    } else {
        event->accept();
    }
}

void MainWindow::checkCredentialSecurity()
{
    if (m_accountManager->hasPlainTextCredentials()) {
        QMessageBox::StandardButton reply = QMessageBox::question(
            this,
            i18n("Security Update Available"),
            i18n("K, Y'all has detected credentials stored in plain text from a previous version.\n\n"
                 "For your security, these credentials should be migrated to encrypted storage.\n\n"
                 "This is a one-time migration that will encrypt your access tokens and private keys.\n\n"
                 "Would you like to migrate your credentials now?"),
            QMessageBox::Yes | QMessageBox::No,
            QMessageBox::Yes
        );
        
        if (reply == QMessageBox::Yes) {
            m_accountManager->migrateToSecureStorage();
            QMessageBox::information(
                this,
                i18n("Migration Complete"),
                i18n("Your credentials have been successfully migrated to encrypted storage.\n\n"
                     "All access tokens and private keys are now stored securely.")
            );
        }
    }
}
