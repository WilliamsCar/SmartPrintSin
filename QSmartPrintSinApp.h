#ifndef QSMARTBIOSINAPP_H
#define QSMARTBIOSINAPP_H

#include <singleapplication.h>
#include "QController.h"
#include "QPrintDirectory.h"

class QSmartPrintSinApp : public SingleApplication
{
  public:
     QSmartPrintSinApp(int &argc, char **argv);
     ~QSmartPrintSinApp();

  protected:
    void start();
    void stop();

  public slots:
     void selectPrinter();
     void startPrint();
     void stopPrint();
     void Message( int instanceId, QByteArray message );

  private:
      QController controller;
      QPrintDirectory *print = nullptr;
};

#endif // QSMARTBIOSINAPP_H
