#ifndef QCONTROLLER_H
#define QCONTROLLER_H

#include <QObject>


class QController : public QObject
{
    Q_OBJECT

public:
    QController();
    ~QController();

    bool add_task(QObject* newtask);

public slots:
    void stop_threads();

signals:
    void finished();

protected:
    QObject *task;
};

#endif // CONTROLLER_H
