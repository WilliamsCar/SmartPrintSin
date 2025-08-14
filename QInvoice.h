#ifndef QINVOICE_H
#define QINVOICE_H

#include "QPrint.h"

class QInvoice : public QPrint
{
Q_OBJECT

public:
  QInvoice(const QString& printerName, const QString& pdfdir, const QJsonDocument& json, bool leftalign, int from = 0, int to = 0, QPrinter::PrinterMode mode = QPrinter::ScreenResolution, QPrinter::Orientation orientation = QPrinter::Portrait);


protected:
  void removelastlines(QJsonArray &lines);
  void exec();

};

#endif // QINVOICE_H
