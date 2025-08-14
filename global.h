#ifndef GLOBAL_H
#define GLOBAL_H

#include <QMap>
#include <QJsonDocument>

#define JSON_NOT_VALID              -1
#define OPERATION_SUCCESSFULLY    0x00
#define OPERATION_CANCEL_BYUSER   0x01
#define PRINTER_ERROR             0x02
#define PDF_DIR_ERROR             0x03
#define SERVICE_BUSY              0x04
#define FILE_READ_ERROR           0x05

extern QString PrinterName;
extern QString PRINTDirectory;
extern QString PDFDirectory;
extern bool LeftAlign;
extern QMap<int, QString> Errors;
extern QString Version;
extern QString SystemInfo;

extern QJsonDocument Reply(int response, const QString &description, const QString &pdf = "");

#endif // GLOBAL_H
