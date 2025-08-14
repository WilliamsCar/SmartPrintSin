#ifndef QFILEINVOICE_H
#define QFILEINVOICE_H

#include "QPrint.h"

class QFileInvoice : public QPrint
{
public:
  QFileInvoice(const QString& printerName, const QString& pdfdir, const QJsonDocument& json, bool leftalign, int from = 0, int to = 0, QPrinter::PrinterMode mode = QPrinter::ScreenResolution, QPrinter::Orientation orientation = QPrinter::Portrait);

protected:
  void exec();

};

#endif // QFILEINVOICE_H
