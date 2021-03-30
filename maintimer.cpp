#include "maintimer.h"

MainTimer::MainTimer(QObject *parent) :
    QTimer(parent)
{
    this->start();
}

void MainTimer::timerEvent ( QTimerEvent * e )
{
}
