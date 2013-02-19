#include "receiver.h"

#include <QtCore>
#include <QtWidgets>

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    Receiver recevier;
    recevier.show();

    return app.exec();
}

