#ifndef QTNETWORKPEER_H
#define QTNETWORKPEER_H

#include <QtCore>

class QtNetworkPeerPrivate;
class QtNetworkPeer : public QObject
{
Q_OBJECT
public:
    QtNetworkPeer();
    ~QtNetworkPeer();

    void setBroadcastEnabled(bool enable);
    void disconnectAllPeers();

    void sendMessage(const QByteArray &message);
Q_SIGNALS:
    void messageReady(const QByteArray &message);
private:
    QtNetworkPeerPrivate *d;
};

#endif
