#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QDebug>
#include "widget.h"
#include <time.h>


Widget::Widget(QWidget *parent): QWidget(parent) {
    QVBoxLayout *layout = new QVBoxLayout();
    QHBoxLayout *l1 = new QHBoxLayout();
    QHBoxLayout *l2 = new QHBoxLayout();

    m_btnAddIcon = new QPushButton(tr("Add tray icon"), this);
    m_btnRemoveIcon = new QPushButton(tr("Remove tray icon"), this);
    m_btnChangeTip = new QPushButton(tr("Change tooltip"), this);
    connect(m_btnAddIcon, &QPushButton::clicked, this, &Widget::on_addTrayIcon);
    connect(m_btnRemoveIcon, &QPushButton::clicked, this, &Widget::on_removeTrayIcon);
    connect(m_btnChangeTip, &QPushButton::clicked, this, &Widget::on_changeTip);

    m_btnAddIconOld = new QPushButton(tr("Add old tray icon"), this);
    m_btnRemoveIconOld = new QPushButton(tr("Remove old tray icon"), this);
    m_btnChangeTipOld = new QPushButton(tr("Change old tooltip"), this);
    connect(m_btnAddIconOld, &QPushButton::clicked, this, &Widget::on_addTrayIconOld);
    connect(m_btnRemoveIconOld, &QPushButton::clicked, this, &Widget::on_removeTrayIconOld);
    connect(m_btnChangeTipOld, &QPushButton::clicked, this, &Widget::on_changeTipOld);

    l1->addStretch();
    l1->addWidget(m_btnAddIcon);
    l1->addWidget(m_btnRemoveIcon);
    l1->addWidget(m_btnChangeTip);
    l1->addStretch();

    l2->addStretch();
    l2->addWidget(m_btnAddIconOld);
    l2->addWidget(m_btnRemoveIconOld);
    l2->addWidget(m_btnChangeTipOld);
    l2->addStretch();

    layout->addLayout(l1);
    layout->addLayout(l2);

    this->setLayout(layout);

    m_icon = QIcon(":/icon.png");
    setWindowIcon(m_icon);

    // test tray icon
    trayIcon = new WindowsTrayIcon(this);
    trayIcon->setIcon(m_icon);
    connect(trayIcon, &WindowsTrayIcon::activated,
            this, &Widget::onTrayIconActivated);
    connect(trayIcon, &WindowsTrayIcon::mouseHovered,
            this, &Widget::onTrayIconMouseHovered);

    // oldschool icon
    trayIconOld = new QSystemTrayIcon(this);
    trayIconOld->setIcon(m_icon);
    connect(trayIconOld, &QSystemTrayIcon::activated,
            this, &Widget::onTrayIconActivated);
}

Widget::~Widget() {
    //
}


void Widget::onTrayIconActivated(QSystemTrayIcon::ActivationReason reason) {
    qDebug() << "activated(), reason:" << (uint)reason;
}


void Widget::onTrayIconMouseHovered(int x, int y) {
    qDebug() << "hovered(), hovered:" << x << y;
}


void Widget::on_addTrayIcon(bool checked) {
    Q_UNUSED(checked)
    qDebug() << "add";
    trayIcon->show();
}


void Widget::on_removeTrayIcon(bool checked) {
    Q_UNUSED(checked)
    qDebug() << "remove";
    trayIcon->hide();
}


void Widget::on_changeTip(bool checked) {
    Q_UNUSED(checked)
    quint32 tmnow = (quint32)time(NULL);
    QString tip = QString("Unix ts: %1").arg(tmnow);
    trayIcon->setToolTip(tip);
}



void Widget::on_addTrayIconOld(bool checked) {
    Q_UNUSED(checked)
    qDebug() << "add old";
    trayIconOld->show();
}


void Widget::on_removeTrayIconOld(bool checked) {
    Q_UNUSED(checked)
    qDebug() << "remove old";
    trayIconOld->hide();
}


void Widget::on_changeTipOld(bool checked) {
    Q_UNUSED(checked)
    quint32 tmnow = (quint32)time(NULL);
    QString tip = QString("Unix ts: %1").arg(tmnow);
    trayIconOld->setToolTip(tip);
}
