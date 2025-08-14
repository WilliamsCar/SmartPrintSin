QT += core network widgets
QT += gui printsupport

CONFIG += c++11  c++14
CONFIG -= app_bundle

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS
DEFINES += QAPPLICATION_CLASS=QApplication

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += \
        QController.cpp \
        QDeposit.cpp \
        QFileInvoice.cpp \
        QInvoice.cpp \
        QPrint.cpp \
        QPrintRange.cpp \
        QSelectPrinter.cpp \
        QSmartPrintSin.cpp \
        QSmartPrintSinApp.cpp \
    QStatus.cpp \
        main.cpp \
        singleapplication.cpp \
        singleapplication_p.cpp \
    QPrintDirectory.cpp \
    QPrintFile.cpp \
    qgsdelimitedtextfile.cpp

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

HEADERS += \
    QController.h \
    QDeposit.h \
    QFileInvoice.h \
    QInvoice.h \
    QPrint.h \
    QPrintRange.h \
    QSelectPrinter.h \
    QSmartPrintSin.h \
    QSmartPrintSinApp.h \
    QStatus.h \
    global.h \
    singleapplication.h \
    singleapplication_p.h \
    QPrintDirectory.h \
    QPrintFile.h \
    qgsdelimitedtextfile.h

FORMS += \
    QPrintRange.ui \
    QSelectPrinter.ui

RESOURCES += \
    resources.qrc

win32 {
    CONFIG(release, debug|release): LIBS += -L$$PWD/../build-qhttp-Desktop_Qt_5_13_0_MinGW_32_bit-Release/xbin/ -lqhttp
    else:CONFIG(debug, debug|release): LIBS += -L$$PWD/../build-qhttp-Desktop_Qt_5_13_0_MinGW_32_bit-Debug/xbin/ -lqhttp

    INCLUDEPATH += $$PWD/../qhttp-master/src
    DEPENDPATH += $$PWD/../qhttp-master/src

    INCLUDEPATH += $$PWD/../qhttp-master/example/include
    DEPENDPATH += $$PWD/../qhttp-master/example/include


    CONFIG(release, debug|release): LIBS += -L$$PWD/../build-QQREncode-Desktop_Qt_5_13_0_MinGW_32_bit-Release/release/ -lQQREncode
    else:CONFIG(debug, debug|release): LIBS += -L$$PWD/../build-QQREncode-Desktop_Qt_5_13_0_MinGW_32_bit-Debug/debug/ -lQQREncode

    INCLUDEPATH += $$PWD/../QQREncode
    DEPENDPATH += $$PWD/../QQREncode

    INCLUDEPATH += $$PWD/'../Program Files/poppler/include/poppler/qt5'
    DEPENDPATH += $$PWD/'../Program Files/poppler/include/poppler/qt5'

    LIBS += -L$$PWD/'../Program Files/poppler/lib/' -llibpoppler-qt5.dll


    win32:RC_FILE = SmartPrintSin.rc

    msvc:LIBS += Advapi32.lib
    gcc:LIBS += -ladvapi32

    msvc:LIBS += Winspool.lib
    gcc:LIBS +=  -lwinspool

}

unix{

    CONFIG(release, debug|release): LIBS += -L$$PWD/../REST-Services/build-qhttp-Desktop_Qt_5_13_0_GCC_64bit-Release/xbin/ -lqhttp
    else:CONFIG(debug, debug|release): LIBS += -L$$PWD/../REST-Services/build-qhttp-Desktop_Qt_5_13_0_GCC_64bit-Debug/xbin/ -lqhttp

    INCLUDEPATH += $$PWD/../REST-Services/qhttp-master/src
    DEPENDPATH += $$PWD/../REST-Services/qhttp-master/src

    INCLUDEPATH += $$PWD/../REST-Services/qhttp-master/example/include
    DEPENDPATH += $$PWD/../REST-Services/qhttp-master/example/include

    CONFIG(release, debug|release): LIBS += -L$$PWD/../build-QQREncode-Desktop_Qt_5_13_0_GCC_64bit-Release/ -lQQREncode
    else:CONFIG(debug, debug|release): LIBS += -L$$PWD/../build-QQREncode-Desktop_Qt_5_13_0_GCC_64bit-Debug/ -lQQREncode
ls
    INCLUDEPATH += $$PWD/../QQREncode
    DEPENDPATH += $$PWD/../QQREncode

    unix:!macx|win32: LIBS += -L$$PWD/../../../usr/local/lib/ -lpoppler-qt5

    INCLUDEPATH += $$PWD/../../../usr/local/include/poppler/qt5
    DEPENDPATH += $$PWD/../../../usr/local/include/poppler/qt5

}

CONFIG(release, debug|release) {
DEFINES += QT_NO_DEBUG_OUTPUT
}
