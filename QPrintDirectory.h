#ifndef QPRINTDIRECTORY_H
#define QPRINTDIRECTORY_H

#include <QObject>
#include <QReadWriteLock>

#include "QPrintFile.h"

class QPrintDirectory : public QObject
{
    Q_OBJECT
public:
    explicit QPrintDirectory(QObject *parent = nullptr);
    ~QPrintDirectory();

public:
    void setPrinter(const QString &name);
    void setPrintDir(const QString &name);
    void setPDFDir(const QString &name);
    void cancel();

protected:
    QPrinter::PrinterMode GetMode(const QString &fileName);


protected slots:
    void run();

signals:
     void finished();

private:
    QReadWriteLock lock;

    QString printername;
    QString printdir;
    QString pdfdir;

    bool bCancel = false;

};

#endif // QPRINTDIRECTORY_H
