#ifndef SETTINGS_H
#define SETTINGS_H

#include <QObject>
#include <QSettings>

class Settings : public QObject
{
    Q_OBJECT

public:
    static Settings* instance();
    
    // General settings
    bool startMinimized() const;
    void setStartMinimized(bool minimized);
    
    bool showNotifications() const;
    void setShowNotifications(bool show);
    
    int theme() const;
    void setTheme(int theme);
    
    // Window settings
    QByteArray mainWindowGeometry() const;
    void setMainWindowGeometry(const QByteArray &geometry);
    
    QByteArray postWindowGeometry() const;
    void setPostWindowGeometry(const QByteArray &geometry);

signals:
    void settingsChanged();

private:
    explicit Settings(QObject *parent = nullptr);
    QSettings *m_settings;
    static Settings *s_instance;
};

#endif // SETTINGS_H
