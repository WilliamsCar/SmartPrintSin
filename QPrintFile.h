#ifndef QPRINTFILE_H
#define QPRINTFILE_H

#include "QPrint.h"

QT_BEGIN_NAMESPACE
class QPrintDirectory;
QT_END_NAMESPACE

class QPrintFile : public QPrint
{
    Q_OBJECT
public:
  QPrintFile(const QString& printerName, const QString& pdfdir, const QJsonDocument& json, bool leftalign, int from = 0, int to = 0,  QPrinter::PrinterMode mode = QPrinter::ScreenResolution, QPrinter::Orientation orientation = QPrinter::Portrait);

protected:
  void exec();

};

#endif // QPRINTFILE_H
