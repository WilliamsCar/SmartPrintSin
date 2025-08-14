#ifndef QSTATUS_H
#define QSTATUS_H

#include <QObject>

class QStatus : public QObject
{
    Q_OBJECT
public:
    QStatus(const QString& printerName);

protected slots:
    void run();

signals:
     void response(const QJsonDocument &json);
     void finished();


protected:
    QString Name;


};

#endif // QSTATUS_H
