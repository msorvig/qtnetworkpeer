#ifndef RECEIVER_H
#define RECEIVER_H

#include <QtCore>
#include <QtWidgets>

#include <qtnetworkpeer.h>

class Receiver : public QWidget
{
Q_OBJECT
public:
    Receiver();
private Q_SLOTS:
    void newMessage(const QByteArray &message);
    void disconnect();
private:
    QtNetworkPeer networkpeer;
    QLabel *label;
    QPushButton *disconnectButton;
};

#endif
