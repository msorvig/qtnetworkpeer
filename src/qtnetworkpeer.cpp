#include "qtnetworkpeer.h"
#include "qtnetworkpeer_p.h"

QtNetworkPeer::QtNetworkPeer()
    :d(new QtNetworkPeerPrivate(this))
{

}

QtNetworkPeer::~QtNetworkPeer()
{
    delete d;
}

void QtNetworkPeer::setBroadcastEnabled(bool enable)
{
    d->setBroadcastEnabled(enable);
}

void QtNetworkPeer::disconnectAllPeers()
{
    d->disconnectAllPeers();
}

void QtNetworkPeer::sendMessage(const QByteArray &message)
{
    d->sendMessage(message);
}
