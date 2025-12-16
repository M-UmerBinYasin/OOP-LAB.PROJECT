QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

SOURCES += \
    main.cpp \
    bankmainwindow.cpp \
    bankaccount.cpp \
    currentaccount.cpp

HEADERS += \
    bankmainwindow.h \
    bankaccount.h \
    currentaccount.h

FORMS += \
    bankmainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
