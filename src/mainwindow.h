#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QAction>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <KStatusNotifierItem>

class PostWidget;
class AccountManager;
class SettingsDialog;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void showPostWindow();
    void showSettings();
    void showAbout();
    void onTrayActivated(QSystemTrayIcon::ActivationReason reason);
    void quit();

private:
    void createTrayIcon();
    void createMenus();
    void setupUI();
    void checkCredentialSecurity();

    KStatusNotifierItem *m_trayIcon;
    QMenu *m_trayIconMenu;
    QAction *m_postAction;
    QAction *m_settingsAction;
    QAction *m_aboutAction;
    QAction *m_quitAction;
    
    PostWidget *m_postWidget;
    AccountManager *m_accountManager;
    SettingsDialog *m_settingsDialog;
    
    QWidget *m_centralWidget;
    QVBoxLayout *m_mainLayout;
    QLabel *m_welcomeLabel;
    QPushButton *m_newPostButton;
    QPushButton *m_manageAccountsButton;
};

#endif // MAINWINDOW_H
