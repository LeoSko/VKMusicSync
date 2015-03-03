QT       += core gui network webkitwidgets webkit

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = VKMusicSync
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp

HEADERS  += mainwindow.h

FORMS    += mainwindow.ui

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/3rdparty/lib/ -lvreen
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/3rdparty/lib/ -lvreend
else:unix: LIBS += -L$$PWD/3rdparty/lib/ -lvreen

INCLUDEPATH += $$PWD/3rdparty
DEPENDPATH += $$PWD/3rdparty

unix|win32: LIBS += -L$$PWD/3rdparty/lib/ -lvreenoauth
