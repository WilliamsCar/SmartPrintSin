#include "QSmartPrintSinApp.h"
#include "QSelectPrinter.h"
#include <QWindowList>
#include <QTimer>

#include "global.h"


QSmartPrintSinApp::QSmartPrintSinApp(int &argc, char **argv)
    :SingleApplication(argc, argv, true)
{ 
}

 QSmartPrintSinApp::~QSmartPrintSinApp()
 {
 }


 void QSmartPrintSinApp:: start()
 {
     if (!print){

        print = new QPrintDirectory();

        controller.add_task(print);

     }

     print->setPrinter(PrinterName);

     print->setPrintDir(PRINTDirectory);

     print->setPDFDir(PDFDirectory);

 }


 void QSmartPrintSinApp::stop()
 {
     if (print)
       print->cancel();
 }

void QSmartPrintSinApp::Message(int instanceId, QByteArray message )
{
    Q_UNUSED(instanceId)

    Q_UNUSED(message)

    const QWidgetList topLevelWidgets = QApplication::topLevelWidgets();

    if (!topLevelWidgets.size())
      QTimer::singleShot(100, this, &QSmartPrintSinApp::selectPrinter);
    else{

      QWidget *w = topLevelWidgets[0];

      QApplication::alert(w);
    }

}

void QSmartPrintSinApp::selectPrinter()
{
  QSelectPrinter  SelectPrinter;

  SelectPrinter.exec();

  start();
}

void QSmartPrintSinApp::startPrint()
{
  start();
}

void QSmartPrintSinApp::stopPrint()
{
  stop();
}
