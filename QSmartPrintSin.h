#ifndef QSMARTPRINTSIN_H
#define QSMARTPRINTSIN_H

#include <QObject>
#include <QJsonDocument>
#include <QTimer>
#include "qhttpfwd.hpp"
#include "QController.h"
#include "QPrintRange.h"

using namespace qhttp::server;

class Responder;


class QSmartPrintSin : public QObject
{
    Q_OBJECT
public:
    QSmartPrintSin(QString url);
    ~QSmartPrintSin();

private slots:
    void handleRequest(QHttpRequest *req, QHttpResponse *resp);

protected:
    QString URL;
    QList<Responder *> m_ListResponses;
    QController *controller;
    QTimer *timer;
};


class Responder : public QObject
{
    Q_OBJECT

public:

    enum RestType{
      Unknow = 0,
      Invoice = 1,
      Deposit = 2,
      FileInvoice = 3,
      Status = 4
    };

public:
    Responder(QHttpRequest *req, QHttpResponse *resp);
    ~Responder();

public:
    bool Finished(){return m_finished;}

protected:
    bool ValidateJson(const QJsonDocument &json, const QStringList &keys);
    RestType JsonType(const QJsonDocument &json);
    int InvoiceNumPages(const QJsonDocument &json);
    int DepositNumPages(const QJsonDocument &json);
    int FileInvoiceNumPages(const QJsonDocument &json);
    int NumPages(const QJsonDocument &json);

private slots:
    void accumulate(const QByteArray &data);
    void ended();
    void reply(const QJsonDocument &json);
    void deleteLater();
    void ToPrint();
    void ToStatus();

signals:
    void disconnected();

private:
    QHttpRequest *m_req;
    QHttpResponse *m_resp;
    QByteArray Data;
    QJsonDocument Json;
    bool m_finished = false;

    QController controller;

    RestType Type = Unknow;

    QPrinter::PrinterMode Mode;

private:
    Q_DISABLE_COPY(Responder)
};

#endif // QSMARTPRINTSIN_H
