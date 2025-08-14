#include "QStatus.h"
#include <QPrinter>
#include <QJsonDocument>
#include <QJsonObject>

#include "global.h"


#ifdef Q_OS_WIN

#include "windows.h"

BOOL GetJobs(HANDLE hPrinter, JOB_INFO_2 **ppJobInfo, int *pcJobs, DWORD *pStatus)
{

    DWORD               cByteNeeded,
                        nReturned,
                        cByteUsed;
    JOB_INFO_2          *pJobStorage = NULL;
    PRINTER_INFO_2       *pPrinterInfo = NULL;

   if (!GetPrinter(hPrinter, 2, NULL, 0, &cByteNeeded))
   {
       if (GetLastError() != ERROR_INSUFFICIENT_BUFFER)
           return FALSE;
   }

    pPrinterInfo = (PRINTER_INFO_2 *)malloc(cByteNeeded);

    if (!(pPrinterInfo))
      return FALSE;

   if (!GetPrinter(hPrinter,
           2,
           (LPBYTE)pPrinterInfo,
           cByteNeeded,
           &cByteUsed))
   {

       free(pPrinterInfo);
       pPrinterInfo = NULL;
       return FALSE;
   }

   if (!EnumJobs(hPrinter,
           0,
           pPrinterInfo->cJobs,
           2,
           NULL,
           0,
           (LPDWORD)&cByteNeeded,
           (LPDWORD)&nReturned))
   {
       if (GetLastError() != ERROR_INSUFFICIENT_BUFFER)
       {
           free(pPrinterInfo);
           pPrinterInfo = NULL;
           return FALSE;
       }
   }

   pJobStorage = (JOB_INFO_2 *)malloc(cByteNeeded);

   if (!pJobStorage)
   {
       free(pPrinterInfo);
       pPrinterInfo = NULL;
       return FALSE;
   }

   ZeroMemory(pJobStorage, cByteNeeded);


   if (!EnumJobs(hPrinter,
           0,
           pPrinterInfo->cJobs,
           2,
           (LPBYTE)pJobStorage,
           cByteNeeded,
           (LPDWORD)&cByteUsed,
           (LPDWORD)&nReturned))
   {
       free(pPrinterInfo);
       free(pJobStorage);
       pJobStorage = NULL;
       pPrinterInfo = NULL;
       return FALSE;
   }

   *pcJobs = nReturned;
   *pStatus = pPrinterInfo->Status;
   *ppJobInfo = pJobStorage;
   free(pPrinterInfo);

    return TRUE;
}

BOOL IsPrinterError(HANDLE hPrinter, int &cJobs)
   {

       JOB_INFO_2  *pJobs;
       DWORD       dwPrinterStatus;

       cJobs = 0;

       if (!GetJobs(hPrinter, &pJobs, &cJobs, &dwPrinterStatus))
          return FALSE;

       if (dwPrinterStatus &
           (PRINTER_STATUS_ERROR |
           PRINTER_STATUS_PAPER_JAM |
           PRINTER_STATUS_PAPER_OUT |
           PRINTER_STATUS_PAPER_PROBLEM |
           PRINTER_STATUS_OUTPUT_BIN_FULL |
           PRINTER_STATUS_NOT_AVAILABLE |
           PRINTER_STATUS_NO_TONER |
           PRINTER_STATUS_OUT_OF_MEMORY |
           PRINTER_STATUS_OFFLINE |
           PRINTER_STATUS_DOOR_OPEN))
       {
           free( pJobs );
           return TRUE;
       }

       free( pJobs );

       return FALSE;

}

#endif

QStatus::QStatus(const QString& printerName)
    : QObject(nullptr),  Name(printerName)
{
}

void QStatus::run()
{
    QJsonDocument jsonReply;
    QJsonObject object;
    QJsonObject printer_obj;
    QString status = "Error";

#ifdef Q_OS_WIN

    HANDLE hPrinter = NULL;
    int jobs = 0;

    if (OpenPrinter((LPTSTR)Name.toStdWString().c_str(), &hPrinter, NULL)){

      if (!IsPrinterError(hPrinter, jobs))
        status = QString("%1 documentos en cola").arg(jobs);

      ClosePrinter(hPrinter);

    }
#else

    QPrinter printer(QPrinter::ScreenResolution);

    printer.setPrinterName(Name);

    switch (printer.printerState()) {

     case QPrinter::Idle:
       status = "Listo";
     break;
     case QPrinter::Active:
        status = "Imprimiendo";
     break;
     case QPrinter::Aborted:
        status = "Abortado";
     break;
     case QPrinter::Error:
        status = "Error";
     break;

    }

#endif


    jsonReply = Reply(OPERATION_SUCCESSFULLY, Errors[OPERATION_SUCCESSFULLY]);

    printer_obj["name"] = Name;
    printer_obj["status"] = status;

    object = jsonReply.object();

    object["printer"] = QJsonDocument(printer_obj).object();

    emit response(QJsonDocument(object));

    emit finished();

}
