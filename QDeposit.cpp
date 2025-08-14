#include <QPrinter>
#include <QJsonArray>
#include <QApplication>
#include <QDir>
#include <QDateTime>
#include <QPainter>
#include <QDebug>

#include "QDeposit.h"
#include "global.h"

QDeposit::QDeposit(const QString& printerName, const QString& pdfdir, const QJsonDocument& json, bool leftalign, int from, int to, QPrinter::PrinterMode mode, QPrinter::Orientation orientation)
    :QPrint(printerName, pdfdir, json, leftalign, from, to, mode, orientation)
{
}

QImage QDeposit::toImage(const QJsonObject& object, const QRect& rect, int &left)
{
  QByteArray data = QByteArray::fromBase64(object["data"].toString().toLatin1());
  QImage image = QImage::fromData(data);

  if ((image.width() <= rect.width()) &&  (image.height() <= rect.height())){

    left = qMax(0, qRound((rect.width() - image.width()) / 2.0)) + rect.left();

    return  image;
  }

  left = rect.left();

  return image.scaled(rect.size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
}

int QDeposit::paintLines(QPainter& painter, const QJsonArray& lines, const QRect& rect, const QRect& fullRect, int resolution)
{
    QString FontName = FONT_NAME;
    QString Format = FORMAT;

    double NormalPointSize = NORMAL_POINT_SIZE;
    int NormalStretch = NORMAL_STRETCH;
    int NormalLetterSpacing = NORMAL_LETTER_SPACING;
    int NormalWeight = NORMAL_WEIGHT;
    double BoldPointSize = BOLD_POINT_SIZE;
    int BoldStretch = BOLD_STRETCH;
    int BoldLetterSpacing = BOLD_LETTER_SPACING;
    int BoldWeight = BOLD_WEIGHT;
    int dySpacing = DY_SPACING;

#if defined(Q_OS_WIN)

    if (Mode == QPrinter::HighResolution){
      NormalLetterSpacing -= 7;
      BoldLetterSpacing  -=  7;
      dySpacing  -=  1;
    }

#endif


    QFont normalFont = QFont(FontName);
    QImage qr;
    QString text, data;

    normalFont.setPointSizeF(NormalPointSize);

    normalFont.setStretch(NormalStretch);

    normalFont.setLetterSpacing(QFont::PercentageSpacing, NormalLetterSpacing);

    normalFont.setWeight(NormalWeight);

    painter.setFont(normalFont);

    TagTypes tag;

    int marginX = rect.left();
    int marginY = rect.top();
    int lineSpacing = painter.fontMetrics().height();
    int CharWidth = qRound(painter.fontMetrics().size(Qt::TextSingleLine, "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz ").width() / 54.0);
    int qrX, qrY;
    int lineX = marginX;
    int lineY = marginY;
    int lineRight = 0;
    int qrDY = 0;

    bool newPage = false;

    int size = lines.size();

    for (int i = 0;  i < size && lineY < rect.bottom(); i++){

      text = lines[i].toString();

      tag =  GetTagType(rect, text, qrX, qrY, newPage);

      switch(tag){

      case Bold:{

          QFont boldFont = QFont(FontName);

          boldFont.setPointSizeF(BoldPointSize);

          boldFont.setStretch(BoldStretch);

          boldFont.setLetterSpacing(QFont::PercentageSpacing, BoldLetterSpacing);

          boldFont.setWeight(BoldWeight);

          painter.setFont(boldFont);

          painter.drawText(lineX, lineY, text);

          text = trimRight(text);

          int right = lineX + painter.fontMetrics().size(0, text).width();

          if (lineRight < right)
            lineRight = right;

          lineY += lineSpacing;

          painter.setFont(normalFont);

          if (qrDY){

            qrDY -= lineSpacing;

            if (qrDY <= 0) {

              lineX = marginX;

              lineY = qrY +  qr.height() + lineSpacing;

              qrDY = 0;

            }
          }

          text.clear();

      }
      break;
      case QR:{

          data.clear();

          if ((i + 1) < size){

            i++;

            data = lines[i].toString();

            data = data.trimmed();
          }

          text += data;

          if (text.isEmpty())
            continue;

          qr = GeneratedQR(text);

          qrX = lineX + MM_TO_PIXEL(QR_MARGIN_MM, resolution);
          qrY = lineY;

          painter.drawImage(QPoint(qrX, qrY), qr);

          lineX = qrX + qr.width() + MM_TO_PIXEL(QR_MARGIN_MM, resolution);

          qrDY = qr.height();

          text.clear();
      }

      break;
      case QRXY:{

          data.clear();

          if ((i + 1) < size){

            i++;

            data = lines[i].toString();

            data = data.trimmed();
          }

          text += data;

          if (text.isEmpty())
            continue;

          qr = GeneratedQR(text);

          qrX = GetPositionX(fullRect, qrX);

          qrY = GetPositionY(fullRect, qrY);

          painter.drawImage(QPoint(qrX, qrY), qr);

          text.clear();
      }
      break;
      case QRRD:{

          data.clear();

          if ((i + 1) < size){

            i++;

            data = lines[i].toString();

            data = data.trimmed();
          }

          text += data;

          if (text.isEmpty())
            continue;

          qr = GeneratedQR(text, MM_TO_PIXEL(BARCODE_RD_SIZE_MM, resolution));

          qrX = lineX + MM_TO_PIXEL(QR_MARGIN_MM, resolution);
          qrY = lineY;

          painter.drawImage(QPoint(qrX, qrY), qr);

          lineX = qrX + qr.width() + MM_TO_PIXEL(QR_MARGIN_MM, resolution);

          qrDY = qr.height();

          text.clear();
      }

      break;
      case QRRDXY:{

          data.clear();

          if ((i + 1) < size){

            i++;

            data = lines[i].toString();

            data = data.trimmed();
          }

          text += data;

          if (text.isEmpty())
            continue;

          qr = GeneratedQR(text, MM_TO_PIXEL(BARCODE_RD_SIZE_MM, resolution));

          qrX = GetPositionX(fullRect, qrX);

          if (!leftAlign){

            qrX = lineRight - qr.width();

            if (qrX <= 0)
              qrX = rect.width() - (qr.width() + marginX);

          }

          qrY = GetPositionY(fullRect, qrY);

          painter.drawImage(QPoint(qrX, qrY), qr);

          text.clear();
      }
      break;
      case QREND:{

          data.clear();

          if ((i + 1) < size){

            i++;

            data = lines[i].toString();

            data = data.trimmed();
          }

          text += data;

          if (text.isEmpty())
            continue;

          qr = GeneratedQR(text);

          if (leftAlign)
            qrX = lineRight - (qr.width() + marginX + marginX);
          else
            qrX = lineRight - qr.width();

          if (qrX <= 0)
            qrX = rect.width() - (qr.width() + marginX);

          qrY = lineY + lineSpacing - qr.height();

          painter.drawImage(QPoint(qrX, qrY), qr);

          lineX = marginX;

          lineY = qrY + qr.height() + lineSpacing;

          text.clear();
      }

      break;
      case QRLC:{

          data.clear();

          if ((i + 1) < size){

            i++;

            data = lines[i].toString();

            data = data.trimmed();
          }

          text += data;

          if (text.isEmpty())
            continue;

          qr = GeneratedQR(text);

          qrX = (qrX * CharWidth) + marginX;

          qrY = (qrY * lineSpacing) + marginY;

          painter.drawImage(QPoint(qrX, qrY), qr);

          text.clear();
      }
      break;
      case QRRDLC:{

          data.clear();

          if ((i + 1) < size){

            i++;

            data = lines[i].toString();

            data = data.trimmed();
          }

          text += data;

          if (text.isEmpty())
            continue;

          qr = GeneratedQR(text, MM_TO_PIXEL(BARCODE_RD_SIZE_MM, resolution));

          qrX = (qrX * CharWidth) + marginX;

          qrY = (qrY * lineSpacing) + marginY;

          painter.drawImage(QPoint(qrX, qrY), qr);

          text.clear();
      }
      break;
      default:

          painter.drawText(lineX, lineY, text);

          text = trimRight(text);

          int right = lineX + painter.fontMetrics().size(0, text).width();

          if (lineRight < right)
            lineRight = right;

          lineY += lineSpacing;

          if (qrDY){

            qrDY -= lineSpacing;

            if (qrDY <= 0) {

              lineX = marginX;

              lineY = qrY +  qr.height() + lineSpacing;

              qrDY = 0;

            }
          }

          text.clear();

      break;
      }

    }

    return lineY;
}

void QDeposit::exec()
{
  QPrinter printer(Mode);
  QJsonArray images = JSon["images"].toArray();
  QJsonDocument jsonReply;
  QPainter painter;
  QDateTime dateTime = QDateTime::currentDateTime();
  QString FontName;
  QString Format;
  QString fileName;

  int resolution = LoadResolution();

  if (resolution > 0)
   printer.setResolution(resolution);

  printer.setPageSize(QPrinter::Letter);

  printer.setOrientation(Orientation);

#if defined(Q_OS_UNIX)
  Format = "yyyy-MM-dd hh:mm:zzz";
  FontName = "Courier Prime";
#else
  Format = "yyyy-MM-dd hh_mm_zzz";
  FontName = "Courier New";
#endif

  if (!JSon["output"].toString().compare("printer",Qt::CaseInsensitive))
     printer.setPrinterName(Name);
  else {

     QDir dir(pdf);

     if (!dir.exists())
       jsonReply =  Reply(PDF_DIR_ERROR, Errors[PDF_DIR_ERROR]);
     else {

       QString printName = dateTime.toString(Format) + ".pdf";
       fileName = pdf +  QDir::separator() + printName;

       printer.setOutputFormat(QPrinter::PdfFormat);
       printer.setOutputFileName(fileName);
    }
  }

  if (jsonReply.isEmpty()){

  printer.setDocName("SmarPrintSin - " + dateTime.toString(Format));

  printer.setFullPage(true);

  if (!painter.begin(&printer)){

     jsonReply =  Reply(PRINTER_ERROR, Errors[PRINTER_ERROR]);

  }
  else {

        int marginX = MM_TO_PIXEL(TOP_MARGIN_MM, printer.resolution());
        int marginY = MM_TO_PIXEL(TOP_MARGIN_MM, printer.resolution());

        From = !From ? 0 : From -1;

        To = !To ? images.size() : To;

        for (int i = From;  i < To; i++){

          QJsonObject object = images[i].toObject();
          QPageLayout layout = printer.pageLayout();
          QRect fullRect = layout.fullRectPixels(printer.resolution());
          QRect rect = fullRect;
          QRect textRect = rect;
          QJsonArray lines = object["lines"].toArray();
          QImage image;
          int left;

          if (layout.orientation() == QPageLayout::Portrait)
            textRect.setBottom(qRound(rect.height() * 0.50));
          else
            textRect.setBottom(qRound(rect.width() * 0.50));

          textRect = textRect.marginsAdded(QMargins(-marginX, -marginY, -marginX, -marginY));

          int top = paintLines(painter, lines, textRect, fullRect, printer.resolution());

          rect.setTop(top);

          rect = rect.marginsAdded(QMargins(-marginX, -marginY, -marginX, -marginY));

          image = toImage(object, rect, left);

          painter.drawImage(QPoint(left, top), image);

          if (i < (To - 1))
            printer.newPage();
        }

        painter.end();

        jsonReply =  Reply(OPERATION_SUCCESSFULLY, Errors[OPERATION_SUCCESSFULLY], isPDFMem ? LoadPDF(fileName) : "");
   }
  }

  emit response(jsonReply);

  if (isPDFMem)
    QFile::remove(fileName);

  emit finished();
}
