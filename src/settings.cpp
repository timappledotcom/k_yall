#include "settings.h"

Settings *Settings::s_instance = nullptr;

Settings::Settings(QObject *parent)
    : QObject(parent)
    , m_settings(new QSettings(this))
{
}

Settings* Settings::instance()
{
    if (!s_instance) {
        s_instance = new Settings();
    }
    return s_instance;
}

bool Settings::startMinimized() const
{
    return m_settings->value("General/StartMinimized", false).toBool();
}

void Settings::setStartMinimized(bool minimized)
{
    m_settings->setValue("General/StartMinimized", minimized);
    emit settingsChanged();
}

bool Settings::showNotifications() const
{
    return m_settings->value("General/ShowNotifications", true).toBool();
}

void Settings::setShowNotifications(bool show)
{
    m_settings->setValue("General/ShowNotifications", show);
    emit settingsChanged();
}

int Settings::theme() const
{
    return m_settings->value("General/Theme", 0).toInt();
}

void Settings::setTheme(int theme)
{
    m_settings->setValue("General/Theme", theme);
    emit settingsChanged();
}

QByteArray Settings::mainWindowGeometry() const
{
    return m_settings->value("MainWindow/Geometry").toByteArray();
}

void Settings::setMainWindowGeometry(const QByteArray &geometry)
{
    m_settings->setValue("MainWindow/Geometry", geometry);
}

QByteArray Settings::postWindowGeometry() const
{
    return m_settings->value("PostWindow/Geometry").toByteArray();
}

void Settings::setPostWindowGeometry(const QByteArray &geometry)
{
    m_settings->setValue("PostWindow/Geometry", geometry);
}
