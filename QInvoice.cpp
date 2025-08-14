#include <QPrinter>
#include <QJsonArray>
#include <QApplication>
#include <QDir>
#include <QDateTime>
#include <QPainter>
#include <QDebug>
#include <QMessageLogger>
#include <QTemporaryFile>
#include <QDesktopServices>
#include "QInvoice.h"
#include "global.h"

QInvoice::QInvoice(const QString& printerName, const QString& pdfdir, const QJsonDocument& json, bool leftalign, int from, int to, QPrinter::PrinterMode mode, QPrinter::Orientation orientation)
 :QPrint(printerName, pdfdir, json, leftalign, from, to, mode, orientation)
{
}


void QInvoice::removelastlines(QJsonArray &lines)
{
   int size = lines.size();

   while((size = lines.size()) > 1){

       QString line = lines[size-1].toString();

       line = line.trimmed();

       if (line.isEmpty())
         lines.removeLast();
       else if (line.contains("@PB", Qt::CaseInsensitive))
         lines.removeLast();
       else if (line.contains(0x0C, Qt::CaseInsensitive))
         lines.removeLast();
       else
         break;

   }
}


void QInvoice::exec()
{
  QPrinter printer(Mode);
  QJsonArray lines = LinesToPrint(JSon["lines"].toArray());
  QJsonDocument jsonReply;
  QPainter painter;
  QDateTime dateTime = QDateTime::currentDateTime();

  QString FontName = FONT_NAME;
  QString Format = FORMAT;

  QRect fullRect;

  QString fileName;

  int resolution = LoadResolution();

  if (resolution > 0)
    printer.setResolution(resolution);

  printer.setPageSize(QPrinter::Letter);

  printer.setOrientation(Orientation);

  if (!JSon["output"].toString().compare("printer",Qt::CaseInsensitive))
     printer.setPrinterName(Name);
  else {

     QDir dir(pdf);

     if (!dir.exists())
       jsonReply =  Reply(PDF_DIR_ERROR, Errors[PDF_DIR_ERROR]);
     else {

       if (!isShowPDF)  {

         QString printName = dateTime.toString(Format) + ".pdf";

         fileName = pdf +  QDir::separator() + printName;

       } else {

         QTemporaryFile temporaryFile(QDir::tempPath() + QDir::separator() + "SPSXXXXXX.pdf");

         temporaryFile.open();

         fileName = temporaryFile.fileName();

       }

       printer.setOutputFormat(QPrinter::PdfFormat);
       printer.setOutputFileName(fileName);

       leftAlign = false;
    }
  }

  if (jsonReply.isEmpty()){

    printer.setDocName("SmarPrintSin - " + dateTime.toString(Format));

    printer.setFullPage(true);

    fullRect = printer.pageLayout().fullRectPixels(printer.resolution());

  if (!painter.begin(&printer)){

     jsonReply =  Reply(PRINTER_ERROR, Errors[PRINTER_ERROR]);

  }
  else {

      QFont normalFont = QFont(FontName);
      QImage qr;
      QString text, data;

      double NormalPointSize = NORMAL_POINT_SIZE;
      int NormalStretch = NORMAL_STRETCH;
      int NormalLetterSpacing = !leftAlign ? NORMAL_LETTER_SPACING : MATRIX_NORMAL_LETTER_SPACING;
      int NormalWeight = NORMAL_WEIGHT;
      double BoldPointSize = BOLD_POINT_SIZE;
      int BoldStretch = BOLD_STRETCH;
      int BoldLetterSpacing = BOLD_LETTER_SPACING;
      int BoldWeight = BOLD_WEIGHT;
      int dySpacing = DY_SPACING;
      bool bEntel = isEntel(lines);

    #if defined(Q_OS_WIN)

        if (Mode == QPrinter::HighResolution){
          NormalLetterSpacing -= 7;
          BoldLetterSpacing  -=  7;
          dySpacing  -=  1;
        }

    #endif

        if (bEntel){

          NormalLetterSpacing += 12;

          BoldLetterSpacing += 12;

        }

      normalFont.setPointSizeF(NormalPointSize);

      normalFont.setStretch(NormalStretch);

      normalFont.setLetterSpacing(QFont::PercentageSpacing, NormalLetterSpacing);

      normalFont.setWeight(NormalWeight);

      painter.setFont(normalFont);

      TagTypes tag, last_tag = Line;

      int marginX = MM_TO_PIXEL(!leftAlign ? LEFT_MARGIN_MM : MATRIX_LEFT_MARGIN_MM, printer.resolution());
      int marginY = MM_TO_PIXEL(!leftAlign ? TOP_MARGIN_MM : MATRIX_TOP_MARGIN_MM, printer.resolution());
      int lineSpacing = painter.fontMetrics().height();
      int CharWidth = qRound(painter.fontMetrics().size(Qt::TextSingleLine, "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz ").width() / 54.0);
      int qrX, qrY;
      int lineX = marginX;
      int lineY = From > 1 ? (bEntel ? marginY : marginY + lineSpacing) : marginY;
      int lineRight = 0;
      int qrDY = 0;
      bool newPage = false;

      if (!leftAlign)
        marginX = qMax(qRound((fullRect.size().width() - GetMaxLineSize(lines, painter, printer))/2.0), marginX);
      else
        GetMaxLineSize(lines, painter, printer);

      lineX = marginX;

      removelastlines(lines);

      int size = lines.size();

      for (int i = 0;  i < size; i++){

        text = lines[i].toString();

        tag =  GetTagType(fullRect, text, qrX, qrY, newPage);

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

            last_tag = tag;

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

            qrX = lineX + MM_TO_PIXEL(QR_MARGIN_MM, printer.resolution());
            qrY = lineY;

            painter.drawImage(QPoint(qrX, qrY), qr);

            lineX = qrX + qr.width() + MM_TO_PIXEL(QR_MARGIN_MM, printer.resolution());

            qrDY = qr.height();

            text.clear();

            last_tag = tag;
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

            last_tag = tag;
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

            qr = GeneratedQR(text, MM_TO_PIXEL(BARCODE_RD_SIZE_MM, printer.resolution()));

            qrX = lineX + MM_TO_PIXEL(QR_MARGIN_MM, printer.resolution());
            qrY = lineY;

            painter.drawImage(QPoint(qrX, qrY), qr);

            lineX = qrX + qr.width() + MM_TO_PIXEL(QR_MARGIN_MM, printer.resolution());

            qrDY = qr.height();

            text.clear();

            last_tag = tag;
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

            qr = GeneratedQR(text, MM_TO_PIXEL(BARCODE_RD_SIZE_MM, printer.resolution()));

            qrX = GetPositionX(fullRect, qrX);

            if (!leftAlign){

              qrX = lineRight - qr.width();

              if (qrX <= 0)
                qrX = fullRect.width() - (qr.width() + marginX);

            }

            qrY = GetPositionY(fullRect, qrY);

            painter.drawImage(QPoint(qrX, qrY), qr);

            text.clear();

            last_tag = tag;
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
              qrX = fullRect.width() - (qr.width() + marginX);

            qrY = lineY + lineSpacing - qr.height();

            painter.drawImage(QPoint(qrX, qrY), qr);

            lineX = marginX;

            lineY = qrY + qr.height() + lineSpacing;

            text.clear();

            last_tag = tag;
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

            last_tag = tag;
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

            qr = GeneratedQR(text, MM_TO_PIXEL(BARCODE_RD_SIZE_MM, printer.resolution()));

            qrX = (qrX * CharWidth) + marginX;

            qrY = (qrY * lineSpacing) + marginY;

            painter.drawImage(QPoint(qrX, qrY), qr);

            text.clear();

            last_tag = tag;
        }
        break;
        case PDF:{

            if (text.isEmpty())
              continue;

            lineX = marginX;
            lineY = marginY;

            if (newPage){

              lineY = bEntel ? marginY : marginY + lineSpacing;

              qrDY = lineRight = 0;

              printer.newPage();

            } else if (last_tag == PDF){

              lineY = bEntel ? marginY : marginY + lineSpacing;

              qrDY = lineRight = 0;

              printer.newPage();

            }

            drawPDF(printer, painter, text);

            text.clear();

            last_tag = tag;
        }
        break;
        case NewPage:

            lineX = marginX;
            lineY = bEntel ? marginY : marginY + lineSpacing;

            qrDY = lineRight = 0;

            printer.newPage();

            last_tag = tag;

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

            last_tag = tag;

        break;
        }
      }

      painter.end();

      jsonReply =  Reply(OPERATION_SUCCESSFULLY, Errors[OPERATION_SUCCESSFULLY], isPDFMem ? LoadPDF(fileName) : "");
   }
  }

  emit response(jsonReply);

  if (isShowPDF)
    QDesktopServices::openUrl(QUrl::fromLocalFile(fileName));

  if (isPDFMem && !isShowPDF)
    QFile::remove(fileName);

  emit finished();
}
