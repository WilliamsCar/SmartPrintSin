#ifndef QPRINT_H
#define QPRINT_H

#include <QObject>
#include <QJsonDocument>
#include <QImage>
#include <QPrinter>

#include "qgsdelimitedtextfile.h"

#define MM_TO_PIXEL(MM, RES) (qRound((MM * RES)/25.4))

#define MAX_PAGES 5

#define MAX_LINES 128

#define QR_MARGIN_MM 5
#define MAX_QR_LINES 6

#define PDF_X_RES 600
#define PDF_Y_RES 600

#if defined(Q_OS_UNIX)
#define LEFT_MARGIN_MM          0
#define TOP_MARGIN_MM           2
#define MATRIX_LEFT_MARGIN_MM   5
#define MATRIX_TOP_MARGIN_MM    0
#define FORMAT                  "yyyy-MM-dd hh:mm:zzz"
#define FONT_NAME               "Courier Prime"
#define NORMAL_POINT_SIZE       10.5
#define NORMAL_STRETCH          60
#define NORMAL_LETTER_SPACING   100
#define MATRIX_NORMAL_LETTER_SPACING 110
#define NORMAL_WEIGHT           QFont::Normal
#define BOLD_POINT_SIZE         10.5
#define BOLD_STRETCH            60
#define BOLD_LETTER_SPACING     100
#define BOLD_WEIGHT             QFont::DemiBold
#define DY_SPACING              1
#define BARCODE_SIZE_MM         25
#define BARCODE_RD_SIZE_MM      22
#else
#define LEFT_MARGIN_MM          0
#define TOP_MARGIN_MM           2
#define MATRIX_LEFT_MARGIN_MM   5
#define MATRIX_TOP_MARGIN_MM    0
#define FORMAT                  "yyyy-MM-dd hh_mm_zzz"
#define FONT_NAME               "Courier New"
#define NORMAL_POINT_SIZE       10.1
#define NORMAL_STRETCH          65
#define NORMAL_LETTER_SPACING   100
#define MATRIX_NORMAL_LETTER_SPACING 110
#define NORMAL_WEIGHT           QFont::Normal
#define BOLD_POINT_SIZE         10
#define BOLD_STRETCH            55
#define BOLD_LETTER_SPACING     100
#define BOLD_WEIGHT             QFont::DemiBold
#define DY_SPACING              1
#define BARCODE_SIZE_MM         25
#define BARCODE_RD_SIZE_MM      22
#endif

class QPrint : public QObject
{

Q_OBJECT

public:

    enum TagTypes{
        Line = 0,
        Bold = 1,
        QR = 2,
        QRXY = 3,
        QRRD = 4,
        QRRDXY = 5,
        NewPage = 6,
        PDF = 7,
        QREND = 8,
        QRLC = 9,
        QRRDLC = 10
    };

    Q_ENUM(TagTypes)


public:
   QPrint(const QString& printerName, const QString& pdfdir, const QJsonDocument& json, bool leftalign, int from = 0, int to = 0, QPrinter::PrinterMode mode = QPrinter::ScreenResolution, QPrinter::Orientation orientation = QPrinter::Portrait);

public:
   static QString trimRight(const QString& str);

protected:
    bool isEntel(QgsDelimitedTextFile &file);
    bool isEntel(const QJsonArray& lines);
    int GetMaxLineSize(QgsDelimitedTextFile &file, QPainter &painter, QPrinter &printer);
    int GetMaxLineSize(const QJsonArray& lines, QPainter &painter, QPrinter &printer);
    QJsonArray LinesToPrint(const QJsonArray&  lines);
    TagTypes GetTagType(QString& Text);
    TagTypes GetTagType(const QRect& rect, QString& Text, int& x, int& y, bool &newPage);
    QImage GeneratedQR(const QString& data, int size = 0);
    bool drawPDF(QPrinter &printer, QPainter &painter, const QString& data);
    void drawPage(QPainter &painter, QImage &image, const QRect& rect);
    int GetPosition(int resolution, int position);
    int GetPositionX(const QRect& rect, int position);
    int GetPositionY(const QRect& rect, int position);
    bool isMatrix(QPrinter &printer);
    int LoadResolution();
    QString LoadPDF(const QString& fileName);

    virtual void exec() = 0;

protected slots:
    void run();

signals:
     void response(const QJsonDocument &json);
     void finished();


protected:
    QString Name;
    QString pdf;
    QJsonDocument JSon;
    bool leftAlign;

    int From;
    int To;

    QPrinter::PrinterMode Mode;

    QPrinter::Orientation Orientation;

    bool isPDFMem = false;

    bool isShowPDF = false;
};

#endif // QPRINT_H
