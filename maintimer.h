#ifndef MAINTIMER_H
#define MAINTIMER_H

#include <QTimer>

class MainTimer : public QTimer
{
    Q_OBJECT
public:
    explicit MainTimer(QObject *parent = 0);
    void timerEvent ( QTimerEvent * e );

signals:

public slots:

};

#endif // MAINTIMER_H
