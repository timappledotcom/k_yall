#include <QApplication>
#include <QSystemTrayIcon>
#include <QMessageBox>
#include <KAboutData>
#include <KLocalizedString>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    KAboutData aboutData(
        QStringLiteral("kyall"),
        i18n("K, Y'all"),
        QStringLiteral("1.0.0"),
        i18n("Social posting app for KDE and Plasma"),
        KAboutLicense::GPL_V3,
        i18n("© 2025"),
        QString(),
        QStringLiteral("https://github.com/yourrepo/kyall")
    );
    
    KAboutData::setApplicationData(aboutData);
    app.setQuitOnLastWindowClosed(false);
    
    if (!QSystemTrayIcon::isSystemTrayAvailable()) {
        QMessageBox::critical(nullptr, QObject::tr("K, Y'all"),
                            QObject::tr("System tray is not available on this system."));
        return 1;
    }
    
    MainWindow window;
    window.show();
    
    return app.exec();
}
