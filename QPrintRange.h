#ifndef QPRINTRANGE_H
#define QPRINTRANGE_H

#include <QDialog>
#include <QPrinter>

namespace Ui {
class QPrintRange;
}

class QPrintRange : public QDialog
{
    Q_OBJECT

public:
    QPrintRange(int pages, bool bOrientation);
    ~QPrintRange();

public:
    int From(){return from;}
    int To(){return to;}
    QPrinter::Orientation Orientation();

public slots:
    void cancel();

private slots:
    void on_range_clicked(bool checked);

    void on_all_clicked(bool checked);

    void on_print_clicked();

    void on_from_textChanged(const QString &arg1);

    void on_to_textChanged(const QString &arg1);

    void on_cancel_clicked();

protected:
    int Pages;
    int from = 0;
    int to = 0;

private:
    Ui::QPrintRange *ui;
};

#endif // QPRINTRANGE_H
