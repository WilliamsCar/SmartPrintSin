#include "QPrint.h"
#include "QQREncode.h"

#include <QPainter>

#include <poppler-qt5.h>

#include <iostream>

#define LETTER_WIDTH_PIXELS   816.0
#define LETTER_HEIGHT_PIXELS  1056.0

QPrint::QPrint(const QString& printerName, const QString& pdfdir, const QJsonDocument& json, bool leftalign,int from, int to, QPrinter::PrinterMode mode,  QPrinter::Orientation orientation)
    : QObject(nullptr),  Name(printerName), pdf(pdfdir), JSon(json), leftAlign(leftalign), From(from), To(to), Mode(mode), Orientation(orientation)
{
    if (!JSon["output"].toString().compare("pdfjson", Qt::CaseInsensitive)){

      pdf = QDir::tempPath();

      isPDFMem = true;

    }

    QString file = QCoreApplication::applicationDirPath() + QDir::separator() + "SmartPrintSin.ini";
    QSettings settings(file, QSettings::IniFormat);

    isShowPDF = settings.value("ShowPDF", false).toBool();

    QDir temp(QDir::tempPath(), "SPS*.pdf");
    QFileInfoList infoList = temp.entryInfoList();

    for (auto &info : infoList)
      QFile::remove(info.absoluteFilePath());
}


QString QPrint::trimRight(const QString& str)
{
  int length = str.size() - 1;

  for (; length >= 0; --length) {
    if (!str.at(length).isSpace()) {
      return str.left(length + 1);
    }
  }
  return "";
}


bool QPrint::isEntel(QgsDelimitedTextFile &file)
{
    if (!file.isValid())
      return false;

    file.reset();

    QString text;
    TagTypes tag = Line;
    int Pages = MAX_PAGES;

    while (file.nextLine(text) == QgsDelimitedTextFile::RecordOk){

        text = text.trimmed();

        tag = GetTagType(text);

        if (tag == QREND)
          break;
        else if (tag == NewPage) {

            Pages--;

            if (!Pages)
              break;

        }
    }

    file.reset();

    return tag == QREND;
}

bool QPrint::isEntel(const QJsonArray& lines)
{
    QString text;
    TagTypes tag;

    for (const auto& element : lines){

      text = element.toString();

      text = text.trimmed();

      tag = GetTagType(text);

      if (tag == QREND)
        return true;;

    }

    return false;
}

int QPrint::GetMaxLineSize(QgsDelimitedTextFile &file, QPainter &painter, QPrinter &printer)
{
    if (!file.isValid())
      return 0;

    file.reset();

    QString text;
    TagTypes tag;
    int CurrentSize = 0;
    int Size = 0;
    int QRSize = 0;
    int Lines = 0;
    int Pages = MAX_PAGES;

    while (file.nextLine(text) == QgsDelimitedTextFile::RecordOk){

        text = text.trimmed();

        tag = GetTagType(text);

        if (tag == Line || tag == Bold){

          if (!Lines)
            CurrentSize = painter.fontMetrics().size(0, text).width();
          else {

            CurrentSize = painter.fontMetrics().size(0, text).width() + QRSize;

            Lines--;

          }

        }
        else if (tag == QR || tag == QRRD){
          QRSize = MM_TO_PIXEL(BARCODE_SIZE_MM, printer.resolution()) + MM_TO_PIXEL(QR_MARGIN_MM, printer.resolution()) + MM_TO_PIXEL(QR_MARGIN_MM, printer.resolution());
          Lines = MAX_QR_LINES;
         }
        else if (tag == NewPage) {

            Pages--;

            if (!Pages)
              break;

        }

        if (Size < CurrentSize)
          Size = CurrentSize;

    }

    file.reset();

    return Size;
}

int QPrint::GetMaxLineSize(const QJsonArray& lines, QPainter &painter, QPrinter &printer)
{
    QString text;
    TagTypes tag;
    int CurrentSize = 0;
    int Size = 0;
    int QRSize = 0;
    int Lines = 0;

    for (const auto& element : lines){

      text = element.toString();

      text = text.trimmed();

      tag = GetTagType(text);

      if (tag == Line || tag == Bold){

        if (!Lines)
          CurrentSize = painter.fontMetrics().size(0, text).width();
        else {

          CurrentSize = painter.fontMetrics().size(0, text).width() + QRSize;

          Lines--;

        }

      }
      else if (tag == QR || tag == QRRD){
        QRSize = MM_TO_PIXEL(BARCODE_SIZE_MM, printer.resolution()) + MM_TO_PIXEL(QR_MARGIN_MM, printer.resolution()) + MM_TO_PIXEL(QR_MARGIN_MM, printer.resolution());
        Lines = MAX_QR_LINES;
       }

      if (Size < CurrentSize)
        Size = CurrentSize;

    }

    return Size;
}

QJsonArray QPrint::LinesToPrint(const QJsonArray&  lines)
{
    if (!From && !To)
      return lines;

    QJsonArray printLines;

    QString text;
    int page = 1;

    for (int i = 0;  i < lines.size(); i++){

      if (page >= From)
        printLines.append(lines[i]);

      text = lines[i].toString();

      if (text.contains("@PB", Qt::CaseInsensitive))
        page++;

      if (text.contains(0x0C, Qt::CaseInsensitive))
        page++;

      if (page > To){

       printLines.removeLast();

       break;

      }
    }

    return printLines;
}

QPrint::TagTypes QPrint::GetTagType(QString& Text)
{
   if (Text.contains("<b>", Qt::CaseInsensitive))
     return Bold;

   if (Text.contains("<SinteBase64>", Qt::CaseInsensitive))
     return PDF;

   if (Text.contains("@PB", Qt::CaseInsensitive))
     return NewPage;

   if (Text.contains(0x0C, Qt::CaseInsensitive))
     return NewPage;

   if (Text.contains("<QR>", Qt::CaseInsensitive))
     return QR;

   if (Text.contains("<QRXY_", Qt::CaseInsensitive))
       return QRXY;

   if (Text.contains("<QRRD>", Qt::CaseInsensitive))
     return QRRD;

   if (Text.contains("<QRRDXY_"))
     return QRRDXY;

   if (Text.contains("<QR_ENT_G>", Qt::CaseInsensitive))
     return QREND;

   if (Text.contains("<QRLC_", Qt::CaseInsensitive))
     return QRLC;

   if (Text.contains("<QRRDLC_", Qt::CaseInsensitive))
     return QRRDLC;

   return Line;
}

QPrint::TagTypes QPrint::GetTagType(const QRect& rect,  QString& Text, int& x, int& y, bool &newPage)
{
   int pos;

   newPage = false;

   if ((pos = Text.indexOf("<b>", Qt::CaseInsensitive)) >= 0){

       Text = Text.remove("<b>");

       return Bold;
   }

   if ((pos = Text.indexOf("<SinteBase64>", Qt::CaseInsensitive)) >= 0){

       newPage = Text.mid(0, pos).contains("@PB");

       Text = Text.mid(pos + 13);

       Text = Text.trimmed();

       return PDF;
   }

   if (Text.contains("@PB", Qt::CaseInsensitive))
     return NewPage;

   if (Text.contains(0x0C, Qt::CaseInsensitive))
     return NewPage;

   if ((pos = Text.indexOf("<QR>", Qt::CaseInsensitive)) >= 0){

       Text = Text.mid(pos + 4);

       Text = Text.trimmed();

       return QR;
   }

   if ((pos = Text.indexOf("<QRXY_")) >= 0){

       if ((pos = Text.indexOf(">", pos)) != -1){

         QString qr = Text.mid(0, pos + 1);
         QStringList qrPos ;

         Text.remove(qr);

         Text = Text.trimmed();

         qr.remove("<QRXY_");

         qr.remove(">");

         qrPos = qr.split('_');

         if (qrPos.size() == 2){

           x = qrPos[0].toInt();
           y = qrPos[1].toInt();

           int dy = qRound(rect.height() / 4.0);

           if (y < dy)
             y = qMax(0, y - 10);
           else if (y >= dy && y < (dy * 2)) {
              y = qMax(0, y - 30);
           }else if (y >= (dy * 2) && y < (dy * 3)) {
               y = qMax(0, y - 40);
           }else {

               y = qMax(0, y - 60);
            }

           return QRXY;

         }

       }
   }

   if (!(pos = Text.indexOf("<QRRD>", Qt::CaseInsensitive))){

       Text = Text.mid(pos + 6);

       Text = Text.trimmed();

       return QRRD;
   }

   if ((pos = Text.indexOf("<QRRDXY_")) >= 0){

       if ((pos = Text.indexOf(">")) != -1){

         QString qr = Text.mid(0, pos + 1);
         QStringList qrPos ;

         Text.remove(qr);

         Text = Text.trimmed();

         qr.remove("<QRRDXY_");

         qr.remove(">");

         qrPos = qr.split('_');

         if (qrPos.size() == 2){

           x = qrPos[0].toInt();
           y = qrPos[1].toInt();

           int dy = qRound(rect.height() / 4.0);

           if (y < dy)
             y = qMax(0, y - 0);
           else if (y >= dy && y < (dy * 2)) {
              y = qMax(0, y - 20);
           }else if (y >= (dy * 2) && y < (dy * 3)) {
               y = qMax(0, y - 30);
           }else {

               y = qMax(0, y - 40);
            }

           return QRRDXY;
         }

       }
   }

   if ((pos = Text.indexOf("<QR_ENT_G>", Qt::CaseInsensitive)) >= 0){

       Text = Text.mid(pos + 10);

       Text = Text.trimmed();

       return QREND;
   }

   if ((pos = Text.indexOf("<QRLC_")) >= 0){

       if ((pos = Text.indexOf(">", pos)) != -1){

         QString qr = Text.mid(0, pos + 1);
         QStringList qrPos ;

         Text.remove(qr);

         Text = Text.trimmed();

         qr.remove("<QRLC_");

         qr.remove(">");

         qrPos = qr.split('_');

         if (qrPos.size() == 2){

           y = qrPos[0].toInt();
           x = qrPos[1].toInt();

           return QRLC;

         }

       }
   }

   if ((pos = Text.indexOf("<QRRDLC_")) >= 0){

       if ((pos = Text.indexOf(">")) != -1){

         QString qr = Text.mid(0, pos + 1);
         QStringList qrPos ;

         Text.remove(qr);

         Text = Text.trimmed();

         qr.remove("<QRRDLC_");

         qr.remove(">");

         qrPos = qr.split('_');

         if (qrPos.size() == 2){

           y = qrPos[0].toInt();
           x = qrPos[1].toInt();

           return QRRDLC;
         }

       }
   }

   return Line;
}

QImage QPrint::GeneratedQR(const QString& data, int size)
{
   QQREncode QREncode;

   QREncode.setVersion(0);

   QREncode.setLevel(QQREncode::MEDIUM);

   QREncode.encode(data, true);

   return QREncode.toQImage(size);
}

bool QPrint::drawPDF(QPrinter &printer, QPainter &painter, const QString& data)
{
    QPageLayout layout = printer.pageLayout();
    QRect rect = layout.fullRectPixels(printer.physicalDpiX());
    Poppler::Document *newdoc = Poppler::Document::loadFromData(QByteArray::fromBase64(data.toLatin1()));
    //auto res = printer.resolution();

    qDebug() << "rect=" << rect;

    rect.adjust(0, 0, -100, -100);

    if (!newdoc)
      return false;

    newdoc->setRenderHint(Poppler::Document::TextAntialiasing);
    newdoc->setRenderHint(Poppler::Document::Antialiasing);
    newdoc->setRenderHint(Poppler::Document::TextHinting);
    newdoc->setRenderHint(Poppler::Document::TextSlightHinting);
    newdoc->setRenderHint(Poppler::Document::ThinLineSolid);

    newdoc->setRenderBackend(Poppler::Document::ArthurBackend);

    int numPages = newdoc->numPages();

    for (int i = 0; i < numPages; i++){

      Poppler::Page *page = newdoc->page(i);

      QImage image = page->renderToImage(PDF_X_RES, PDF_X_RES);

      drawPage(painter, image, rect);

      //page->renderToPainter(&painter, res, res);

      if (i < (numPages - 1))
        printer.newPage();

      delete page;

    }

    delete newdoc;

    newdoc = nullptr;

    return true;

}

void QPrint::drawPage(QPainter &painter, QImage &image, const QRect& rect)
{
    int left;
    int top;

    image = image.scaled(rect.size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);

    left = qMax(0, qRound((rect.width() - image.width()) / 2.0)) + rect.left();

    top = qMax(0, qRound((rect.height() - image.height()) / 2.0)) + rect.top();

    painter.drawImage(QPoint(left, 0), image);

}


int QPrint::GetPosition(int resolution, int position)
{
   if (Mode != QPrinter::ScreenResolution){

     QPrinter printer(QPrinter::ScreenResolution);

     position = (resolution * position) / printer.resolution();

   }

  return position;
}


int QPrint::GetPositionX(const QRect& rect, int position)
{
  return qRound((rect.width() * position) / LETTER_WIDTH_PIXELS);
}

int QPrint::GetPositionY(const QRect& rect, int position)
{
  return qRound((rect.height() * position) / LETTER_HEIGHT_PIXELS);
}

bool QPrint::isMatrix(QPrinter &printer){

  if (printer.outputFormat() == QPrinter::PdfFormat)
    return false;

  QList<int> resolutions = printer.supportedResolutions();

  int maxRes = 0;

  for ( const auto& resolution : resolutions  ) {

      if (resolution > maxRes)
        maxRes = resolution;
  }

  return maxRes >= 0 && maxRes < 300;
}

int QPrint::LoadResolution()
{
    QString file = QCoreApplication::applicationDirPath() + QDir::separator() + "SmartPrintSin.ini";
    QSettings settings(file, QSettings::IniFormat);

    int resolution = settings.value("RESOLUTION", 0).toInt();

    return resolution;
}

QString QPrint::LoadPDF(const QString& fileName)
{
  QFile file(fileName);
  QByteArray data;

  if (!file.open(QIODevice::ReadOnly))
    return "";

  data = file.readAll();

  return data.toBase64();
}

void QPrint::run()
{
   exec();
}
