#include <QPrinterInfo>
#include <QSettings>
#include <QDir>
#include <QDebug>
#include <QFileDialog>
#include <QMessageBox>
#include <QStandardPaths>
#include "QSelectPrinter.h"
#include "ui_QSelectPrinter.h"
#include "global.h"


QSelectPrinter::QSelectPrinter(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::QSelectPrinter)
{
    ui->setupUi(this);

    qApp->setWindowIcon(QIcon(":/resources/print.png"));

    QString file = QCoreApplication::applicationDirPath() + QDir::separator() + "SmartPrintSin.ini";
    QSettings settings(file, QSettings::IniFormat);
    int index;

    printer = curprinter = settings.value("PRINTER").toString();
    printDir = settings.value("PRINTDIR").toString();
    pdfDir = settings.value("PDFDIR").toString();
    bLeft = settings.value("LEFT", true).toBool();

    setWindowFlags((windowFlags() | Qt::CustomizeWindowHint) & ~Qt::WindowContextHelpButtonHint);

    QStringList printers = QPrinterInfo::availablePrinterNames();

    ui->printers->addItems(printers);

    ui->printers->setCurrentIndex(-1);

    bFill = false;

    if (printer.isEmpty())
      printer = QPrinterInfo::defaultPrinter().printerName();

    index = ui->printers->findText(printer, Qt::MatchExactly);

    if (index == -1){

      printer = QPrinterInfo::defaultPrinter().printerName();

      index = ui->printers->findText(printer, Qt::MatchExactly);
    }

    if (!printDir.isEmpty() && !QDir(printDir).exists())
      printDir.clear();

    if (!pdfDir.isEmpty() && !QDir(pdfDir).exists())
      pdfDir.clear();

    ui->printers->setCurrentIndex(index);

    ui->left->setChecked(bLeft);

    ui->center->setChecked(!bLeft);

    defaultvalues();

    PrinterName = printer;

    PRINTDirectory = printDir;

    PDFDirectory = pdfDir;

    LeftAlign = bLeft;

    ui->accept->setEnabled(!printer.isEmpty() && !pdfDir.isEmpty() && !printDir.isEmpty());
}

QSelectPrinter::~QSelectPrinter()
{
    delete ui;
}

void QSelectPrinter::save()
{
    QString file = QCoreApplication::applicationDirPath() + QDir::separator() + "SmartPrintSin.ini";
    QSettings settings(file, QSettings::IniFormat);

    settings.setValue("PRINTER", printer);

    settings.setValue("PRINTDIR", printDir);

    settings.setValue("PDFDIR", pdfDir);

    settings.setValue("LEFT", bLeft);

    PrinterName = printer;

    PRINTDirectory = printDir;

    PDFDirectory = pdfDir;

    LeftAlign = bLeft;

    ui->printDir->setText(printDir);

    ui->pdfDir->setText(pdfDir);
}

void QSelectPrinter::defaultvalues()
{
    if (printDir.isEmpty()){

      QDir dir;

      #if defined(Q_OS_UNIX)
        printDir = QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + "/SpoolerSintesis";
      #else
        printDir = "C:/SpoolerSintesis";
      #endif

      if (!dir.mkpath(printDir))
        printDir.clear();

    }

    if (pdfDir.isEmpty()){

      QDir dir;

      pdfDir = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/PDF";

      if (!dir.mkpath(pdfDir))
        pdfDir.clear();

    }

    save();
}


bool QSelectPrinter::IsLeft(const QString &name)
{
    QPrinter printer(QPrinter::ScreenResolution);

    printer.setPrinterName(name);

    QList<int> resolutions = printer.supportedResolutions();

    int maxRes = 0;

    for ( const auto& resolution : resolutions  ) {

        if (resolution > maxRes)
          maxRes = resolution;
    }

    return maxRes >= 0 && maxRes < 300;
}


void QSelectPrinter::on_cancel_clicked()
{
    QDialog::reject();
}

void QSelectPrinter::on_accept_clicked()
{
    save();

    QDialog::accept();
}

void QSelectPrinter::on_printers_currentIndexChanged(const QString &arg1)
{
    if (bFill)
      return;

    printer = arg1;

    if (curprinter != printer){

      curprinter.clear();

      bLeft = IsLeft(printer);

      ui->left->setChecked(bLeft);
      ui->center->setChecked(!bLeft);

    }

    ui->accept->setEnabled(!printer.isEmpty() && !pdfDir.isEmpty() && !printDir.isEmpty());
}

void QSelectPrinter::on_selPdfDir_clicked()
{
    QString path = QFileDialog::getExistingDirectory(this, tr("Seleccionar directorio para PDF"),
                                                        QDir::homePath(),
                                                     QFileDialog::ShowDirsOnly
                                                     | QFileDialog::DontResolveSymlinks);

    if (!path.isEmpty()){

        pdfDir = path;

        ui->pdfDir->setText(pdfDir);
    }

    ui->accept->setEnabled(!printer.isEmpty() && !pdfDir.isEmpty() && !printDir.isEmpty());
}

void QSelectPrinter::on_selPrintDir_clicked()
{
    QString path = QFileDialog::getExistingDirectory(this, tr("Seleccionar directorio de archivos a imprimir"),
                                                        QDir::homePath(),
                                                     QFileDialog::ShowDirsOnly
                                                     | QFileDialog::DontResolveSymlinks);

    if (!path.isEmpty()){

        printDir = path;

        ui->printDir->setText(printDir);
    }

    ui->accept->setEnabled(!printer.isEmpty() && !pdfDir.isEmpty() && !printDir.isEmpty());
}

void QSelectPrinter::on_left_clicked(bool checked)
{
    bLeft = checked;
}

void QSelectPrinter::on_center_clicked(bool checked)
{
    bLeft = !checked;
}
