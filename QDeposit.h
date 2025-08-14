#ifndef QDEPOSIT_H
#define QDEPOSIT_H

#include <QJsonObject>

#include "QPrint.h"

class QDeposit : public QPrint
{
  public:
    QDeposit(const QString& printerName, const QString& pdfdir, const QJsonDocument& json, bool leftalign, int from = 0, int to = 0, QPrinter::PrinterMode mode = QPrinter::ScreenResolution, QPrinter::Orientation orientation = QPrinter::Portrait);

  protected:
     QImage toImage(const QJsonObject& object, const QRect& rect, int &left);
     int paintLines(QPainter& painter, const QJsonArray& lines, const QRect& rect, const QRect& fullRect, int resolution);

  protected:
    void exec();
};

#endif // QDEPOSIT_H
