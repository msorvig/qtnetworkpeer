#include "qtnetworkpeer_p.h"
#include <QtCore>


QtNetworkPeerPrivate::QtNetworkPeerPrivate(QtNetworkPeer *qtDistributed)
    :q(qtDistributed)
    ,m_port(45456)
    ,m_helloString("Connect QtDistributed ")
    ,m_portString("Port ")
    ,m_addressString(" Address ")
{
    // Start Tcp server for inbound connections
    connect(&m_tcpServer, SIGNAL(newConnection()), SLOT(inboundConnectionAvailable()));
    bool ok = m_tcpServer.listen();
    if (!ok)
        qDebug() << "tcp server fail";

    // Start broadcast sender
    connect(&m_timer, SIGNAL(timeout()), this, SLOT(update()));
    setBroadcastEnabled(true);

    // set up broadcast receiver
    m_udpReceiverSocket.bind(m_port, QUdpSocket::ShareAddress);
    connect(&m_udpReceiverSocket, SIGNAL(readyRead()), this, SLOT(processBroadcastDatagrams()));
}

QtNetworkPeerPrivate::~QtNetworkPeerPrivate()
{
    m_timer.stop();
}

void QtNetworkPeerPrivate::setBroadcastEnabled(bool enable)
{
    if (enable && !m_timer.isActive()) {
        m_timer.start(1000);
        broadcast();
    } else if (m_timer.isActive()) {
        m_timer.stop();
    }
}

void QtNetworkPeerPrivate::disconnectAllPeers()
{
    foreach(QTcpSocket *socket, m_acceptedOutboundConnections.values())
        socket->close();
    m_acceptedInboundConnections.clear();

    foreach(QTcpSocket *socket, m_acceptedInboundConnections.values())
        socket->close();
    m_acceptedOutboundConnections.clear();
}

void QtNetworkPeerPrivate::sendMessage(const QByteArray &message)
{
    qDebug() << "sendMessage" << message;

    foreach (const QHostAddress &address, m_acceptedOutboundConnections.keys()) {
        qDebug() << "sendMessage" << address;
        QTcpSocket *socket = m_acceptedOutboundConnections.value(address);
        qDebug() << "socket" << socket->isValid() << socket->isOpen();
        socket->write(message);
        socket->flush();
    }
}

void QtNetworkPeerPrivate::broadcast()
{
    qDebug() << "udp broadcast";

    foreach (const QHostAddress &address, m_acceptedInboundConnections.keys()) {
        qDebug() << "connected to" << address;
    }

    QByteArray datagram = m_helloString;
    datagram += m_portString;
    datagram += QString::number(m_tcpServer.serverPort()).toLatin1();
    datagram += m_addressString;

    foreach (QHostAddress address, nonlocalAddresses()) {
        datagram += address.toString().toLatin1();
        datagram += " ";
    }

    m_udpSenderSocket.writeDatagram(datagram.data(), datagram.size(), QHostAddress::Broadcast, m_port);
}

QList<QHostAddress> QtNetworkPeerPrivate::nonlocalAddresses()
{
    QList<QHostAddress> localAddresses;
    foreach (QHostAddress address, QNetworkInterface::allAddresses()) {
        if (address == QHostAddress::LocalHost || address == QHostAddress::LocalHostIPv6)
            continue;
        if (address.toString().contains("fe80::1%lo0")) // ### whats this?
            continue;
        if (address.protocol() == QAbstractSocket::IPv6Protocol) // ### Skip ipv6 for now
            continue;

        localAddresses.append(address);
    }

    return localAddresses;
}

// Process incomming udp broadcast datagrams. These are "request to connect"
// messages. This function parses the messages and possibly initiates a TCP
// connection. This connection will be used for outbound messages.
void QtNetworkPeerPrivate::processBroadcastDatagrams()
{
    while (m_udpReceiverSocket.hasPendingDatagrams()) {
        QByteArray datagram;
        datagram.resize(m_udpReceiverSocket.pendingDatagramSize());
        m_udpReceiverSocket.readDatagram(datagram.data(), datagram.size());

        // Check for unknown message type.
        if (!datagram.startsWith(m_helloString))
            return;
        datagram = datagram.mid(m_helloString.length()); // remove m_helloString

        // Get port ("port 12345")
        if (!datagram.startsWith(m_portString))
            return;
        datagram = datagram.mid(m_portString.length());
        QByteArray portText = datagram.left(datagram.indexOf(' '));
        quint16 port = portText.toInt();
        datagram = datagram.mid(portText.length());

        // Get Addresses ("Address a.b.c.d e.f.g.h")
        if (!datagram.startsWith(m_addressString))
            return;
        datagram = datagram.mid(m_addressString.length()); // remove m_AddressStrings

        // Skip connections to self
        foreach (QHostAddress address, nonlocalAddresses()) {
            if (datagram.contains(address.toString().toLatin1()))
                return;
        }

        // Parse adresses
        QList<QHostAddress> hostAdresses;
        const QList<QByteArray> parts = datagram.split(' ');
        foreach (const QByteArray &part, parts) {
            QByteArray address = part.simplified();
            if (address.isEmpty())
                continue;

            QHostAddress hostAddress(QString::fromLatin1(address));
            hostAdresses.append(hostAddress);
        }

        // Maintain known peers. Each peer can have multiple adresses.
        foreach (const QHostAddress &address, hostAdresses) {
            if (m_knownPeers.contains(address))
                continue;
            qDebug() << "adding known peer" << address;
            m_knownPeers[address] = hostAdresses;
        }

        // Check if we are already connected to one of the adresses
        foreach (const QHostAddress &address, hostAdresses) {
            if (m_acceptedOutboundConnections.contains(address))
                return;
        }

        // Connect to each address (duplicate connections will
        // be dropped later on).
        foreach (const QHostAddress &address, hostAdresses) {
            qDebug() << "connect to" << address.toString() << portText;
            QTcpSocket *socket = new QTcpSocket(this);
            connect(socket, SIGNAL(connected()), SLOT(outboundConnectionAvailable()));
            socket->connectToHost(address, port);
        }
    }
}

// Handle a successful outbound connection.
void QtNetworkPeerPrivate::outboundConnectionAvailable()
{
    QTcpSocket *clientConnection = qobject_cast<QTcpSocket *>(sender());
    if (!clientConnection)
        return;
    connect(clientConnection, SIGNAL(disconnected()), clientConnection, SLOT(deleteLater()));

    qDebug() << "outbound connect to" << clientConnection->peerAddress().toString();
    // Don't connect to unknown peers. We need to know all addresses accociated with
    // the Peer to avoid duplicate connections.
    if (!m_knownPeers.contains(clientConnection->peerAddress())) {
        qDebug() << "unkonown host" << clientConnection->peerAddress().toString();
        clientConnection->close();
        return;
    }

    // Disconnect if we already have an outgoing connection to the peer.
    foreach (const QHostAddress &address, m_knownPeers.value(clientConnection->peerAddress())) {
        if (m_acceptedOutboundConnections.contains(address)) {
            qDebug() << "outbound: already connected";
            clientConnection->close();
            return;
        }
    }

    qDebug() << "accept outbound connection to" << clientConnection->peerAddress().toString();
    m_acceptedOutboundConnections[clientConnection->peerAddress()] = clientConnection;
    connect(clientConnection, SIGNAL(disconnected()), SLOT(outboundConnectionDisconnected()));
}

// Handle inbound disconnect
void QtNetworkPeerPrivate::outboundConnectionDisconnected()
{
    QTcpSocket *socket = qobject_cast<QTcpSocket *>(sender());
    if (!socket)
        return;

    qDebug() << "initiated disconnect" << socket->peerAddress();
    m_acceptedOutboundConnections.remove(socket->peerAddress());
}

//
void QtNetworkPeerPrivate::inboundConnectionAvailable()
{
    QTcpSocket *clientConnection = m_tcpServer.nextPendingConnection();
    connect(clientConnection, SIGNAL(disconnected()), clientConnection, SLOT(deleteLater()));

    qDebug() << "inbound connect from" << clientConnection->peerAddress().toString();
    if (!m_knownPeers.contains(clientConnection->peerAddress())) {
        qDebug() << "unkonown host" << clientConnection->peerAddress().toString();
        clientConnection->close();
        return;
    }
    // Disconnect if we already have an inbound connection to the peer
    foreach (const QHostAddress &address, m_knownPeers.value(clientConnection->peerAddress())) {
        if (m_acceptedInboundConnections.contains(address)) {
            qDebug() << "inbound: already connected";
            clientConnection->close();
            return;
        }
    }

    qDebug() << "accept inbound connection from" << clientConnection->peerAddress().toString();
    m_acceptedInboundConnections[clientConnection->peerAddress()] = clientConnection;
    connect(clientConnection, SIGNAL(disconnected()), SLOT(inboundConnectionDisconnected()));
    connect(clientConnection, SIGNAL(readyRead()), SLOT(processMessage()));
}

void QtNetworkPeerPrivate::inboundConnectionDisconnected()
{
    QTcpSocket *socket = qobject_cast<QTcpSocket *>(sender());
    if (!socket)
        return;

    qDebug() << "inbound disconnect" << socket->peerAddress();
    m_acceptedInboundConnections.remove(socket->peerAddress());
}

void QtNetworkPeerPrivate::processMessage()
{
    qDebug() << "processMessage";
    QTcpSocket *socket = qobject_cast<QTcpSocket *>(sender());
    if (!socket)
        return;

    QByteArray message = socket->readAll();
    qDebug() << "message" << message;

    emit q->messageReady(message);
}

void QtNetworkPeerPrivate::update()
{
    broadcast();
}
