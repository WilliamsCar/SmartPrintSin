#include <QByteArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QApplication>
#include <QMap>
#include <QString>
#include <QMessageBox>
#include <QTimer>
#include <QFile>

#include "qhttpserver.hpp"
#include "qhttpserverresponse.hpp"
#include "qhttpserverrequest.hpp"

#include "QSmartPrintSin.h"

#include "global.h"

#include "QInvoice.h"
#include "QDeposit.h"
#include "QFileInvoice.h"
#include "QStatus.h"


static QPrintRange *PrintRange = nullptr;

QSmartPrintSin::QSmartPrintSin(QString url) : QObject(nullptr),
    URL(url)
{
    QHttpServer *server = new QHttpServer(this);

    connect(server, SIGNAL(newRequest(QHttpRequest*, QHttpResponse*)),
                  this, SLOT(handleRequest(QHttpRequest*, QHttpResponse*)));

    QUrl Url(url);

    if (server->listen(QHostAddress(Url.host()), Url.port()))
      qInfo().noquote().nospace() << "Servicio iniciado, esperando peticiones ...\n";
    else{

     qCritical().noquote().nospace() << "No fue posible iniciar el servicio en la direccion " << Url.host() <<  " y puerto " << Url.port();

     qCritical().noquote().nospace() << "Abortar programa\n";

     QString text = QString("Error, No fue posible iniciar el servicio\ndireccion : %1\npuerto : %2").arg(Url.host().toLatin1().constData()).arg(Url.port());

     QMessageBox::critical(nullptr,"SmartBioSin", text);

     exit(EXIT_FAILURE);

    }
}


QSmartPrintSin::~QSmartPrintSin()
{
}

void QSmartPrintSin::handleRequest(QHttpRequest *req, QHttpResponse *resp)
{
    if (req->method() == qhttp::EHTTP_OPTIONS)
      {
        resp->addHeader("content-type", "application/json; charset=utf-8");

        resp->addHeader("Accept", "application/json");

        resp->addHeader("Access-Control-Allow-Origin", "*");

        resp->addHeader("Access-Control-Allow-Methods", "GET,POST,PUT,PATCH,DELETE,HEAD,OPTIONS");

        resp->addHeader("Access-Control-Allow-Headers", "Content-Type, Origin, Accept, Authorization, Content-Length, X-Requested-With");

        resp->setStatusCode(qhttp::ESTATUS_OK);

        resp->end();

        return;
      }

    if (!m_ListResponses.isEmpty()){

          if (m_ListResponses.first()->Finished())
            delete m_ListResponses.takeFirst();
       }

    Responder *responder = new Responder(req, resp);

    m_ListResponses.append(responder);
}

Responder::Responder(QHttpRequest *req, QHttpResponse *resp)
    : m_req(req),
      m_resp(resp)
{
    connect(req, SIGNAL(data(const QByteArray&)), this, SLOT(accumulate(const QByteArray&)));

    connect(req, SIGNAL(end()), this, SLOT(ended()));

    connect(resp, SIGNAL(destroyed()), this, SLOT(deleteLater()));
}

Responder::~Responder()
{
}


bool Responder::ValidateJson(const QJsonDocument &json, const QStringList &keys)
{
  QJsonObject obj = json.object();
  QStringList objKeys = obj.keys();

  if (keys.size() != objKeys.size())
     return  false;

  for (int i = 0; i < keys.size(); i++)
     if (!objKeys.contains(keys[i]))
       return false;

 return true;
}

Responder::RestType Responder::JsonType(const QJsonDocument &json)
{
  QStringList keys;
  QJsonObject obj = json.object();
  QStringList objKeys = obj.keys();

  keys << "status";

  if (ValidateJson(json, keys))
    return Status;

  keys.clear();

  if (json["output"].toString().compare("printer", Qt::CaseInsensitive) != 0 &&
          json["output"].toString().compare("pdf", Qt::CaseInsensitive) != 0 &&
          json["output"].toString().compare("pdfjson", Qt::CaseInsensitive) != 0)
      return Unknow;

  keys << "output" << "lines";

  if (ValidateJson(json, keys))
     return Invoice;

  keys.clear();

  keys << "output" << "images";

  if (ValidateJson(json, keys))
     return Deposit;

  keys.clear();

  keys << "output" << "file";

  if (ValidateJson(json, keys))
     return FileInvoice;

  keys.clear();



  return Unknow;
}


int Responder::InvoiceNumPages(const QJsonDocument &json)
{
   int Pages = 1;

   QJsonArray lines = json["lines"].toArray();
   QString text;
   int size = lines.size();

   if (lines[size - 1].toString().contains("@PB", Qt::CaseInsensitive)
           || lines[size - 1].toString().contains(0x0C, Qt::CaseInsensitive))
       size--;

   for (int i = 0;  i < size; i++){

     text = lines[i].toString();

     if (text.contains("@PB", Qt::CaseInsensitive))
       Pages++;

     if (text.contains(0x0C, Qt::CaseInsensitive))
       Pages++;

     if (text.contains("<SinteBase64>", Qt::CaseInsensitive)){

       Mode = QPrinter::HighResolution;

       return 1;

      }

   }

   text = text.trimmed();

   if (Pages > 1 && text.isEmpty() && ((lines[size-2].toString().contains("@PB", Qt::CaseInsensitive))
                || (lines[size-2].toString().contains(0x0C, Qt::CaseInsensitive))))
        Pages--;

   Mode = QPrinter::ScreenResolution;

   return Pages;
}

int Responder::DepositNumPages(const QJsonDocument &json)
{
    int Pages = json["images"].toArray().size();

    Mode = QPrinter::ScreenResolution;

    return Pages;
}

int Responder::FileInvoiceNumPages(const QJsonDocument &json)
{
    int Pages = 1;

    QgsDelimitedTextFile file;

    file.setFileName(json["file"].toString());

    if (!file.open())
      return 0;

    file.setEncoding(QStringList() << "UTF-8" << "ISO 8859-1");

    file.setUseHeader(false);

    QString line, prev;

     while (file.nextLine(line) == QgsDelimitedTextFile::RecordOk){

         if (line.contains("@PB", Qt::CaseInsensitive))
           Pages++;

         if (line.contains(0x0C, Qt::CaseInsensitive))
           Pages++;

         if (line.contains("<SinteBase64>", Qt::CaseInsensitive)){

            Mode = QPrinter::HighResolution;

            return 1;

          }

         prev = line;

         line.clear();

     }

     if (Pages > 1 && line.isEmpty() && ((prev.contains("@PB", Qt::CaseInsensitive))
                  || (prev.contains(0x0C, Qt::CaseInsensitive))))
          Pages--;

    Mode = QPrinter::ScreenResolution;

    return Pages;
}

int Responder::NumPages(const QJsonDocument &json)
{
   int Pages = 0;

   switch (Type) {

     case Invoice:
       Pages = InvoiceNumPages(json);
     break;
     case Deposit:
       Pages = DepositNumPages(json);
     break;
     case FileInvoice:
       Pages = FileInvoiceNumPages(json);
     break;
     default:
       Pages = 0;
     break;
   }

   return Pages;
}

void Responder::accumulate(const QByteArray &data)
{
    Data.append(data);
}


void Responder::ended()
{
    Json = QJsonDocument::fromJson(Data);

    if (Json.isEmpty() ) {

        QJsonDocument jsonReply = Reply(JSON_NOT_VALID, Errors[JSON_NOT_VALID]);

        reply(jsonReply);

        return;
    }
    else
    {

      Type = JsonType(Json);

      switch(Type){

        case Invoice:
           QTimer::singleShot(50, this, &Responder::ToPrint);
        break;
        case Deposit:
           QTimer::singleShot(50, this, &Responder::ToPrint);
        break;
        case FileInvoice:
          QTimer::singleShot(50, this, &Responder::ToPrint);
        break;
        case Status:
          QTimer::singleShot(50, this, &Responder::ToStatus);
        break;
        default:
          QJsonDocument jsonReply = Reply(JSON_NOT_VALID, Errors[JSON_NOT_VALID]);
          reply(jsonReply);
        break;
      }
   }
}

void Responder::reply(const QJsonDocument &json)
{
   if (m_finished)
      return;

   m_resp->addHeader("content-type", "application/json; charset=utf-8");

   m_resp->addHeader("Accept", "application/json");

   m_resp->addHeaderValue("content-length",json.toJson().length());

   m_resp->addHeader("Access-Control-Allow-Origin", "*");

   m_resp->addHeader("Access-Control-Allow-Methods", "GET,POST,PUT,PATCH,DELETE,HEAD,OPTIONS");

   m_resp->addHeader("Access-Control-Allow-Headers", "Content-Type, Origin, Accept, Authorization, Content-Length, X-Requested-With");

   m_resp->setStatusCode(qhttp::ESTATUS_OK);

   m_resp->end(json.toJson());
}

void Responder::deleteLater()
{
   m_finished = true;

   emit disconnected();
}

void Responder::ToPrint()
{
    int Pages = NumPages(Json);
    int From = 0, To = 0;
    QPrinter::Orientation Orientation = QPrinter::Portrait;

    if (!Pages){

      QJsonDocument jsonReply = Reply(FILE_READ_ERROR, Errors[FILE_READ_ERROR]);

      reply(jsonReply);

      return;

    }

    if (Pages > 1){

        if (PrintRange){

            QJsonDocument jsonReply = Reply(SERVICE_BUSY, Errors[SERVICE_BUSY]);

            reply(jsonReply);

            return;
        }

        PrintRange = new QPrintRange(Pages, Type == Deposit);

        connect(this, &Responder::disconnected, PrintRange, &QPrintRange::cancel);

        if (PrintRange->exec() == QDialog::Rejected){

            QJsonDocument jsonReply = Reply(OPERATION_CANCEL_BYUSER, Errors[OPERATION_CANCEL_BYUSER]);

            reply(jsonReply);

            delete PrintRange;

            PrintRange = nullptr;

            return;
        }

        From = PrintRange->From();
        To = PrintRange->To();

        Orientation = PrintRange->Orientation();

        delete PrintRange;

        PrintRange = nullptr;

    }


    QPrint *print;

    if (Type == Invoice)
      print = new QInvoice(PrinterName, PDFDirectory, Json, LeftAlign, From, To, Mode, Orientation);
    else if (Type == Deposit)
      print = new QDeposit(PrinterName, PDFDirectory, Json, LeftAlign, From, To, Mode, Orientation);
    else
      print = new QFileInvoice(PrinterName, PDFDirectory, Json, LeftAlign, From, To, Mode, Orientation);

    connect(print, &QPrint::response, this, &Responder::reply);

    controller.add_task(print);
}



void Responder::ToStatus()
{

    QStatus *status = new QStatus(PrinterName);

    connect(status, &QStatus::response, this, &Responder::reply);

    controller.add_task(status);
}
