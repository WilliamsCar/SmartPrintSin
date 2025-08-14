#include <QRegExpValidator>
#include <QMessageBox>
#include <QDebug>
#include "QPrintRange.h"
#include "ui_QPrintRange.h"

QPrintRange::QPrintRange(int pages, bool bOrientation) :
    QDialog(nullptr), Pages(pages),
    ui(new Ui::QPrintRange)
{
    ui->setupUi(this);

    qApp->setWindowIcon(QIcon(":/resources/pages.png"));

    setWindowFlags((windowFlags() | Qt::CustomizeWindowHint | Qt::WindowStaysOnTopHint) & ~Qt::WindowContextHelpButtonHint);

    ui->from->setValidator(new QRegExpValidator(QRegExp("[1-9][0-9]*")));

    ui->to->setValidator(new QRegExpValidator(QRegExp("[1-9][0-9]*")));

    ui->from->setText("1");

    ui->to->setText(QString("%1").arg(Pages));

    QStringList orientations;

    orientations << "Vertical" << "Horizontal";

    ui->orientation->addItems(orientations);

    ui->orientation->setCurrentIndex(0);

    ui->all->setEnabled(Pages > 1);

    ui->range->setEnabled(Pages > 1);

    ui->orientation->setEnabled(bOrientation);
}

QPrintRange::~QPrintRange()
{
    delete ui;
}


QPrinter::Orientation QPrintRange::Orientation()
{
    if (!ui->orientation->currentText().compare(tr("Vertical"), Qt::CaseInsensitive))
      return QPrinter::Portrait;

    return QPrinter::Landscape;
}


void QPrintRange::cancel()
{
   QDialog::reject();
}

void QPrintRange::on_range_clicked(bool checked)
{
    ui->from->setEnabled(checked);
    ui->to->setEnabled(checked);

    ui->print->setEnabled(checked && !ui->to->text().isEmpty() && !ui->from->text().isEmpty());
}

void QPrintRange::on_all_clicked(bool checked)
{
    ui->print->setEnabled(checked);
    ui->from->setEnabled(!checked);
    ui->to->setEnabled(!checked);

    if (checked)
      from = to = 0;
}

void QPrintRange::on_print_clicked()
{
    int iFrom = ui->from->text().toInt();
    int iTo = ui->to->text().toInt();

    if (iFrom < 1 || iFrom > Pages){

        QString msg = QString("Rango de páginas no válido");

        QMessageBox::warning(this, "SmartPrintSin", msg);

        return;
    }

    if (iTo < 1 || iTo > Pages){

        QString msg = QString("Rango de páginas no válido");

        QMessageBox::warning(this, "SmartPrintSin", msg);

        return;
    }

    if (iFrom > iTo ){

        QString msg = QString("Rango de páginas no válido");

        QMessageBox::warning(this, "SmartPrintSin", msg);

        return;
    }

    from = iFrom;

    to = iTo;

    QDialog::accept();
}

void QPrintRange::on_from_textChanged(const QString &arg1)
{
    ui->print->setEnabled(!arg1.isEmpty() && !ui->to->text().isEmpty());
}

void QPrintRange::on_to_textChanged(const QString &arg1)
{
    ui->print->setEnabled(!arg1.isEmpty() && !ui->from->text().isEmpty());
}

void QPrintRange::on_cancel_clicked()
{
     QDialog::reject();
}
