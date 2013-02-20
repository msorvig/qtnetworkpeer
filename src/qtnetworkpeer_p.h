#ifndef QTNETWORKPEER_P_H
#define QTNETWORKPEER_P_H

#include <QtCore>
#include <QtNetwork/QtNetwork>

#include "qtnetworkpeer.h"

class QtNetworkPeerPrivate : public QObject
{
Q_OBJECT
public:
    QtNetworkPeerPrivate(QtNetworkPeer *qtDistributed);
    ~QtNetworkPeerPrivate();

    void setBroadcastEnabled(bool enable);
    void disconnectAllPeers();

    void sendMessage(const QByteArray &message);

    void broadcast();
    QList<QHostAddress> nonlocalAddresses();

private Q_SLOTS:
    void update();
    void processBroadcastDatagrams();

    void inboundConnectionAvailable();
    void inboundConnectionDisconnected();

    void outboundConnectionAvailable();
    void outboundConnectionDisconnected();

   void processMessage();

private:
    QtNetworkPeer *q;

    int m_port;
    QByteArray m_helloString;
    QByteArray m_portString;
    QByteArray m_addressString;

    QTimer m_timer;
    QUdpSocket m_udpSenderSocket;
    QUdpSocket m_udpReceiverSocket;

    QTcpServer m_tcpServer;

    QHash<QHostAddress, QList<QHostAddress> > m_knownPeers;

    QHash<QHostAddress, QTcpSocket *> m_acceptedInboundConnections;
    QHash<QHostAddress, QTcpSocket *> m_acceptedOutboundConnections;
};

#endif

