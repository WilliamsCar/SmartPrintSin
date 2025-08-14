#include "QPrintDirectory.h"
#include <QThread>
#include <QUuid>
#include <QTemporaryFile>
#include <QDir>
#include <QJsonObject>
#include <QDebug>

#include "global.h"

QPrintDirectory::QPrintDirectory(QObject *parent) : QObject(parent)
{
}

QPrintDirectory::~QPrintDirectory()
{
    qDebug() << "~QPrintDirectory";
}

void QPrintDirectory::setPrinter(const QString &name)
{
    lock.lockForWrite();

    printername = name;

    lock.unlock();
}

void QPrintDirectory::setPrintDir(const QString &name)
{
    lock.lockForWrite();

    printdir = name;

    lock.unlock();
}

void QPrintDirectory::setPDFDir(const QString &name)
{
    lock.lockForWrite();

    pdfdir = name;

    lock.unlock();
}

void QPrintDirectory::cancel()
{
    lock.lockForWrite();

    bCancel = true;

    lock.unlock();
}


QPrinter::PrinterMode QPrintDirectory::GetMode(const QString &fileName)
{
    QgsDelimitedTextFile file;

    file.setFileName(fileName);

    if (!file.open())
      return QPrinter::ScreenResolution;

    file.setEncoding(QStringList() << "UTF-8" << "ISO 8859-1");

    file.setUseHeader(false);

    QString line;

     while (file.nextLine(line) == QgsDelimitedTextFile::RecordOk){

         if (line.contains("<SinteBase64>", Qt::CaseInsensitive))
             return  QPrinter::HighResolution;

     }

    return QPrinter::ScreenResolution;
}

bool isFileWritable(const QString& fileName)
{
   QFile file(fileName);
   bool isWritable = file.open(QFile::ReadWrite);

   file.close();

   return isWritable;
}

bool isFileRenamed(const QFileInfo& fileInfo, QString& newName)
{
    QTemporaryFile file(QDir::tempPath() + QDir::separator() + "SPSXXXXXX.dat");

    if (!file.open())
      return false;

    newName = file.fileName();

    file.remove();

    return QFile::rename(fileInfo.absoluteFilePath(), newName);
}

void QPrintDirectory::run()
{
   QString printer;
   QString print;
   QString pdf;
   bool Continue;

   do{

     lock.lockForRead();

     printer = printername;

     print = printdir;

     pdf = pdfdir;

     Continue = !bCancel;

     lock.unlock();

     if (!Continue)
       break;
     else {

         if (printer.isEmpty() || print.isEmpty() || pdf.isEmpty())
           QThread::sleep(1);
         else {

           QDir dir(print);

           if (!dir.exists())
             QThread::sleep(1);
           else{

               QFileInfoList files = dir.entryInfoList(QDir::Files, QDir::Time);

               if (files.isEmpty())
                 QThread::sleep(1);
               else{

                 QString newName;

                 foreach(QFileInfo file, files){

                   if (!isFileRenamed(file, newName))
                     QThread::sleep(1);
                   else {

                     QJsonObject object;

                     object["output"] = "printer";
                     object["file"] = newName;

                     QPrintFile PrintFile(printer, pdf, QJsonDocument(object), LeftAlign, 0, 0, GetMode(newName));
                   }

                 }
              }
           }
        }
     }

   }while (true);

   qDebug() << "QPrintDirectory";
}


