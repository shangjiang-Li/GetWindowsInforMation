QT       += core gui concurrent axcontainer

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0
include(../QXlsx/QXlsx.pri)

SOURCES += \
    DumpSMBIOS.cpp \
    TabBarStyle.cpp \
    applistver.cpp \
    delegate_excel.cpp \
    infomachine.cpp \
    main.cpp \
    mainwindow.cpp \
    mpm.cpp \
    smbios.cpp \
    soic_excel.cpp \
    tabbarplus.cpp \
    threadcrl.cpp \
    xlsxsheetmodel.cpp

HEADERS += \
    DefStrings.h \
    DumpSMBIOS.h \
    TabBarStyle.h \
    applistver.h \
    delegate_excel.h \
    infomachine.h \
    mainwindow.h \
    mpm.h \
    smbios.h \
    soic_excel.h \
    tabbarplus.h \
    threadcrl.h \
    xlsxsheetmodel.h

FORMS += \
    SOIC.ui \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

unix|win32: LIBS += -lSetupAPI

unix|win32: LIBS += -lpowrprof

unix|win32: LIBS += -lOle32

win32: LIBS += -lwbemuuid

win32: LIBS += -lOleAut32

win32: LIBS += -lkernel32

win32: LIBS += -liphlpapi

RESOURCES += \
    SOIC.qrc \

RC_ICONS = SOIC.ico

VERSION = 2.0.1.1

QMAKE_TARGET_COMPANY ="Quantacn.com"

QMAKE_TARGET_DESCRIPTION = "SIT Omnipresent Information Collect"

QMAKE_TARGET_COPYRIGHT = "Shangjiang.Li@Quantacn.com"

QMAKE_TARGET_PRODUCT = "QT App"

RC_LANG = 0x0800
win32:CONFIG(release, debug|release): LIBS += -L'E:/Windows Kits/10/Lib/10.0.22621.0/um/x64/' -lappmgmts
else:win32:CONFIG(debug, debug|release): LIBS += -L'E:/Windows Kits/10/Lib/10.0.22621.0/um/x64/' -lappmgmtsd

INCLUDEPATH += 'E:/Windows Kits/10/Lib/10.0.22621.0/um/x64'
DEPENDPATH += 'E:/Windows Kits/10/Lib/10.0.22621.0/um/x64'



