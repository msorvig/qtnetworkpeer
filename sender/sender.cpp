#include "sender.h"

Sender::Sender()
{
    timer.start(2000);
    connect(&timer, SIGNAL(timeout()), SLOT(sendMessage()));

    disconnectButton = new QPushButton(this);
    disconnectButton->setText("Disconnect");
    disconnectButton->setGeometry(10, 50, 200, 32);
    connect(disconnectButton, SIGNAL(clicked()), SLOT(disconnect()));
}

void Sender::sendMessage()
{
    static int count = 0;

    networkpeer.sendMessage(QByteArray("Hello from sender ") + QByteArray::number(count));
    ++count;
}

void Sender::disconnect()
{
    networkpeer.disconnectAllPeers();
}
