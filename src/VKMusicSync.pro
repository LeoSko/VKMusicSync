QT       += core gui network webkitwidgets webkit

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = VKMusicSync
TEMPLATE = app
CONFIG += c++11

SOURCES += main.cpp\
        mainwindow.cpp \
    fileremovedialog.cpp

HEADERS  += mainwindow.h \
    stringconstants.h \
    numericalconstants.h \
    fileremovedialog.h

FORMS    += mainwindow.ui \
    fileremovedialog.ui

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/3rdparty/lib/ -lvreen
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/3rdparty/lib/ -lvreend
else:unix: LIBS += -L$$PWD/3rdparty/lib/ -lvreen

INCLUDEPATH += $$PWD/3rdparty
DEPENDPATH += $$PWD/3rdparty

LIBS += -L$$PWD/3rdparty/lib/ -lvreenoauth

RC_ICONS = $$PWD/icons/VKMusicSyncIcon.ico

RESOURCES += \
    resources.qrc
