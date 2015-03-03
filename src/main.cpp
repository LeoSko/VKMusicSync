#include "mainwindow.h"
#include "stringconstants.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    a.setApplicationName(APP_NAME);
    a.setOrganizationName(ORG_NAME);
    a.setApplicationVersion(APP_VERSION);
    w.show();

    return a.exec();
}
