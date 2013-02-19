#ifndef SENDER_H
#define SENDER_H

#include <QtCore>
#include <QtWidgets>

#include <qtnetworkpeer.h>

class Sender : public QWidget
{
Q_OBJECT
public:
    Sender();
private Q_SLOTS:
    void sendMessage();
    void disconnect();
private:
    QtNetworkPeer networkpeer;
    QTimer timer;
    QPushButton *disconnectButton;
};

#endif
