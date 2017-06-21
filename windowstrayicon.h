#ifndef CUSTOM_WINDOWSTRAYICON_H
#define CUSTOM_WINDOWSTRAYICON_H

#include <QObject>
#include <QIcon>
#include <QSystemTrayIcon>

class QWidget;
class QMenu;
class WindowsTrayIconPrivate;

#ifdef Q_OS_WIN

/**
 * @brief The ROWindowsTrayIcon class
 * Replaces QSystemTrayIcon: has the same methods, signals and slots,
 * but adds extra signal:
 *
 *   mouseHovered() - emitted when mouse hovers over tray icon
 */
class WindowsTrayIcon: public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString toolTip READ toolTip WRITE setToolTip)
    Q_PROPERTY(QIcon icon READ icon WRITE setIcon)
    Q_PROPERTY(bool visible READ isVisible WRITE setVisible DESIGNABLE false)

public:
    WindowsTrayIcon(QWidget *parent = Q_NULLPTR);
    WindowsTrayIcon(const QIcon &icon, QWidget *parent = Q_NULLPTR);
    ~WindowsTrayIcon();

    QIcon icon() const;
    void setIcon(const QIcon &icon);

    QString toolTip() const;
    bool isVisible() const;

#ifndef QT_NO_MENU
    // context menu not supported
    void setContextMenu(QMenu *menu) { Q_UNUSED(menu); }
    QMenu *contextMenu() const { return Q_NULLPTR; }
#endif

    static bool isSystemTrayAvailable();  // returns true
    static bool supportsMessages();

public Q_SLOTS:
    void setVisible(bool visible);
    inline void show() { setVisible(true); }
    inline void hide() { setVisible(false); }
    void showMessage(const QString &title, const QString &msg,
                     QSystemTrayIcon::MessageIcon icon = QSystemTrayIcon::Information,
                     int msecs = 10000);

    void setToolTip(const QString &tip);

Q_SIGNALS:
    void activated(QSystemTrayIcon::ActivationReason reason);
    void messageClicked();  // possibly never emitted in this implementation
    void mouseHovered(int x, int y);  // emitted when mouse moves over tray icon

protected:
    WindowsTrayIconPrivate *d;

private:
    Q_DISABLE_COPY(WindowsTrayIcon)
};

#else // not Q_OS_WIN

// on other platforms just use directly QSystemTrayIcon

class WindowsTrayIcon: public QSystemTrayIcon {
public:
    Q_OBJECT
    WindowsTrayIcon(QWidget *parent = Q_NULLPTR):
        QSystemTrayIcon(parent) {
    }
    WindowsTrayIcon(const QIcon &icon, QWidget *parent = Q_NULLPTR):
        QSystemTrayIcon(icon, parent) {
    }
    ~WindowsTrayIcon() { }

private:
    Q_DISABLE_COPY(WindowsTrayIcon)
}

#endif // Q_OS_WIN

#endif // CUSTOM_WINDOWSTRAYICON_H
