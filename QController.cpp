#include "QController.h"

#include <QDebug>
#include <QThread>
#include <QTimer>
#include <QVector>

QController::QController()
{
}

QController::~QController()
{
    qDebug() << "Controller object was destroyed\n";
}

bool QController::add_task(QObject *newtask)
{
    newtask->setParent(nullptr);

    QThread* thread = new QThread();

    newtask->moveToThread(thread);

    connect(thread, SIGNAL(started()), newtask, SLOT(run()));
    connect(newtask, SIGNAL(finished()), thread, SLOT(quit()));
    connect(newtask, SIGNAL(finished()), newtask, SLOT(deleteLater()));
    connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
    connect(thread, SIGNAL(destroyed()), this, SLOT(stop_threads()));

    thread->start();

    return true;
}

void QController::stop_threads()
{ 
   emit finished();

   qDebug() << "Controller stop threads\n";
}



