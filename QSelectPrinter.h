#ifndef QSELECTPRINTER_H
#define QSELECTPRINTER_H

#include <QDialog>

namespace Ui {
class QSelectPrinter;
}

class QSelectPrinter : public QDialog
{
    Q_OBJECT

public:
    explicit QSelectPrinter(QWidget *parent = nullptr);
    ~QSelectPrinter();

private:
    void save();
    void defaultvalues();
    bool IsLeft(const QString &name);

private slots:
    void on_cancel_clicked();

    void on_accept_clicked();

    void on_printers_currentIndexChanged(const QString &arg1);

    void on_selPdfDir_clicked();

    void on_selPrintDir_clicked();

    void on_left_clicked(bool checked);

    void on_center_clicked(bool checked);

protected:
    QString printer, curprinter;
    QString printDir;
    QString pdfDir;
    bool bLeft;
    bool bFill = true;

private:
    Ui::QSelectPrinter *ui;
};

#endif // QSELECTPRINTER_H
