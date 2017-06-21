#include <QtGlobal>  // for the definition of Q_OS_WIN

#ifdef Q_OS_WIN  // do not compile the whole .cpp file on platforms other than windows

#include <QCoreApplication>
#include <QWidget>
#include <QPixmap>
#include <QAbstractNativeEventFilter>
#include <QHash>
#include <QtWin>
#include <QDebug>

#include <string>

#include "windowstrayicon.h"


#define _WIN32_WINNT _WIN32_WINNT_VISTA
#define _WIN32_IE _WIN32_IE_LONGHORN
#define NTDDI_VERSION NTDDI_VISTA
#include <Windows.h>
#include <windowsx.h>
#include <shellapi.h>
#define WM_TRAYMESSAGE (WM_APP + 101)


class WindowsTrayMessageFilter: public QAbstractNativeEventFilter {
protected:
    WindowsTrayMessageFilter() {
        next_iconId = 1;
    }

    static WindowsTrayMessageFilter *s_instance;
    uint next_iconId = 0;
    QHash<uint, WindowsTrayIconPrivate *> handlers;

public:
    static WindowsTrayMessageFilter *instance() {
        if (s_instance == nullptr) {
            s_instance = new WindowsTrayMessageFilter();
            QCoreApplication::instance()->installNativeEventFilter(s_instance);
        }
        return s_instance;
    }

    static uint nextIconId() {
        instance()->next_iconId++;
        return s_instance->next_iconId;
    }

    static void addCallbackHandler(uint iconId, WindowsTrayIconPrivate *handler) {
        instance()->handlers.insert(iconId, handler);
    }

    virtual ~WindowsTrayMessageFilter() {
        //
    }

public:
    virtual bool nativeEventFilter(const QByteArray &eventType, void *message, long *result) {
        if ((eventType == "windows_generic_MSG") || (eventType == "windows_dispatcher_MSG")) {
            MSG *msg = static_cast<MSG *>(message);
            if (result) *result = 0;

            if (msg->message == WM_TRAYMESSAGE) {
                // When the uVersion member is NOTIFYICON_VERSION_4, the interpretation of the
                // lParam and wParam parameters of that message is changed as follows:
                // - LOWORD(lParam) contains notify events, such as NIN_BALLOONSHOW, NIN_POPUPOPEN, or WM_CONTEXTMENU.
                // - HIWORD(lParam) contains the icon ID. Icon IDs are restricted to a length of 16 bits.
                // - GET_X_LPARAM(wParam) returns the X anchor coordinate for notification events
                //   NIN_POPUPOPEN, NIN_SELECT, NIN_KEYSELECT, and all mouse messages between
                //   WM_MOUSEFIRST and WM_MOUSELAST. If any of those messages are generated by
                //   the keyboard, wParam is set to the upper-left corner of the target icon.
                //   For all other messages, wParam is undefined.
                uint iconId = HIWORD(msg->lParam);
                // call message handler for this icon
                if (iconId > 0)
                    this->call_handler(iconId, msg);
                return true;
            }
        }
        return false;
    }

protected:
    void call_handler(uint iconId, MSG *msg);
};


// static
WindowsTrayMessageFilter *WindowsTrayMessageFilter::s_instance = nullptr;


class WindowsTrayIconPrivate {
public:
    explicit WindowsTrayIconPrivate(WindowsTrayIcon *roTrayIcon, QWidget *widget) {
        ownerIcon = roTrayIcon;
        ownerWidget = widget;
        ownerHwnd = (HWND)widget->winId();
        visible = false;
        iconId = (UINT)WindowsTrayMessageFilter::nextIconId();
        WindowsTrayMessageFilter::addCallbackHandler(iconId, this);
        //
        //qDebug() << "WindowsTrayIconPrivate: new icon with id = " << iconId;
    }

    void init_notifyIconData(NOTIFYICONDATAW *pData) {
        memset(pData, 0, sizeof(NOTIFYICONDATAW));
        pData->cbSize = sizeof(NOTIFYICONDATAW);
        pData->hWnd = ownerHwnd;
        pData->uID = iconId;
        pData->uFlags = NIF_MESSAGE | NIF_TIP;
        pData->uCallbackMessage = WM_TRAYMESSAGE;
        pData->uVersion = NOTIFYICON_VERSION_4;
        if (!toolTip.isEmpty()) {
            std::wstring wstr = toolTip.toStdWString();
            wcsncpy(pData->szTip, wstr.c_str(), sizeof(pData->szTip)/sizeof(WCHAR) - 1);
        }
        if (!icon.isNull()) {
            pData->uFlags |= NIF_ICON;
            // setup icon. convert QIcon to HICON (using QWinExtras)
            pData->hIcon = QtWin::toHICON(icon.pixmap(32, 32));
        }
    }

    void add_trayIcon() {
        NOTIFYICONDATAW ndata;
        init_notifyIconData(&ndata);
        BOOL ret = FALSE;
        ret = Shell_NotifyIconW(NIM_ADD, &ndata);
        Q_ASSERT(ret == TRUE);
        ret = Shell_NotifyIconW(NIM_SETVERSION, &ndata);
        Q_ASSERT(ret == TRUE);
    }

    void remove_trayIcon() {
        NOTIFYICONDATAW ndata;
        init_notifyIconData(&ndata);
        BOOL ret = FALSE;
        ret = Shell_NotifyIconW(NIM_DELETE, &ndata);
        Q_ASSERT(ret == TRUE);
    }

    void edit_trayIcon() {
        NOTIFYICONDATAW ndata;
        init_notifyIconData(&ndata);
        BOOL ret = FALSE;
        ret = Shell_NotifyIconW(NIM_MODIFY, &ndata);
        Q_ASSERT(ret == TRUE);
    }

    void setVisible(bool vis) {
        if (vis && visible) return; // already visible
        if (!vis && !visible) return; // already invisible
        visible = vis;
        if (visible) {
            // make visible
            add_trayIcon();
        } else {
            // make invisible
            remove_trayIcon();
        }
    }

    void setToolTip(const QString& tip) {
        toolTip = tip;
        if (visible) edit_trayIcon();
    }

    void setIcon(const QIcon& ico) {
        icon = ico;
        if (visible) edit_trayIcon();
    }

    void on_activate(QSystemTrayIcon::ActivationReason reason) {
        emit ownerIcon->activated(reason);
    }

    void on_mouseMove(int x, int y) {
        emit ownerIcon->mouseHovered(x, y);
    }


    WindowsTrayIcon *ownerIcon;
    QWidget *ownerWidget;
    HWND ownerHwnd;
    UINT iconId;

    QIcon icon;
    QString toolTip;
    bool visible;

private:
    Q_DISABLE_COPY(WindowsTrayIconPrivate)
};


// this function needs to be below ROWindowsTrayIconPrivate class declaration
void WindowsTrayMessageFilter::call_handler(uint iconId, MSG *msg) {
    // first, find handler for this tray icon
    if (!handlers.contains(iconId)) {
        qDebug() << "No handler for icon id: " << iconId;
        return;
    }
    WindowsTrayIconPrivate *handler = handlers[iconId];
    if (!handler) return; // avoid dereference nullptr
    //
    uint notify_event = LOWORD(msg->lParam);
    int x = GET_X_LPARAM(msg->wParam);
    int y = GET_Y_LPARAM(msg->wParam);
    switch (notify_event) {
    case WM_MOUSEMOVE:
        handler->on_mouseMove(x, y);
        break;
    case WM_LBUTTONUP:
        handler->on_activate(QSystemTrayIcon::Trigger);
        break;
    case WM_CONTEXTMENU:
        handler->on_activate(QSystemTrayIcon::Context);
        break;
    case WM_MBUTTONUP:
        handler->on_activate(QSystemTrayIcon::MiddleClick);
        break;
    case WM_LBUTTONDBLCLK:
        handler->on_activate(QSystemTrayIcon::DoubleClick);
        break;
    default:
        break;
    }
}


WindowsTrayIcon::WindowsTrayIcon(QWidget *parent):
    QObject(parent), d(new WindowsTrayIconPrivate(this, parent))
{
    //
}


WindowsTrayIcon::WindowsTrayIcon(const QIcon &icon, QWidget *parent):
    QObject(parent), d(new WindowsTrayIconPrivate(this, parent))
{
    setIcon(icon);
}


WindowsTrayIcon::~WindowsTrayIcon() {
    if (isVisible()) setVisible(false);
    delete d;
}


QIcon WindowsTrayIcon::icon() const {
    return d->icon;
}


QString WindowsTrayIcon::toolTip() const {
    return d->toolTip;
}


bool WindowsTrayIcon::isVisible() const {
    return d->visible;
}


void WindowsTrayIcon::setIcon(const QIcon &icon) {
    d->setIcon(icon);
}


void WindowsTrayIcon::setVisible(bool visible) {
    d->setVisible(visible);
}


void WindowsTrayIcon::setToolTip(const QString &tip) {
    d->setToolTip(tip);
}


bool WindowsTrayIcon::isSystemTrayAvailable() { return true; }

bool WindowsTrayIcon::supportsMessages() { return true; }

void WindowsTrayIcon::showMessage(const QString &title, const QString &msg,
        QSystemTrayIcon::MessageIcon icon, int msecs) {
    Q_UNUSED(title)
    Q_UNUSED(msg)
    Q_UNUSED(icon)
    Q_UNUSED(msecs)
    return;
}

#endif  // Q_OS_WIN