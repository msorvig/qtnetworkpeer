#include "receiver.h"

Receiver::Receiver()
{
    connect(&networkpeer, SIGNAL(messageReady(QByteArray)), SLOT(newMessage(QByteArray)));

    label = new QLabel(this);
    label->setText("foo");
    label->setGeometry(10, 10, 400, 30);

    disconnectButton = new QPushButton(this);
    disconnectButton->setText("Disconnect");
    disconnectButton->setGeometry(10, 50, 200, 32);
    connect(disconnectButton, SIGNAL(clicked()), SLOT(disconnect()));
}

void Receiver::newMessage(const QByteArray &message)
{
    label->setText(message);
}

void Receiver::disconnect()
{
    networkpeer.disconnectAllPeers();
}
