#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QIcon>
#include <QSystemTrayIcon>
#include "windowstrayicon.h"


class QPushButton;


class Widget: public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = 0);
    ~Widget();

public slots:
    void onTrayIconActivated(QSystemTrayIcon::ActivationReason reason);
    void onTrayIconMouseHovered(int x, int y);

public slots:
    void on_addTrayIcon(bool checked = false);
    void on_removeTrayIcon(bool checked = false);
    void on_changeTip(bool checked = false);

    void on_addTrayIconOld(bool checked = false);
    void on_removeTrayIconOld(bool checked = false);
    void on_changeTipOld(bool checked = false);

protected:
    QPushButton *m_btnAddIcon;
    QPushButton *m_btnRemoveIcon;
    QPushButton *m_btnChangeTip;

    QPushButton *m_btnAddIconOld;
    QPushButton *m_btnRemoveIconOld;
    QPushButton *m_btnChangeTipOld;

    QIcon m_icon;
    WindowsTrayIcon *trayIcon;
    QSystemTrayIcon *trayIconOld;
};

#endif // WIDGET_H
