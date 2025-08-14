#include "QSmartPrintSinApp.h"
#include <QPrinterInfo>
#include <QDesktopWidget>
#include <QProcess>
#include <QDir>
#include <QJsonObject>
#include <QTimer>
#include <QSettings>
#include <QMessageBox>
#include <QDebug>
#include <QXmlStreamReader>
#include <QSysInfo>

#include "QSmartPrintSin.h"
#include "QSelectPrinter.h"
#include "global.h"

#include "../include/unixcatcher.hpp"


//sudo apt-get install  libopenjp2-7-dev

QString PrinterName;
QString PRINTDirectory;
QString PDFDirectory;
bool LeftAlign = true;
QMap<int, QString> Errors;
QString Version;
QString SystemInfo;



void ReadError(QXmlStreamReader& xml, QMap<int, QString>& errors)
{
    int code;
    QString message;

    if (!xml.isStartElement() && xml.name() != "ERROR")
      xml.raiseError(QObject::tr("No es un documento XML valido"));
    else{

        while (!xml.atEnd() && !xml.hasError())
        {
            xml.readNext();

            if (xml.isEndElement() &&  xml.name() == "ERROR"){

                break;
            }
            else{

            if (xml.isStartElement() && xml.name() == "Code"){

              QString strcode = xml.readElementText();

              code = strcode.toInt();
             }

            if (xml.isStartElement() && xml.name() == "Message"){

              message = xml.readElementText();

             }
         }
       }
    }

   if (!message.isEmpty())
     errors[code] = message;
}


void ReadErrors(QXmlStreamReader& xml, QMap<int, QString>& errors)
{
   if (!xml.isStartElement() && xml.name() != "ERRORS")
     xml.raiseError(QObject::tr("No es un documento XML valido"));
   else {

       while (!xml.atEnd() && !xml.hasError())
       {
         xml.readNext();

         if (xml.isEndElement() &&  xml.name() == "ERRORS"){

           break;
          }
         else{

             if (xml.isStartElement() && xml.name() == "ERROR"){

              ReadError(xml, errors);

             }
          }
       }
     }
}


int LoadErrors(const QString& fileName, QMap<int, QString>& errors)
{
  QString errorsFile = QCoreApplication::applicationDirPath() + QDir::separator() + fileName;

  QFile file(errorsFile);

  if(!file.open(QFile::ReadOnly | QFile::Text)){
          qDebug() << "Cannot read file" << file.errorString();
          return -1;
      }


  QXmlStreamReader xml(&file);

  xml.readNextStartElement();

  ReadErrors(xml, errors);

  return errors.size();
}



bool LoadSettings(QString& localhost)
{
    QString file = QCoreApplication::applicationDirPath() + QDir::separator() + "SmartPrintSin.ini";
    QSettings settings(file, QSettings::IniFormat);

    if (!settings.value("LOCALHOST").isValid())
      return false;

    localhost = settings.value("LOCALHOST").toString();

    return !localhost.isEmpty();
}


#ifdef Q_OS_WIN
QString getWMIC(const QString &args)
{
    QProcess p;

    QString winPath = QString::fromUtf8(qgetenv("windir"));
    QString cmd;

    if (QDir(winPath + "\\SysWOW64\\wbem\\").exists())
      cmd = winPath + "\\SysWOW64\\wbem\\wmic " + args;
    else
      cmd = winPath + "\\system32\\wbem\\wmic " + args;

    p.start(cmd);
    p.waitForFinished();

    QString result = QString::fromLocal8Bit(p.readAllStandardOutput());
    QStringList list = cmd.split(" ");

    result = result.remove(list.last(), Qt::CaseInsensitive);
    result = result.replace("\r", "");
    result = result.replace("\n", "");
    result = result.simplified();

    return result;
}
#else
QString getLMIC(const QString &args)
{
   QProcess linuxcpuinfo;

   linuxcpuinfo.start("bash", QStringList() << "-c" << args);

   linuxcpuinfo.waitForFinished();

   QString linuxOutput = linuxcpuinfo.readAllStandardOutput();

   return linuxOutput;
}
#endif

QString GetPhysicalMemory()
{
    double memory = 0;

#ifdef Q_OS_WIN
    memory = getWMIC("computersystem get TotalPhysicalMemory").toDouble();

    memory /= 1024.0*1024.0*1024.0;
#else

   QString strmem = getLMIC("cat  /proc/meminfo | grep 'MemTotal' | uniq");

   strmem.remove("MemTotal", Qt::CaseInsensitive);

   strmem.remove(":", Qt::CaseInsensitive);

   strmem.remove("kb", Qt::CaseInsensitive);

   strmem.remove('\n');

   strmem = strmem.trimmed();

   memory =  strmem.toDouble();

   memory /= 1024.0*1024.0;

#endif

   return QString("%1 GB").arg(memory, 0 ,'g', 2);
}

QString GetCPU()
{
  QString cpu;

#ifdef Q_OS_WIN
  cpu = getWMIC("cpu get name");
#else
  cpu = getLMIC("cat /proc/cpuinfo | grep 'model name' | uniq");

  int pos = cpu.indexOf('\t');

  if (pos != -1){

      cpu = cpu.mid(pos + 2);

      cpu = cpu.trimmed();

      cpu.remove('\n');
  }

#endif

  return cpu;
}

QString GetSystemInfo()
{
 QString os, resolution, cpu, memory;

 os = QSysInfo::prettyProductName() + QString(" %1 bits").arg(QSysInfo::WordSize);

 QRect rect = QApplication::desktop()->screenGeometry();

 resolution = QString("%1x%2").arg(rect.width()).arg(rect.height());

 cpu = GetCPU();

 memory = GetPhysicalMemory();

 return QString("%1, %2, %3, %4").arg(os).arg(resolution).arg(cpu).arg(memory);
}

QJsonDocument Reply(int response, const QString &description, const QString &pdf)
{
    QJsonObject json_obj, addint_json, system_json;
    QStringList systemfields = SystemInfo.split(",");

    if (systemfields.size() >= 4)
     {
      system_json["OS"] = systemfields[0].trimmed();
      system_json["resolucion"] = systemfields[1].trimmed();
      system_json["cpu"] = systemfields[2].trimmed();
      system_json["mem"] = systemfields[3].trimmed();
     }

    addint_json["version"] = Version;

    json_obj["codigoRespuesta"] = response;
    json_obj["descripcion"] = description;

    if (!pdf.isEmpty())
      json_obj["pdf"] = pdf;

    json_obj["adicionalesInt"] = QJsonDocument(addint_json).object();
    json_obj["adicionalesOS"] = QJsonDocument(system_json).object();

    QJsonDocument json = QJsonDocument(json_obj);

    return json;
}


bool IsPrinter(const QString &name)
{
    if (!name.isEmpty()){

      QStringList printers = QPrinterInfo::availablePrinterNames();

      for ( const auto& printer : printers  ) {

        if (!printer.compare(name))
          return true;
      }

   }

    return false;
}


bool IsDir(const QString &name)
{
    if (!name.isEmpty())
      return QDir(name).exists();

    return false;
}

bool IsSettings()
{
    QString file = QCoreApplication::applicationDirPath() + QDir::separator() + "SmartPrintSin.ini";
    QSettings settings(file, QSettings::IniFormat);

    PrinterName = settings.value("PRINTER").toString();

    PRINTDirectory = settings.value("PRINTDIR").toString();

    PDFDirectory = settings.value("PDFDIR").toString();

    LeftAlign = settings.value("LEFT", true).toBool();

    Version = settings.value("VERSION", "").toString();

    return IsPrinter(PrinterName) && IsDir(PRINTDirectory) && IsDir(PDFDirectory);
}

int main(int argc, char *argv[])
{
    QSmartPrintSinApp app(argc, argv);

#if defined(Q_OS_UNIX)
    catchUnixSignals({SIGQUIT, SIGINT, SIGTERM, SIGHUP});
#endif

    if( app.isSecondary() ) {
            app.sendMessage("secondary instance");
            return 0;
        } else {
            QObject::connect(
                &app,
                &QSmartPrintSinApp::receivedMessage,
                &app,
                &QSmartPrintSinApp::Message
            );
        }

    QString LocalHost;

    if (!LoadSettings(LocalHost)){

      qCritical().noquote().nospace() << "Error, cargando SmartPrintSin.ini ...\n";
      qCritical().noquote().nospace() << "Abortar programa\n";

      QMessageBox::critical(nullptr,"SmartPrintSin", "Error, cargando SmartPrintSin.ini");

      exit(EXIT_FAILURE);

    }

    if (LoadErrors("errors.xml", Errors) == -1){

        qCritical() << "Errors file not found";

        QMessageBox::critical(nullptr,"SmartPrintSin", "NO EXISTE ARCHIVO 'erros.xml' DE DEFINICION DE ERRORES");

        exit(EXIT_FAILURE);
    }

    if (!Errors.size()){

        qCritical() << "Errors file empty";

        QMessageBox::critical(nullptr,"SmartPrintSin", "ARCHIVO 'errors.xml' DE DEFINICION DE ERRORES NO VALIDO");

        exit(EXIT_FAILURE);
    }

    SystemInfo = GetSystemInfo();

    app.setApplicationName("SmartPrintSin");
    app.setApplicationVersion("1.3.0");

    QApplication::setQuitOnLastWindowClosed(false);

    QSmartPrintSin SmartPrintSin(LocalHost);

    if (!IsSettings())
      QTimer::singleShot(100, &app, &QSmartPrintSinApp::selectPrinter);
    else
      QTimer::singleShot(100, &app, &QSmartPrintSinApp::startPrint);

    QObject::connect(&app, &QSmartPrintSinApp::aboutToQuit, &app, &QSmartPrintSinApp::stopPrint);

    return app.exec();
}
